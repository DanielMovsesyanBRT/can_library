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

namespace brt {
namespace can {

/**
 * \fn  constructor CanDeviceDatabase::CanDeviceDatabase
 *
 */
CanDeviceDatabase::CanDeviceDatabase(CanProcessor* processor)
: _processor(processor)
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
 * \fn  CanDeviceDatabase::initialize
 *
 */
void CanDeviceDatabase::initialize()
{
  using namespace std::placeholders;
  _processor->register_pgn_receiver(PGN_AddressClaimed, [&](CanMessagePtr message,const std::string& bus)
  {
    pgn_received(message, bus);
  });
}

/**
 * \fn  CanDeviceDatabase::create_bus
 *
 * @param  & bus_name : const std::string
 */
void CanDeviceDatabase::create_bus(const std::string& bus_name)
{
  if (_device_map.find(bus_name) != _device_map.end())
    return;
  
  _device_map.insert(DeviceMap::value_type(bus_name, BusMap()));
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
 * @param  message : CanMessagePtr 
 * @param  bus_name : const std::string&
 */
void CanDeviceDatabase::pgn_received(CanMessagePtr message,const std::string& bus_name)
{
  if (message->pgn() != PGN_AddressClaimed)
    return;

  auto bus_iter = _device_map.find(bus_name);
  if (bus_iter == _device_map.end())
    return;

  BusMap& bus_map = bus_iter->second;
  CanName name(message->data());
  
  auto ecu_left = bus_map.find_left(message->sa());
  auto ecu_right = bus_map.find_right(name.data64());
  
  if (ecu_left.first && ecu_right.first && (ecu_left.second.second == ecu_right.second.second))
  {
    // This is the same device and nothing changed about that
    return;
  }

  if (ecu_left.first && ecu_right.first && (ecu_left.second.second != ecu_right.second.second))
  {
    LocalECUPtr local = std::dynamic_pointer_cast<LocalECU>(ecu_right.second.second);
    RemoteECUPtr remote = std::dynamic_pointer_cast<RemoteECU>(ecu_left.second.second);

    if (!local) // not a local device
    {
      // remove from left and put back under new source address
      bus_map.erase_left(message->sa());
      bus_map.insert(message->sa(), name.data64(), ecu_left.second.second);
    }
    else // address collision that has to be resolved
    {
      if (local->name().data64() < name.data64())
      {
        // address claim
        local->claim_address(ecu_right.second.first, bus_name);
      }
      else if (local->name().data64() == name.data64())
      {
        // impossible situation, however we have to remove
        // local device from the database to avoid collisions
        bus_map.erase_right(local->name().data64());
      }
      else if (local->name().data64() > name.data64())
      {
        if (!local->name().is_self_configurable())
        {
          // Local device address is not self configurable,
          // So we disable it for current bus
          bus_map.erase_right(local->name().data64());
          local->claim_address(NULL_CAN_ADDRESS, bus_name);
        }
        else
        {
          uint8_t sa = find_free_address(bus_map);
          bus_map.erase_right(local->name().data64());
          if (sa == NULL_CAN_ADDRESS)
          {
            // Reached the end of all attemts
            local->claim_address(NULL_CAN_ADDRESS, bus_name);
          }
          else
          {
            bus_map.insert(sa, local->name().data64(), local);
            local->claim_address(sa, bus_name);
          }
        }
      }
    }
  }
}

/**
 * \fn  CanDeviceDatabase::add_local_ecu
 *
 * @param  ecu : LocalECUPtr 
 * @param  bus :  const std::string&
 * @param  address : uint8_t 
 * @return  bool
 */
bool CanDeviceDatabase::add_local_ecu(LocalECUPtr ecu, const std::string& bus,uint8_t address)
{
  auto bus_map = _device_map.find(bus);
  if (bus_map == _device_map.end())
    return false;

  if (address == BROADCATS_CAN_ADDRESS)
  {
    address = find_free_address(bus_map->second);
    if (address == NULL_CAN_ADDRESS)
      return false;

    bus_map->second.insert(address, ecu->name().data64(), ecu);
    ecu->claim_address(address, bus);
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
          ecu->claim_address(address, bus);
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
      ecu->claim_address(address, bus);
    }
  }

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

