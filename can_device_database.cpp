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
  _remote_devices.clear();
  _prerecorded_local_devices.clear();
}

/**
 * \fn  CanDeviceDatabase::create_bus
 *
 * @param   bus_name : const ConstantString&
 */
void CanDeviceDatabase::create_bus(const ConstantString& bus_name)
{
  bool register_pgn = false;
  {
    std::lock_guard<Mutex> l(_mutex);
    register_pgn = _device_map.empty();
    if (_device_map.find_if( [bus_name](const DeviceMap::value_type& value)->bool
        { return value.first == bus_name; }) != _device_map.end())
    {
      return;
    }
  
    _device_map.push(DeviceMap::value_type(bus_name, BusMap()));
  }

  if (register_pgn)
  {
    _processor->register_pgn_receiver(PGN_AddressClaimed, [this](const CanPacket& packet,const ConstantString& bus)
    {
      pgn_received(packet, bus);
    });
  }
}

/**
 * \fn  CanDeviceDatabase::get_ecu_by_address
 *
 * @param  sa : uint8_t 
 * @param   bus_name : const ConstantString&
 * @return  CanECUPtr
 */
CanECUPtr CanDeviceDatabase::get_ecu_by_address(uint8_t sa,const ConstantString& bus_name) const
{
  std::lock_guard<Mutex> l(_mutex);
  auto bus_iter = _device_map.find_if([bus_name](const DeviceMap::value_type& value)->bool
      { return value.first == bus_name; });

  if (bus_iter == _device_map.end())
    return CanECUPtr();

  return bus_iter->second[sa];
}

/**
 * \fn  CanDeviceDatabase::get_ecu_by_name
 *
 * @param   name : const CanName&
 * @param   bus_name : const ConstantString&
 * @return  CanECUPtr
 */
CanECUPtr CanDeviceDatabase::get_ecu_by_name(const CanName& name,const ConstantString& bus_name) const
{
  std::lock_guard<Mutex> l(_mutex);
  if (bus_name.empty())
  {
    for (auto bus_iter : _device_map)
    {
      auto device_iter = std::find_if(bus_iter.second.begin(),bus_iter.second.end(),[name](const CanECUPtr& ecu)->bool
          {
            return (ecu && (ecu->name().data64() == name.data64()));
          });
      
      if (device_iter != bus_iter.second.end())
        return (*device_iter);
    }
  }
  else
  {
    auto bus_iter = _device_map.find_if([bus_name](const DeviceMap::value_type& value)->bool
        { return value.first == bus_name; });

    if (bus_iter != _device_map.end())
    {
      auto device_iter = std::find_if(bus_iter->second.begin(),bus_iter->second.end(),[name](const CanECUPtr& ecu)->bool
          {
            return (ecu && (ecu->name().data64() == name.data64()));
          });
      
      if (device_iter != bus_iter->second.end())
        return (*device_iter);
    }
  }
  
  auto iter = _prerecorded_local_devices.find_if([name](const LocalECUPtr& local)->bool
      { return name.data64() == local->name().data64(); });

  if (iter != _prerecorded_local_devices.end())
    return *iter;

  return CanECUPtr();
}

/**
 * \fn  CanDeviceDatabase::get_ecu_address
 *
 * @param   ecu_name : const CanName&
 * @param   bus_name : const ConstantString&
 * @return  uint8_t
 */
uint8_t CanDeviceDatabase::get_ecu_address(const CanName& ecu_name,const ConstantString& bus_name) const
{
  std::lock_guard<Mutex> l(_mutex);
  if (bus_name.empty())
  {
    for (auto bus_iter : _device_map)
    {
      auto& array = bus_iter.second;
      for (size_t index = 0; index < array.size(); index++)
      {
        if (array[index] && (array[index]->name().data64() == ecu_name.data64()))
          return static_cast<uint8_t>(index);
      }
    }
  }
  else
  {
    auto bus_iter = _device_map.find_if([bus_name](const DeviceMap::value_type& value)->bool
        { return value.first == bus_name; });

    if (bus_iter != _device_map.end())
    {
      auto& array = bus_iter->second;
      for (size_t index = 0; index < array.size(); index++)
      {
        if (array[index] && (array[index]->name().data64() == ecu_name.data64()))
          return static_cast<uint8_t>(index);
      }
    }
  }
  
  return NULL_CAN_ADDRESS;
}

/**
 * \fn  CanDeviceDatabase::pgn_received
 *
 * @param   packet : const CanPacket&
 * @param   bus_name : const ConstantString&
 */
