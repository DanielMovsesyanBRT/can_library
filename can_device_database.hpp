/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 13:30:52
 * File : can_device_database.hpp
 *
 */

#pragma once

#include "local_ecu.hpp"
#include "remote_ecu.hpp"
#include "can_utils.hpp"
#include "can_message.hpp"

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <vector>

namespace brt {
namespace can {

class CanProcessor;
/**
 * \class CanDeviceDatabase
 *
 */
class CanDeviceDatabase  
{
  typedef BiKeyMap<uint8_t,uint64_t,CanECUPtr>  BusMap;
  typedef std::unordered_map<std::string,BusMap> DeviceMap;

public:
  CanDeviceDatabase(CanProcessor*);
  virtual ~CanDeviceDatabase();

          void                    create_bus(const std::string& bus_name);  

          CanECUPtr               get_ecu_by_address(uint8_t sa,const std::string& bus) const;
          CanECUPtr               get_ecu_by_name(const CanName& ecu_name) const;

          std::unordered_map<std::string,uint8_t> get_ecu_source_addresses(const CanName& ecu_name) const;
          std::vector<std::string> get_ecu_bus(const CanName& ecu_name) const;

          bool                    add_local_ecu(LocalECUPtr ecu, const std::string& bus,uint8_t address);

private:
          void                    pgn_received(CanMessagePtr,const std::string& bus_name);
          uint8_t                 find_free_address(BusMap& bus_map);

private:
  CanProcessor*                   _processor;
  
  DeviceMap                       _device_map;
  std::unordered_set<CanECUPtr>   _local_devices;
};

} // can
} // brt

