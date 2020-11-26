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

allocator<RemoteECU> RemoteECU::_allocator;

/**
 * \fn  constructor RemoteECU::RemoteECU
 *
 * @param  processor : CanProcessor* 
 * @param  name : const CanName& 
 */
RemoteECU::RemoteECU(CanProcessor* processor,const CanName& name /*= CanName()*/)
: CanECU(processor,name)
{

}

/**
 * \fn  destructor RemoteECU::~RemoteECU
 *
 */
RemoteECU::~RemoteECU()
{

}

} // can
} // brt

