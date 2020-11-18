/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 14:56:47
 * File : remote_ecu.cpp
 *
 */
    
#include "remote_ecu.hpp"  
#include "can_constants.hpp" 

namespace brt {
namespace can {

/**
 * \fn  constructor RemoteECU::RemoteECU
 *
 * @param  name : const CanName& 
 */
RemoteECU::RemoteECU(const CanName& name /*= CanName()*/)
: CanECU(name)
{

}

/**
 * \fn  destructor RemoteECU::~RemoteECU
 *
 */
RemoteECU::~RemoteECU()
{

}


/**
 * \fn  RemoteECU::get_address
 *
 * @return  uint8_t
 */
uint8_t RemoteECU::get_address() const
{
  std::unordered_map<std::string,uint8_t> sa_map = get_addresses();
  if (sa_map.size() != 1)
    return NULL_CAN_ADDRESS;

  return sa_map.begin()->second;
}


} // can
} // brt

