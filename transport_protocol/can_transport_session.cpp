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

    _processor->send_can_message(msg, local(), remote(), { _bus_name });
  }
  on_abort();
}


} // can
} // brt


