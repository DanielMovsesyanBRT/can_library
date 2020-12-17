/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 12/1/2020 17:23:35
 * File : can_transcoder_ack.cpp
 *
 */
    
#include "can_transcoder_ack.hpp"  
#include "../can_utils.hpp"  

namespace brt {
namespace can {

/**
 * \fn  constructor CanTranscoderAck::CanTranscoderAck
 *
 * @param  value : uint8_t
 * @param  address :  uint8_t 
 * @param  pgn :  uint32_t 
 * @param  group_function : uint8_t 
 */
CanTranscoderAck::CanTranscoderAck(uint8_t value, uint8_t address, uint32_t pgn,uint8_t group_function /*= 0xFF*/)
: _value(value)
, _group_function(group_function)
, _address(address)
, _pgn(pgn)
{

}

/**
 * \fn  constructor CanTranscoderAck::CanTranscoderAck
 *
 * @param  message : const CanMessagePtr& 
 */
CanTranscoderAck::CanTranscoderAck(const CanMessagePtr& message)
{
  if (message->length() >= 8)
  {
    _value = message->data()[0];
    _group_function = message->data()[1];
    _address = message->data()[4];
    _pgn = can_unpack24(&message->data()[5]);
  }
}


/**
 * \fn  CanTranscoderAck::create_message
 *
 * @return  CanMessagePtr
 */
CanMessagePtr CanTranscoderAck::create_message() const
{
  uint8_t data[8] = {_value, _group_function, 0xFF, 0xFF, 0xFF,
    static_cast<uint8_t>(_pgn & 0xFF),
    static_cast<uint8_t>((_pgn >> 8) & 0xFF),
    static_cast<uint8_t>((_pgn >> 16) & 0xFF)
    };
  
  return CanMessagePtr(data, sizeof(data), PGN_AckNack);
}

} // can
} // brt

