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

// #include <memory>
// #include <unordered_map>

namespace brt {
namespace can {

class CanProcessor;
/**
 * \class CanECU
 *
 */
//class CanECU : std::enable_shared_from_this<CanECU>
class CanECU : public shared_class<CanECU>
{
public:
  CanECU(CanProcessor*,const CanName& name = CanName());
  virtual ~CanECU();

          CanProcessor*           processor() { return _processor; }
//          std::shared_ptr<CanECU> getptr() { return shared_from_this(); }
          const CanName&          name() const { return _name; }

          uint8_t                 get_address(const std::string& bus_name) const;

private:
  CanProcessor*                   _processor;
  CanName                         _name;
};

//typedef std::shared_ptr<CanECU>   CanECUPtr;
typedef shared_pointer<CanECU>   CanECUPtr;

} // can
} // brt

