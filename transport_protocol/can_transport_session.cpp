/**
 * can_transport_session.cpp
 * 
 */

#include "can_transport_session.hpp"
#include "can_transport_protocol.hpp"

#include "../can_processor.hpp"

#include <mutex>

namespace brt {
namespace can {


/**
 * \fn  TransportSession::update
 *
 * @return  bool
 */
bool TransportSession::update()
{
  std::lock_guard<Mutex> lock(*_mutex);
  if (!_action)
    return true;

  _action->update();
  return !_action;
}

/**
 * \fn  TransportSession::pgn_received
 *
 * @param  packet : const CanPacket& 
 * @return  bool
 */
bool TransportSession::pgn_received(const CanPacket& packet)
{
  std::lock_guard<Mutex> lock(*_mutex);
  if (!_action)
    return true;

  _action->pgn_received(packet);
  return !_action;
}

/**
 * \fn  TransportSession::change_action
 *
 * @param  new_action : ActionPtr 
 */
void TransportSession::change_action(ActionPtr old_action,ActionPtr new_action)
{
  std::lock_guard<Mutex> lock(*_mutex);
  if (_action == old_action)
    _action = new_action;
}

/**
 * \fn  TransportSession::abort
 *
 * @param  reason : uint8_t 
 */
void TransportSession::abort(uint8_t reason)
{
  if (!is_broadcast() && (reason != AbortIgnoreMessage))
  {
    CanMessagePtr msg( 
      {
        static_cast<uint8_t>(Abort), 
        static_cast<uint8_t>(reason),
        0xFF, 0xFF, 0xFF,
        static_cast<uint8_t>(_message->pgn() & 0xFF),
        static_cast<uint8_t>((_message->pgn() >> 8) & 0xFF),
        static_cast<uint8_t>((_message->pgn() >> 16) & 0xFF),
      }, PGN_TP_CM, 7);

    _processor->send_can_message(msg, local(), remote());
  }
  
  std::lock_guard<Mutex> lock(*_mutex);
  _action.reset();
}

/****************************
 * 
 *    TxSession
 * 
 ****************************/ 

/**
 * \fn  TxSession::send_bam
 *
 * @param   cback : CanMessage::ConfirmationCallback
 * @return  bool
 */
bool TxSession::send_bam(CanMessage::ConfirmationCallback cback 
                                  /*= CanMessage::ConfirmationCallback()*/)
{
  uint16_t total_size = static_cast<uint16_t>(message()->length());
  uint8_t  num_packets = static_cast<uint8_t>((total_size - 1) / 7 + 1);

  CanMessagePtr msg( 
    {
      static_cast<uint8_t>(BAM), 
      static_cast<uint8_t>(total_size & 0xFF),
      static_cast<uint8_t>((total_size >> 8) & 0xFF),
      num_packets,
      0xFF,
      static_cast<uint8_t>(message()->pgn() & 0xFF),
      static_cast<uint8_t>((message()->pgn() >> 8) & 0xFF),
      static_cast<uint8_t>((message()->pgn() >> 16) & 0xFF),
    }, PGN_TP_CM, 7, cback);

  return processor()->send_can_message(msg, local(), RemoteECUPtr(),{ bus_name() });
}

/**
 * \fn  TxSession::send_data
 *
 * @param  sequence : uint8_t 
 * @param   cback :  CanMessage::ConfirmationCallback
 * @return  bool
 */
bool TxSession::send_data(uint8_t sequence, CanMessage::ConfirmationCallback cback 
                            /*= CanMessage::ConfirmationCallback()*/)
{
  uint32_t offset = (sequence * 7);
  if (offset >= message()->length())
    return false;

  uint32_t num_bytes = std::min(message()->length() - offset, 7U);
  std::array<uint8_t,8> data;
  data.fill(0xFF);
  
  data[0] = (sequence + 1);
  memcpy(&data[1], &message()->data()[offset], num_bytes);

  CanMessagePtr msg(data.data(), data.size(),  PGN_TP_DT, 7, cback);
  return processor()->send_can_message(msg, local(), remote(), { bus_name() });
}

/**
 * \fn  TxSession::send_rts
 *
 * @param   cback : CanMessage::ConfirmationCallback
 * @return  bool
 */
bool TxSession::send_rts( CanMessage::ConfirmationCallback cback 
                  /*= CanMessage::ConfirmationCallback()*/)
{
  uint16_t total_size = static_cast<uint16_t>(message()->length());
  uint8_t  num_packets = static_cast<uint8_t>((total_size - 1) / 7 + 1);

  CanMessagePtr msg( 
    {
      static_cast<uint8_t>(RTS), 
      static_cast<uint8_t>(total_size & 0xFF),
      static_cast<uint8_t>((total_size >> 8) & 0xFF),
      num_packets,
      0xFF,
      static_cast<uint8_t>(message()->pgn() & 0xFF),
      static_cast<uint8_t>((message()->pgn() >> 8) & 0xFF),
      static_cast<uint8_t>((message()->pgn() >> 16) & 0xFF),
    }, PGN_TP_CM, 7, cback);

  return processor()->send_can_message(msg, local(), remote(), { bus_name() });
}

/****************************
 * 
 *    RxSession
 * 
 ****************************/ 

RxSession::RxSession(CanProcessor* processor, Mutex* mutex, CanECUPtr source,CanECUPtr destination,
                              const std::string& bus_name, const CanPacket& packet)
: TransportSession(processor, mutex, CanMessagePtr(), source, destination, bus_name)
{
  uint32_t pgn  = packet.data()[5] | (packet.data()[6] << 8) | (packet.data()[7] << 16);
  uint32_t size = packet.data()[1] | (packet.data()[2] << 8);

  _max_packets = packet.data()[4];
  if (_max_packets == 0xFF)
    _max_packets = static_cast<uint8_t>((size - 1) / 7 + 1);

  _action = std::make_shared<ReceiveData>(this, std::pair<uint8_t,uint8_t>(0, _max_packets));
  _received_map.fill(false);

  if (size > MAX_TP_DATA_SIZE)
    abort(AbortSizeToBig);
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
 * \fn  RxSession::is_complete
 *
 * @return  bool
 */
bool RxSession::is_complete() const
{
  size_t num_sequences = (_message->length() - 1) / 7 + 1;
  for (size_t index = 0; index < num_sequences; index++)
  {
    if (!_received_map[index])
      return false;
  }

  return true;
}

/**
 * \fn  RxSession::is_range_complete
 *
 * @param  <uint8_t : const std::pair
 * @param  range : uint8_t>& 
 * @return  bool
 */
bool RxSession::is_range_complete(const std::pair<uint8_t,uint8_t>& range) const
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
  _action.reset();
}

} // can
} // brt


