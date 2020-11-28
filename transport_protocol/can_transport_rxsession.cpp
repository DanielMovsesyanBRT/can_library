/**
 * can_transport_session.cpp
 * 
 */

#include "can_transport_rxsession.hpp"
#include "../can_processor.hpp"

namespace brt {
namespace can {

allocator<RxSession,0,32> RxSession::_allocator;

/**
 * \fn  constructor RxSession::RxSession
 *
 * @param  processor : CanProcessor* 
 * @param  mutex :  Mutex* 
 * @param  source :  CanECUPtr 
 * @param  destination : CanECUPtr 
 * @param  & bus_name :  const std::string
 * @param  packet :  const CanPacket& 
 */
RxSession::RxSession(CanProcessor* processor, Mutex* mutex, CanECUPtr source,CanECUPtr destination,
                              const std::string& bus_name, const CanPacket& packet)
: TransportSession(processor, mutex, CanMessagePtr(), source, destination, bus_name)
, _range()
, _current(0)
, _time_tag(processor->get_time_tick())
, _timeout_value(TRANSPORT_TIMEOUT_T2)
, _attempts(0)
, _complete(false)
{
  uint32_t pgn  = packet.data()[5] | (packet.data()[6] << 8) | (packet.data()[7] << 16);
  uint32_t size = packet.data()[1] | (packet.data()[2] << 8);

  _max_packets = packet.data()[4];
  if (_max_packets == 0xFF)
    _max_packets = static_cast<uint8_t>((size - 1) / 7 + 1);

  _received_map.fill(false);

  if (size > MAX_TP_DATA_SIZE)
    abort(AbortSizeToBig);
}


/**
 * \fn  RxSession::update
 *
 */
void RxSession::update()
{
  if ((processor()->get_time_tick() - _time_tag) > _timeout_value)
  {
    // Timeout occured we are Aborting reception
    if (is_broadcast())
      _complete = true;
    else if (++_timeout_value > MAX_CTS_ATTEMPTS)
    {
      abort(AbortMaxTxRequestLimit);
      _complete = true;
    }
    else
    {
      send_cts();
      _time_tag = processor()->get_time_tick();
      _timeout_value = TRANSPORT_TIMEOUT_T2;
    }
  }
}

/**
 * \fn  RxSession::pgn_received
 *
 * @param  packet : const CanPacket& 
 */
void RxSession::pgn_received(const CanPacket& packet)
{
  uint8_t sequence_number = packet.data()[0];
  uint8_t data_bytes[7];
  memcpy(data_bytes, &packet.data()[1], 7);

  _timeout_value = TRANSPORT_TIMEOUT_T1;

  if (sequence_received(sequence_number, data_bytes))
  {
    if (is_range_complete(range(0,num_sequences()))) 
    {
      if (!is_broadcast())
        send_eom();

      message_complete();
    }
    else 
    {
      if (!is_broadcast() && is_range_complete(_range))
      {
        send_cts();
        _timeout_value = TRANSPORT_TIMEOUT_T2;
      }
    }
  }

  _time_tag = processor()->get_time_tick();
}

/**
 * \fn  RxSession::sequence_received
 *
 * @param  sequence : uint8_t 
 * @param  bytes :  const uint8_t[7]
 * @return  bool
 */
bool RxSession::sequence_received(uint8_t sequence, const uint8_t bytes[7])
{
  uint32_t offset = (sequence - 1) * 7;
  if (offset >= _message->length())
  {
    abort(AbortBadSequenceNumber);
    return false;
  }

  if (_received_map[sequence])
  {
    abort(AbortDupSequenceNumber);
    return false;
  }

  size_t num_bytes = std::min(_message->length() - offset, 7u);
  memcpy(_message->data() + offset, bytes, num_bytes);
  _received_map[sequence] = true;
  return true;
}

/**
 * \fn  RxSession::is_range_complete
 *
 * @param  <uint8_t : const std::pair
 * @param  range : uint8_t>& 
 * @return  bool
 */
bool RxSession::is_range_complete(const range& range) const
{
  for (size_t index = range.first; index < range.second; index++)
  {
    if (!_received_map[index])
      return false;
  }
  return true;
}

/**
 * \fn  RxSession::send_cts
 *
 * @return  bool
 */
bool RxSession::send_cts()
{
  uint8_t starting_sequence = 255;
  uint8_t num_sequences = static_cast<uint8_t>((_message->length() - 1) / 7 + 1);

  for (size_t index = 0; ((index < num_sequences) && (starting_sequence != 255)); index++)
  {
    if (!_received_map[index])
      starting_sequence = index;
  }

  if (starting_sequence == 255)
    return false;

  uint8_t max_packets = std::min(static_cast<uint8_t>(num_sequences - starting_sequence),_max_packets);

  CanMessagePtr msg( 
  {
    static_cast<uint8_t>(CTS), 
    max_packets,
    static_cast<uint8_t>(starting_sequence + 1),
    0xFF, 0xFF,
    static_cast<uint8_t>(message()->pgn() & 0xFF),
    static_cast<uint8_t>((message()->pgn() >> 8) & 0xFF),
    static_cast<uint8_t>((message()->pgn() >> 16) & 0xFF),
  }, PGN_TP_CM, 7);
  
  return processor()->send_can_message(msg, local(), remote(), { bus_name() });
}

/**
 * \fn  RxSession::send_eom
 *
 * @return  bool
 */
bool RxSession::send_eom()
{
  uint32_t total_size = message()->length();
  uint8_t num_packets = static_cast<uint8_t>((total_size - 1) / 7 + 1);

  CanMessagePtr msg( 
    {
      static_cast<uint8_t>(EOM), 
      static_cast<uint8_t>(total_size & 0xFF),
      static_cast<uint8_t>((total_size >> 8) & 0xFF),
      num_packets,
      0xFF,
      static_cast<uint8_t>(message()->pgn() & 0xFF),
      static_cast<uint8_t>((message()->pgn() >> 8) & 0xFF),
      static_cast<uint8_t>((message()->pgn() >> 16) & 0xFF),
    }, PGN_TP_CM, 7);

  
  return processor()->send_can_message(msg, local(), remote(), { bus_name()});
}

/**
 * \fn  RxSession::message_complete
 *
 */
void RxSession::message_complete()
{
  processor()->message_received(message(), local(), remote(), bus_name());
  _complete = true;
}

} // can
} // brt



