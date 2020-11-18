/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 13:35:27
 * File : can_name.hpp
 *
 */

#pragma once

#include <stdint.h>
#include <string.h>

namespace brt {
namespace can {

/**
 * \class CanName
 *
 * \brief <description goes here>
 */
class CanName  
{
public:
  CanName() : _empty(true) {}
  CanName(const CanName& name) : _empty(name._empty) { _name._data64 = name._name._data64; }
  explicit CanName(const uint8_t* name_data) : _empty(false)  { memcpy(_name._data, name_data, 8); }
  explicit CanName(uint64_t name_data64) : _empty(false) { _name._data64 = name_data64; }
  explicit CanName(uint32_t id, uint16_t mf_code, uint8_t ecu_instance, uint8_t function_instance,
                        uint8_t function, uint8_t device_class, uint8_t device_class_instance, 
                        uint8_t industry_group, bool self_configurable);

  virtual ~CanName() {}

  bool                            is_self_configurable() const { return (((_name._data[7] >> 7) & 1) != 0); }
  uint8_t                         industry_group() const { return ((_name._data[7] >> 4) & 7); }
  uint8_t                         device_class_instance() const { return (_name._data[7] & 0xF); }
  uint8_t                         device_class() const { return ((_name._data[6] >> 1) & 0x7F);}
  uint8_t                         function() const { return _name._data[5]; }
  uint8_t                         function_instance() const { return ((_name._data[4] >> 3) & 0x1F); }
  uint8_t                         ecu_instance() const { return (_name._data[4] & 7); }
  uint16_t                        mf_code() const { return ((_name._data[3] << 3) | ((_name._data[2] >> 5) & 7)); }
  uint32_t                        id() const { return _name._data[0] | (_name._data[1] << 8) | ((_name._data[2] & 0x1F) << 16); }

  bool                            is_empty() const { return _empty; }

  const uint8_t*                  data() const { return _name._data; }
  uint64_t                        data64() const { return _name._data64; }

private:
  union Name
  {
     Name() : _data64(0ULL) {}
    uint8_t                         _data[8];
    uint64_t                        _data64;    
  }                               _name;

  bool                            _empty;
};

} // can
} // brt
