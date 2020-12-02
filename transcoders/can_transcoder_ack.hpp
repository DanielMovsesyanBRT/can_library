/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 12/1/2020 17:23:35
 * File : can_transcoder_ack.hpp
 *
 */

#pragma once

#include "../can_message.hpp"

namespace brt {
namespace can {

/**
 * \class CanTranscoderAck
 *
 * \brief <description goes here>
 */
class CanTranscoderAck 
{
public:
  enum Value
  {
    Ack = 0,
    Nack = 1,
    AccessDenied = 2, 
    CannotRespond = 3
  };
  // TODO: extra Ack Nack values


  explicit CanTranscoderAck(uint8_t value, uint8_t address, uint32_t pgn,uint8_t group_function = 0xFF);
  explicit CanTranscoderAck(const CanMessagePtr&);
  
  CanTranscoderAck(const CanTranscoderAck&) = default;
  CanTranscoderAck& operator=(const CanTranscoderAck&) = default;

  virtual ~CanTranscoderAck() {}

          CanMessagePtr           create_message() const;

          uint8_t                 value() const { return _value; }
          uint8_t                 group_function() const { return _group_function; }
          uint8_t                 address() const { return _address; }
          uint32_t                pgn() const { return _pgn; }

private:
  uint8_t                         _value;
  uint8_t                         _group_function;
  uint8_t                         _address;
  uint32_t                        _pgn;
};

} // can
} // brt

