/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 14:56:47
 * File : remote_ecu.hpp
 *
 */

#pragma once

#include "can_ecu.hpp"
#include "can_utils.hpp"

namespace brt {
namespace can {

/**
 * \class RemoteECU
 *
 */
class RemoteECU  : public CanECU
{
public:
  RemoteECU(CanProcessor*,const CanName& name = CanName());
  virtual ~RemoteECU();

          void* operator new( size_t count )
          { return _allocator.allocate(); }

          void operator delete  ( void* ptr )
          { _allocator.free(ptr); }

private:
  static  allocator<RemoteECU>    _allocator;

};

typedef std::shared_ptr<RemoteECU> RemoteECUPtr;

} // can
} // brt


