/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/23/2020 11:9:41
 * File : can_protocol.cpp
 *
 */
    
#include "can_protocol.hpp"  


namespace brt {
namespace can {

/**
 * \fn  constructor CanProtocol::CanProtocol
 *
 * @param  processor : CanProcessor* 
 */
CanProtocol::CanProtocol(CanProcessor* processor)
: _processor(processor)
{

}

/**
 * \fn  destructor CanProtocol::~CanProtocol
 *
 * \brief <description goes here>
 */
CanProtocol::~CanProtocol()
{

}

} // can
} // brt


