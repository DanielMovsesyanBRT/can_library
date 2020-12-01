/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 14:56:47
 * File : remote_ecu.cpp
 *
 */
    
#include "remote_ecu.hpp"  
#include "can_constants.hpp" 
#include "can_processor.hpp" 

namespace brt {
namespace can {

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

/**
 * \fn  delete
 *
 * @param  ptr : void* 
 * @return  void RemoteECU::operator
 */
void RemoteECU::operator delete  ( void* ptr )
{ 
  if (!allocator<RemoteECU>::free(ptr))
    ::free(ptr);
}

/**
 * \fn  constructor RemoteECUPtr::RemoteECUPtr
 *
 * @param  <empty> : CanProcessor*
 * @param  name : const CanName& 
 */
RemoteECUPtr::RemoteECUPtr(CanProcessor* processor,const CanName& name /*= CanName()*/)
{
  RemoteECU* ecu = reinterpret_cast<RemoteECU*>(processor->cfg().remote_ecu_allocator().allocate());
  if (ecu == nullptr)
    ecu = reinterpret_cast<RemoteECU*>(::malloc(sizeof(RemoteECU)));

  ::new (ecu) RemoteECU(processor,name);
  reset(ecu);
}

} // can
} // brt

