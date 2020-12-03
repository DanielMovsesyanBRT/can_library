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
#include "can_transcoder.hpp"

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

          uint8_t                 get_address(const ConstantString& bus_name) const;

protected:
          CanProcessor*           processor() { return _processor; }
          
          bool                    set_pgn_transcoder(const CanTranscoderPtr&);
          CanTranscoderPtr        get_pgn_transcoder(uint32_t pgn) const;
private:
  CanProcessor*                   _processor;
  CanName                         _name;

private:

  fixed_list<CanTranscoderPtr,6>  _trans_map;
};

typedef shared_pointer<CanECU>   CanECUPtr;

} // can
} // brt

