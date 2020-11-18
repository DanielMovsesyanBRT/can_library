/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 15:0:44
 * File : local_ecu.cpp
 *
 */
    
#include "local_ecu.hpp"  

namespace brt {
namespace can {

/**
 * \fn  constructor LocalECU::LocalECU
 *
 * @param  name : const CanName& 
 */
LocalECU::LocalECU(const CanName& name /*= CanName()*/)
: CanECU(name)
{

}

/**
 * \fn  destructor LocalECU::~LocalECU
 *
 */
LocalECU::~LocalECU()
{

}

} // can
} // brt