void CanDeviceDatabase::pgn_received(const CanPacket& packet,const ConstantString& bus_name)
{
  if (packet.pgn() != PGN_AddressClaimed)
    return;

  uint8_t sa = BROADCAST_CAN_ADDRESS;
  LocalECUPtr local;
  RemoteECUPtr remote;

  CanName name(packet.data());
  uint8_t address = get_ecu_address(name, bus_name);
  if (address == packet.sa())
  { 
    // This ecu is already egistered under 
    // received address or it claims "Cannot Claim Address"
    return;
  }

  {
    std::lock_guard<Mutex> l(_mutex);
    auto bus_iter = _device_map.find_if([bus_name](const DeviceMap::value_type& value)->bool
        { return value.first == bus_name; });

    if (bus_iter == _device_map.end())
      return;

    BusMap& bus_map = bus_iter->second;

    CanECUPtr by_addr = bus_map[packet.sa()];
    CanECUPtr by_name = bus_map[address];
    
    if (!by_name)
    {
      auto lost = _remote_devices.find_if([name](CanECUPtr ecu)->bool
      {
        return (ecu && (ecu->name().data64() == name.data64()));
      });

      if (lost != _remote_devices.end())
        by_name = (*lost);
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

      bus_map[address] = CanECUPtr();
    }

    // Check if this is a new Remote device sending address claim
    if (!is_remote_ecu(by_name))
    { 
      remote = RemoteECUPtr(_processor, name);
      by_name = remote;
      
      // Register new device in the Remote Database
      _remote_devices.push(by_name);
    }

    if ((packet.sa() != NULL_CAN_ADDRESS) && (!by_addr || !is_local_ecu(by_addr)))
    {
      // This slot is empty we need to register New ECU there
      // or relocate old one
      // Note: in case if there is some remote device exist under 
      // requested address the thser function will remove it from the map
      bus_map[packet.sa()] = by_name;
    }
    else if (is_local_ecu(by_addr))
    {
      // Address Collision - We need to activate ISO 11783-5 address negotiation protocol
      local = LocalECUPtr(by_addr);
      // Note: we don't process name == name situation, since it will be 
      // processed by the code above  
      if (local->name().data64() < name.data64())
      {
        // Keep the same address, which is source
        // address of received packet
        sa = packet.sa();
      }
      else if (local->name().data64() > name.data64())
      {
        // Ok here we are trying to change our address, but first
        // we will need to put remote device back to the map
        bus_map[packet.sa()] =  by_name;

        if (!local->name().is_self_configurable())
        {
          // Local device address is not self configurable,
          // So we disable it for current bus
          sa = NULL_CAN_ADDRESS;
        }
        else
        {
          sa = find_free_address(bus_map);
          if (sa != NULL_CAN_ADDRESS)
            bus_map[sa] = local;

        }
      }
    }
  } // mutex lock

  if (local && (sa != BROADCAST_CAN_ADDRESS))
    local->claim_address(sa, bus_name);

  if (remote)
  {
    remote->init_status();
    _processor->on_remote_ecu(remote, bus_name);
  }
}

/**
 * \fn  CanDeviceDatabase::add_local_ecu
 *
 * @param  ecu : LocalECUPtr 
 * @param   bus_name :  const ConstantString&
 * @param  address : uint8_t 
 * @return  bool
 */
