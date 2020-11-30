/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 13:33:44
 * File : can_ecu.hpp
 *
 */

#pragma once

#include "can_name.hpp"
#include "can_utils.hpp"

namespace brt {
namespace can {

class CanProcessor;
/**
 * \class CanECU
 *
 */
class CanECU : public shared_class<CanECU>
{
public:
  CanECU(CanProcessor*,const CanName& name = CanName());
  virtual ~CanECU();

          const CanName&          name() const { return _name; }

          uint8_t                 get_address(const std::string& bus_name) const;

protected:
          CanProcessor*           processor() { return _processor; }
private:
  CanProcessor*                   _processor;
  CanName                         _name;
};

typedef shared_pointer<CanECU>   CanECUPtr;

} // can
} // brt

