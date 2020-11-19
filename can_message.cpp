/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 12:25:5
 * File : can_message.cpp
 *
 */
    
#include "can_message.hpp"  

namespace brt {
namespace can {

std::atomic_uint64_t CanMessage::_unique_counter(0ULL);

/**
 * \fn  constructor CanMessage::CanMessage
 *
 */
CanMessage::CanMessage()
: _pgn(0)
, _sa(0)
, _da(0)
, _priority(0)
, _packet_id(_unique_counter++)
{

}

/**
 * \fn  constructor CanMessage::CanMessage
 *
 * @param  id : uint32_t 
 * @param  data :  const uint8_t* 
 * @param  length :  uint32_t 
 */
CanMessage::CanMessage(uint32_t id, const uint8_t* data, uint32_t length)
{
  // Check PDU format
  _priority = static_cast<uint8_t>((id >> 26) & 7);
  _sa = static_cast<uint8_t>(id & 0xFF);
  uint8_t pf = static_cast<uint8_t>((id >> 16) & 0xFF);
  if (pf < 240)
  {
    _pgn = (id >> 8) & 0x3FF00;
    _da = static_cast<uint8_t>((id >> 8) & 0xFF);
  }
  else
  {
    _pgn = (id >> 8) & 0x3FFFF;
    _da = BROADCATS_CAN_ADDRESS;
  }

  if ((data != nullptr) && (length > 0))
    _data.assign(data, data + length);

  _packet_id = _unique_counter++;
}

/**
 * \fn  constructor CanMessage::CanMessage
 *
 * @param  data : const std::vector<uint8_t>&
 * @param  pgn :  uint32_t 
 * @param  da :  uint8_t 
 * @param  sa :  uint8_t 
 * @param  priority :  uint8_t 
 */
CanMessage::CanMessage(const std::vector<uint8_t>& data, uint32_t pgn, uint8_t da, uint8_t sa, uint8_t priority /*= DEFAULT_CAN_PRIORITY*/)
: _pgn(pgn)
, _sa(sa)
, _da(da)
, _priority(priority)
, _data(data)
, _packet_id(_unique_counter++)
{  }

/**
 * \fn  destructor CanMessage::~CanMessage
 *
 */
CanMessage::~CanMessage()
{

}

/**
 * \fn  CanMessage::id
 *
 * @return  uint32_t
 */
uint32_t CanMessage::id() const
{
  uint32_t id = (_priority & 7) << 26 | _sa;
  if (is_pdu1())
    id |= ((_pgn & 0x3FF00) << 8) | (_da << 8);
  
  else
    id |= _pgn << 8;

  return id;
}

} // can
} // brt

