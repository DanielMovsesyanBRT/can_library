/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 13:35:27
 * File : can_name.cpp
 *
 */
    
#include "can_name.hpp"  

namespace brt {
namespace can {

/**
 * \fn  constructor CanName::CanName
 *
 * @param  id : uint32_t 
 * @param  mf_code :  uint16_t 
 * @param  ecu_instance :  uint8_t 
 * @param  function_instance :  uint8_t 
 * @param  function :  uint8_t 
 * @param  device_class :  uint8_t 
 * @param  device_class_instance :  uint8_t 
 * @param  industry_group :  uint8_t 
 * @param  self_configurable :  bool 
 */
CanName::CanName(uint32_t id, uint16_t mf_code, uint8_t ecu_instance, uint8_t function_instance,
                  uint8_t function, uint8_t device_class, uint8_t device_class_instance,
                  uint8_t industry_group, bool self_configurable)
: _empty(false)
{
  _name._data[0] = static_cast<uint8_t>(id & 0xFF);
  _name._data[1] = static_cast<uint8_t>((id >> 8) & 0xFF);
  _name._data[2] = static_cast<uint8_t>( ((id >> 16) & 0x1F) | ((mf_code & 7) << 5) );
  _name._data[3] = static_cast<uint8_t>((mf_code >> 3) & 0xFF);
  _name._data[4] = static_cast<uint8_t>((ecu_instance & 7) | ((function_instance & 0x1F) << 3));
  _name._data[5] = function;
  _name._data[6] = (device_class & 0x7F) << 1;
  _name._data[7] = static_cast<uint8_t>( (device_class_instance & 0xF) | ((industry_group & 7) << 4) | ((self_configurable ? 1 : 0) << 7));
}

} // can
} // brt

