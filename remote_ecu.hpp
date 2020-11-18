/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 14:56:47
 * File : remote_ecu.hpp
 *
 */

#pragma once

#include "can_ecu.hpp"

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

  uint8_t                         get_address() const;
};

typedef std::shared_ptr<RemoteECU> RemoteECUPtr;

} // can
} // brt


