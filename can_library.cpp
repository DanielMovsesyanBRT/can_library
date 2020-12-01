/**
 * 
 * can_library.cpp
 * 
 */

#include "can_library.hpp"
#include "can_processor.hpp"  

namespace brt {
namespace can {

/**
 * \fn  constructor CanInterface::CanInterface
 *
 * @param  callback : Callback* 
 */
CanInterface::CanInterface(Callback* callback)
: _cback(callback)
{
  if (_cback == nullptr)
    std::runtime_error("Callback interface is not set");
}

/**
 * \fn  create_can_interface
 *
 * @param   callback : CanInterface::Callback*
 * @param  cfg : const LibraryConfig& 
 * @return  CanInterface*
 */
CanInterface* create_can_interface(CanInterface::Callback* callback,const LibraryConfig& cfg /* = LibraryConfig()*/)
{
  return new CanProcessor(callback, cfg);
}


} // can
} // brt