bool CanDeviceDatabase::add_local_ecu(LocalECUPtr ecu, const ConstantString& bus_name,uint8_t address)
{
  CanBusStatus stat = _processor->get_bus_status(bus_name);
  if (stat == eBusInactive)
    return false;
  
  if (stat != eBusActive)
  {
    _processor->register_bus_callback(bus_name,[this,ecu,address](const ConstantString& bus_name, CanBusStatus status)->bool
    {
      if (status == eBusInactive)
        return true;
      
      if (status != eBusActive)
        return false; // Waiting state

      add_local_ecu(ecu, bus_name, address);
      return true;
    });

    std::lock_guard<Mutex> l(_mutex);
    _prerecorded_local_devices.push(ecu);

    return true;
  }
  
  bool claim = false;
  {
    std::lock_guard<Mutex> l(_mutex);
    
    // Remove local device from prerecorded list
    auto iter = _prerecorded_local_devices.find_if([ecu](const LocalECUPtr& local)->bool
    { 
      return ecu == local; 
    });

    if (iter != _prerecorded_local_devices.end())
      _prerecorded_local_devices.erase(iter);

    auto bus_iter = _device_map.find_if([bus_name](const DeviceMap::value_type& value)->bool
        { return value.first == bus_name; });

    if (bus_iter == _device_map.end())
      return false;

    if (address == BROADCAST_CAN_ADDRESS)
    {
      address = find_free_address(bus_iter->second);
      if (address == NULL_CAN_ADDRESS)
        return false;

      bus_iter->second[address] = ecu;
      claim = true;
    }
    else
    {
      auto old_ecu = bus_iter->second[address];
      if (old_ecu)
      {
        uint8_t new_address = NULL_CAN_ADDRESS;
        // The address is occupied, so we look to change it 
        // if all conditions are met
        if (ecu->name().is_self_configurable())
          new_address = find_free_address(bus_iter->second);
        
        if (new_address == NULL_CAN_ADDRESS)
        {
          // Old ECU is also local ECU, so 
          // we should not replace it
          if (is_local_ecu(old_ecu))
            return false;

          // Couldn't generate new address or 
          // address is not configurable
          if (ecu->name().data64() < old_ecu->name().data64())
          {
            // Our ecu has higher priority
            // So we try to push the other one out
            bus_iter->second[address] = ecu;
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
        bus_iter->second[address] = ecu;
        claim = true;
      }
    }
  }// mutex unlock

  if (claim)
    ecu->claim_address(address, bus_name);

  return true;
}

/**
 * \fn  CanDeviceDatabase::remove_local_ecu
 *
 * @param   ecu_name : const CanName&
 * @param   bus_name : const ConstantString&
 * @return  bool
 */
bool CanDeviceDatabase::remove_local_ecu(const CanName& ecu_name,const ConstantString& bus_name)
{
  std::lock_guard<Mutex> l(_mutex);
  auto bus_iter = _device_map.find_if([bus_name](const DeviceMap::value_type& value)->bool
      { return value.first == bus_name; });

  if (bus_iter == _device_map.end())
    return false;

  BusMap& bus_map = bus_iter->second;
  for (size_t index = 0; index < bus_map.size(); index++)
  {
    LocalECUPtr local(bus_map[index]);
    if (!local)
      continue;
    
    if (local->name().data64() == ecu_name.data64())
    {
      bus_map[index].reset();
      return true;
    }
  }

  return false;
}

/**
 * \fn  CanDeviceDatabase::add_remote_abstract_ecu
 *
 * @param  ecu : RemoteECUPtr 
 * @param   bus_name :  const ConstantString&
 * @param  address : uint8_t 
 * @return  bool
 */
bool CanDeviceDatabase::add_remote_abstract_ecu(RemoteECUPtr ecu, 
                const ConstantString& bus_name,uint8_t address)
{
  std::lock_guard<Mutex> l(_mutex);
  auto bus_iter = _device_map.find_if([bus_name](const DeviceMap::value_type& value)->bool
      { return value.first == bus_name; });

  if (bus_iter == _device_map.end())
    return false;

  BusMap& bus_map = bus_iter->second;
  if (bus_map[address])
    return false;

  bus_map[address] = ecu;
  return true;
}

/**
 * \fn  CanDeviceDatabase::get_local_ecus
 *
 * @param   list : fixed_list<LocalECUPtr>&
 * @param  buses  :  const std::initializer_list<ConstantString>&
 */
void CanDeviceDatabase::get_local_ecus(fixed_list<LocalECUPtr>& list, 
                      const std::initializer_list<ConstantString>& buses /*= std::initializer_list<ConstantString>()*/)
{
  std::lock_guard<Mutex> l(_mutex);

  if (buses.size() == 0)
  {
    for (auto bus_map : _device_map)
    {
      for (auto device : bus_map.second)
      {
        if (is_local_ecu(device))
          list.push(LocalECUPtr(device));
      }
    }
  }
  else
  {
    for (auto bus_name : buses)
    {
      auto bus_iter = _device_map.find_if([bus_name](const DeviceMap::value_type& value)->bool
          { return value.first == bus_name; });

      if (bus_iter != _device_map.end())
      {
        for (auto device : bus_iter->second)
        {
          if (is_local_ecu(device))
            list.push(LocalECUPtr(device));
        }
      }
    }
  }
}

/**
 * \fn  CanDeviceDatabase::get_remote_ecus
 *
 * @param   list : fixed_list<RemoteECUPtr>&
 * @param  buses  :  const std::initializer_list<ConstantString>&
 */
void CanDeviceDatabase::get_remote_ecus(fixed_list<RemoteECUPtr>& list, 
                      const std::initializer_list<ConstantString>& buses /*= std::initializer_list<ConstantString>()*/)
{
  std::lock_guard<Mutex> l(_mutex);

  if (buses.size() == 0)
  {
    for (auto bus_map : _device_map)
    {
      for (auto device : bus_map.second)
      {
        if (is_remote_ecu(device))
          list.push(RemoteECUPtr(device));
      }
    }
  }
  else
  {
    for (auto bus_name : buses)
    {
      auto bus_iter = _device_map.find_if([bus_name](const DeviceMap::value_type& value)->bool
          { return value.first == bus_name; });

      if (bus_iter != _device_map.end())
      {
        for (auto device : bus_iter->second)
        {
          if (is_remote_ecu(device))
            list.push(RemoteECUPtr(device));
        }
      }
    }
  }
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
    if (bus_map[sa])
      continue;
  }

  if (num_attempts == 0)
    return NULL_CAN_ADDRESS;

  return sa;
}

} // can
} // brt


