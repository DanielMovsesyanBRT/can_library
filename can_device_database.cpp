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
  using namespace std::placeholders;
  _processor->register_pgn_receiver(PGN_AddressClaimed,
        &CanDeviceDatabase::pgn_received,this,_1,_2);
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
    // This is athe same device and nothing changed about that
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
      }
      else if (local->name().data64() == name.data64())
      {
        // impossible situation, however we have to remove
        // local device from the database to avoid collisions
        bus_map.erase_right(local->name().data64());
      }
      else if (local->name().data64() > name.data64())
      {
        // Generate Random address between 128 and 247
        int num_attempts = (247 - 128);
        std::random_device r;
        std::default_random_engine gen(r());
        std::uniform_int_distribution<> distrib(128, 247);

        while (num_attempts-- > 0)
        {
          uint8_t sa = static_cast<uint8_t>(distrib(gen));
          auto dv = bus_map.find_left(sa);
          if (dv.first)
            continue;
        }

        if (num_attempts == 0)
        {
          // Send Unable to claim address

        }
      }
    }
  }

}


} // can
} // brt

