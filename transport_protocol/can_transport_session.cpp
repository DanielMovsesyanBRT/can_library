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
  if (!is_broadcast())
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

    _processor->send_can_message(msg, _local, _remote);
  }
  
  std::lock_guard<Mutex> lock(*_mutex);
  _action.reset();
}


} // can
} // brt


