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
 */
CanECU::CanECU(const CanName& name /*= CanName()*/)
: _name(name)
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
 * \fn  CanECU::get_addresses
 *
 * @return  std::unordered_map<std::string,uint8_t>
 */
std::unordered_map<std::string,uint8_t> CanECU::get_addresses() const
{
  return CanProcessor::get()->device_db().get_ecu_source_addresses(name());
}

} // can
} // brt
