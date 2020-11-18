/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 13:33:44
 * File : can_ecu.hpp
 *
 */

#pragma once

#include "can_name.hpp"

#include <memory>
#include <unordered_map>

namespace brt {
namespace can {


/**
 * \class CanECU
 *
 */
class CanECU : std::enable_shared_from_this<CanECU>
{
public:
  CanECU(const CanName& name = CanName());
  virtual ~CanECU();

  std::shared_ptr<CanECU>         getptr() { return shared_from_this(); }
  const CanName&                  name() const { return _name; }

  std::unordered_map<std::string,uint8_t> get_addresses() const;

private:
  CanName                         _name;
};

typedef std::shared_ptr<CanECU>   CanECUPtr;

} // can
} // brt

