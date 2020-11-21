/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 13:33:44
 * File : can_ecu.cpp
 *
 */
    
#include "can_ecu.hpp"  
#include "can_processor.hpp"

namespace brt {
namespace can {

/**
 * \fn  constructor CanECU::CanECU
 *
 * @param  processor : CanProcessor* 
 * @param  name : const CanName& 
 */
CanECU::CanECU(CanProcessor* processor,const CanName& name /*= CanName()*/)
: _processor(processor)
, _name(name)
{

}

/**
 * \fn  destructor CanECU::~CanECU
 *
 */
CanECU::~CanECU()
{

}

/**
 * \fn  CanECU::get_address
 *
 * @param  & bus_name : const std::string
 * @return  uint8_t
 */
uint8_t CanECU::get_address(const std::string& bus_name) const
{
  return _processor->device_db().get_ecu_address(name(), bus_name);
}

} // can
} // brt
