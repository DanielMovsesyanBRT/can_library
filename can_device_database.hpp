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
#include <array>

namespace brt {
namespace can {


class CanProcessor;
/**
 * \class CanDeviceDatabase
 *
 */
class CanDeviceDatabase  
{
  typedef std::array<CanECUPtr, 256>              BusMap;
  typedef std::unordered_map<std::string,BusMap>  DeviceMap;

public:
  CanDeviceDatabase(CanProcessor*);
  virtual ~CanDeviceDatabase();

          void                    create_bus(const std::string& bus_name);  

          CanECUPtr               get_ecu_by_address(uint8_t sa,const std::string& bus_name) const;
          CanECUPtr               get_ecu_by_name(const CanName& ecu_name,const std::string& bus_name) const;
          uint8_t                 get_ecu_address(const CanName& ecu_name,const std::string& bus_name) const;
      
          bool                    add_local_ecu(LocalECUPtr ecu, const std::string& bus_name,uint8_t address);
          bool                    remove_local_ecu(const CanName& ecu_name,const std::string& bus_name);
          
          bool                    add_remote_abstract_ecu(RemoteECUPtr ecu, const std::string& bus_name,uint8_t address);

          void                    get_local_ecus(fixed_list<LocalECUPtr>& list, 
                                                const std::initializer_list<std::string>& buses = std::initializer_list<std::string>());
          void                    get_remote_ecus(fixed_list<RemoteECUPtr>& list, 
                                                const std::initializer_list<std::string>& buses = std::initializer_list<std::string>());

private:
          void                    pgn_received(const CanPacket& packet,const std::string& bus_name);
          uint8_t                 find_free_address(BusMap& bus_map);
          
          bool                    is_local_ecu(CanECUPtr ecu) 
          { return LocalECUPtr(ecu).get() != nullptr; }

          bool                    is_remote_ecu(CanECUPtr ecu) 
          { return RemoteECUPtr(ecu).get() != nullptr; }

private:
  CanProcessor*                   _processor;
  mutable Mutex                   _mutex;
  
  DeviceMap                       _device_map;
  fixed_list<CanECUPtr>           _remote_devices;
  fixed_list<LocalECUPtr,32>      _prerecorded_local_devices;
};

} // can
} // brt

