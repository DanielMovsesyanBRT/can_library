/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 13:30:52
 * File : can_device_database.cpp
 *
 */
    
#include "can_device_database.hpp"  
#include "can_constants.hpp"  
#include "can_processor.hpp"  
#include "can_message.hpp"  

#include <algorithm>
#include <random>
#include <mutex>

namespace brt {
namespace can {

/**
 * \fn  constructor CanDeviceDatabase::CanDeviceDatabase
 *
 */
CanDeviceDatabase::CanDeviceDatabase(CanProcessor* processor)
: _processor(processor)
, _mutex(processor)
{
}

/**
 * \fn  destructor CanDeviceDatabase::~CanDeviceDatabase
 *
 */
CanDeviceDatabase::~CanDeviceDatabase()
{
}

/**
 * \fn  CanDeviceDatabase::create_bus
 *
 * @param  & bus_name : const std::string
 */
void CanDeviceDatabase::create_bus(const std::string& bus_name)
{
  bool register_pgn = false;
  {
    std::lock_guard<Mutex> l(_mutex);
    register_pgn = _device_map.empty();
    if (_device_map.find(bus_name) != _device_map.end())
      return;
  
    _device_map.insert(DeviceMap::value_type(bus_name, BusMap()));
  }

  if (register_pgn)
  {
    _processor->register_pgn_receiver(PGN_AddressClaimed, [this](const CanPacket& packet,const std::string& bus)
    {
      pgn_received(packet, bus);
    });
  }
}

/**
 * \fn  CanDeviceDatabase::get_ecu_by_address
 *
 * @param  sa : uint8_t 
 * @param  & bus : const std::string
 * @return  CanECUPtr
 */
CanECUPtr CanDeviceDatabase::get_ecu_by_address(uint8_t sa,const std::string& bus) const
{
  std::lock_guard<Mutex> l(_mutex);
  auto bus_iter = _device_map.find(bus);
  if (bus_iter == _device_map.end())
    return CanECUPtr();

  auto device = bus_iter->second.find_left(sa);
  if (!device.first)
    return CanECUPtr();

  return device.second.second;
}

/**
 * \fn  CanDeviceDatabase::get_ecu_by_name
 *
 * @param  name : const CanName& 
 * @return  CanECUPtr
 */
CanECUPtr CanDeviceDatabase::get_ecu_by_name(const CanName& name) const
{
  std::lock_guard<Mutex> l(_mutex);
  for (auto bus_iter : _device_map)
  {
    auto device = bus_iter.second.find_right(name.data64());
    if (!device.first)
      continue;

    return device.second.second;
  }

  return CanECUPtr();
}

/**
 * \fn  CanDeviceDatabase::get_ecu_source_addresses
 *
 * @param  ecu_name : const CanName& 
 * @return  std::unordered_map<std::string,uint8_t>
 */
std::unordered_map<std::string,uint8_t> CanDeviceDatabase::get_ecu_source_addresses(const CanName& ecu_name) const
{
  std::unordered_map<std::string,uint8_t> result;

  std::lock_guard<Mutex> l(_mutex);
  for (auto bus_iter : _device_map)
  {
    auto device = bus_iter.second.find_right(ecu_name.data64());
    if (!device.first)
      continue;

    result[bus_iter.first] = device.second.first;
  }
  return result;
}

/**
 * \fn  CanDeviceDatabase::get_ecu_bus
 *
 * @param  ecu_name : const CanName& 
 * @return  std::vector<std::string> - can be multiple buses
 */
std::vector<std::string> CanDeviceDatabase::get_ecu_bus(const CanName& ecu_name) const
{
  std::vector<std::string> result;

  std::lock_guard<Mutex> l(_mutex);
  for (auto bus_iter : _device_map)
  {
    auto device = bus_iter.second.find_right(ecu_name.data64());
    if (!device.first)
      continue;

    result.push_back(bus_iter.first);
  }
  return result;
}

/**
 * \fn  CanDeviceDatabase::pgn_received
 *
 * @param  packet : const CanPacket& 
 * @param  & bus_name : const std::string
 */
void CanDeviceDatabase::pgn_received(const CanPacket& packet,const std::string& bus_name)
{
  if (packet.pgn() != PGN_AddressClaimed)
    return;

  uint8_t sa = BROADCATS_CAN_ADDRESS;
  LocalECUPtr local;
  {
    std::lock_guard<Mutex> l(_mutex);
    auto bus_iter = _device_map.find(bus_name);
    if (bus_iter == _device_map.end())
      return;

    BusMap& bus_map = bus_iter->second;
    CanName name(packet.data());
    
    auto ecu_left = bus_map.find_left(packet.sa());
    auto ecu_right = bus_map.find_right(name.data64());
    
    if (ecu_left.first && ecu_right.first && (ecu_left.second.second == ecu_right.second.second))
    {
      // This is the same device and nothing changed about that
      return;
    }

    CanECUPtr by_addr = ecu_left.second.second;
    CanECUPtr by_name = ecu_right.second.second;

    if (!by_name)
    {
      auto lost = _remote_devices.find(name.data64());
      if (lost != _remote_devices.end())
        by_name = lost->second;
    }

    if (by_name)
    {
      // Whether Remote ECU changed its address or
      // Somehow another ECU on the BUS claimed the same name as 
      // one of our local ECUs. We need to erase that slot from teh map

      if (is_local_ecu(by_name))
      {
        // TODO: notify about ECU error !!!
      }

      bus_map.erase_right(name.data64());
    }

    // Check if this is a new Remote device sending address claim
    if (!is_remote_ecu(by_name))
    { 
      by_name.reset(new RemoteECU(_processor, name));
      
      // Register new device iin the Remote Database
      _remote_devices[name.data64()] = by_name;
    }

    if (!by_addr || !is_local_ecu(by_addr))
    {
      // This slot is empty we need to register New ECU there
      // or relocate old one
      // Note: in case if there is some remote device exist under 
      // requested address the thser function will remove it from the map
      bus_map.insert(packet.sa(), name.data64(), by_name);
    }
    else if (is_local_ecu(by_addr))
    {
      // Address Collision - We need to activate ISO 11783-5 address negotiation protocol
      local = std::dynamic_pointer_cast<LocalECU>(by_addr);

      // Note: we don't process name == name situation, since it will be 
      // processed by the code above  
      if (local->name().data64() < name.data64())
      {
        // address claim
        sa = ecu_right.second.first;
      }
      else if (local->name().data64() > name.data64())
      {
        // Ok here we are trying to change our address, but first
        // we will need to put remote device back to the map
        bus_map.insert(packet.sa(), name.data64(), by_name);

        if (!local->name().is_self_configurable())
        {
          // Local device address is not self configurable,
          // So we disable it for current bus
          bus_map.erase_right(local->name().data64());
          sa = NULL_CAN_ADDRESS;
        }
        else
        {
          sa = find_free_address(bus_map);
          if (sa == NULL_CAN_ADDRESS)
          {
            // Reached the end of all attemts
            bus_map.erase_right(local->name().data64());
          }
          else
          {
            bus_map.insert(sa, local->name().data64(), local);
          }
        }
      }
    }
  } // mutex lock

  if (local && (sa != BROADCATS_CAN_ADDRESS))
    local->claim_address(sa, bus_name);
}

/**
 * \fn  CanDeviceDatabase::add_local_ecu
 *
 * @param  ecu : LocalECUPtr 
 * @param  bus_name :  const std::string&
 * @param  address : uint8_t 
 * @return  bool
 */
bool CanDeviceDatabase::add_local_ecu(LocalECUPtr ecu, const std::string& bus_name,uint8_t address)
{
  CanBusStatus stat = _processor->get_bus_status(bus_name);
  if (stat == eBusInactive)
    return false;
  
  if (stat != eBusActive)
  {
    _processor->register_bus_callback(bus_name,[this,ecu,address](const std::string& bus_name, CanBusStatus status)
    {
      if (status == eBusInactive)
        return true;
      
      if (status != eBusActive)
        return false; // Waiting state

      add_local_ecu(ecu, bus_name, address);      
    });

    return true;
  }
  
  
  bool claim = false;
  
  {
    std::lock_guard<Mutex> l(_mutex);

    auto bus_map = _device_map.find(bus_name);
    if (bus_map == _device_map.end())
      return false;

    if (address == BROADCATS_CAN_ADDRESS)
    {
      address = find_free_address(bus_map->second);
      if (address == NULL_CAN_ADDRESS)
        return false;

      bus_map->second.insert(address, ecu->name().data64(), ecu);
      claim = true;
    }
    else
    {
      auto slot = bus_map->second.find_left(address);
      if (slot.first)
      {
        uint8_t new_address = NULL_CAN_ADDRESS;
        // The address is occupied, so we look to change it 
        // if all conditions are met
        if (ecu->name().is_self_configurable())
          new_address = find_free_address(bus_map->second);
        
        if (new_address == NULL_CAN_ADDRESS)
        {
          // Couldn't generate new address or 
          // address is not configurable
          if (ecu->name().data64() < slot.second.first)
          {
            // Our ecu has higher priority
            // So we try to push the other one out
            bus_map->second.insert(address, ecu->name().data64(), ecu);
            claim = true;
          }
          else
          {
            // Unable to register local ECU on this BUS
            // So we are going to discard it
            return false;
          }
        }
      }
      else
      {
        bus_map->second.insert(address, ecu->name().data64(), ecu);
        claim = true;
      }
    }
  }// mutex unlock

  if (claim)
    ecu->claim_address(address, bus_name);

  return true;
}

/**
 * \fn  CanDeviceDatabase::find_free_address
 *
 * @param  bus_map : BusMap& 
 * @return  uint8_t
 */
uint8_t CanDeviceDatabase::find_free_address(BusMap& bus_map)
{
  // Generate Random address between 128 and 247
  int num_attempts = (247 - 128);
  std::random_device r;
  std::default_random_engine gen(r());
  std::uniform_int_distribution<> distrib(128, 247);

  uint8_t sa = NULL_CAN_ADDRESS;
  while (num_attempts-- > 0)
  {
    sa = static_cast<uint8_t>(distrib(gen));
    auto dv = bus_map.find_left(sa);
    if (dv.first)
      continue;
  }

  if (num_attempts == 0)
    return NULL_CAN_ADDRESS;

  return sa;
}

} // can
} // brt


