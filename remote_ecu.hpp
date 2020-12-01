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


class RemoteECUPtr;
/**
 * \class RemoteECU
 *
 */
class RemoteECU  : public CanECU
{
friend RemoteECUPtr;
public:
  virtual ~RemoteECU();

          void operator delete  ( void* ptr );

private:
  RemoteECU(CanProcessor*,const CanName& name = CanName());

};

/**
 * \class RemoteECUPtr
 *
 * Inherited from :
 *             shared_pointer 
 *             RemoteECU 
 */
class RemoteECUPtr : public shared_pointer<RemoteECU>
{
public:
  RemoteECUPtr() {}
  explicit RemoteECUPtr(CanProcessor*,const CanName& name = CanName());
  explicit RemoteECUPtr(CanECUPtr ecu)
  { reset( dynamic_cast<RemoteECU*>(ecu.get())); }
};

} // can
} // brt


