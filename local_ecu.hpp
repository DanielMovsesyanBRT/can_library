/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 15:0:44
 * File : local_ecu.hpp
 *
 */

#pragma once

#include "can_ecu.hpp"

namespace brt {
namespace can {

/**
 * \class LocalECU
 *
 */
class LocalECU : public CanECU
{
public:
  LocalECU(const CanName& name = CanName());
  virtual ~LocalECU();

};

typedef std::shared_ptr<LocalECU>   LocalECUPtr;


} // can
} // brt
