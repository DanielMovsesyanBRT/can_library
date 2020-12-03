/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/23/2020 11:47:59
 * File : can_transport_protocol.cpp
 *
 */
    
#include "can_transport_protocol.hpp"  
#include "can_processor.hpp"

#include "can_transport_txsession.hpp"
#include "can_transport_rxsession.hpp"

#include <algorithm>
#include <mutex>

namespace brt {
namespace can {


/**
 * \fn  constructor CanTransportProtocol::CanTransportProtocol
 *
 * @param  processor : CanProcessor* 
 */
CanTransportProtocol::CanTransportProtocol(CanProcessor* processor)
: CanProtocol(processor)
, _mutex(processor)
{
  processor->register_updater([this]()->bool { return on_update(); });
  
  processor->register_pgn_receiver(PGN_TP_CM, [this](const CanPacket& packet,const ConstantString& bus_name)
  { on_pgn_callback(packet,bus_name); } );

  processor->register_pgn_receiver(PGN_TP_DT, [this](const CanPacket& packet,const ConstantString& bus_name)
  { on_pgn_callback(packet,bus_name); } );
}

/**
 * \fn  destructor CanTransportProtocol::~CanTransportProtocol
 *
 * \brief <description goes here>
 */
CanTransportProtocol::~CanTransportProtocol()
{
  _session_stack[eTransmit]._session_queue.clear();
  _session_stack[eReceive]._session_queue.clear();
}

/**
 * \fn  CanTransportProtocol::send_message
 *
 * @param   message : const CanMessagePtr&
 * @param   local : const LocalECUPtr&
 * @param   remote : const RemoteECUPtr&
 * @param   bus_name :  const ConstantString&
 * @return  bool
 */
bool CanTransportProtocol::send_message(const CanMessagePtr& message,const LocalECUPtr& local,
                              const RemoteECUPtr& remote, const ConstantString& bus_name)
{
  if ((message->length() <= 8) || (message->length() > 1785))
    return false;

  std::lock_guard<Mutex> lock(_mutex);
  _session_stack[eTransmit].add(TxSessionPtr(processor(), &_mutex, message, local, remote, bus_name));
  return true;
}

/**
 * \fn  CanTransportProtocol::on_update
 *
 * @return  bool
 */
bool CanTransportProtocol::on_update()
{
  std::lock_guard<Mutex> lock(_mutex);
  _session_stack[eTransmit].update();
  _session_stack[eReceive].update();
  return false;
}

/**
 * \fn  CanTransportProtocol::on_pgn_callback
 *
 * @param   packet : const CanPacket&
 * @param   bus_name : const ConstantString&
 */
void CanTransportProtocol::on_pgn_callback(const CanPacket& packet,const ConstantString& bus_name)
{
  if (packet.dlc() < 8)
    return;

  LocalECUPtr local(processor()->device_db().get_ecu_by_address(packet.da(),bus_name));
  RemoteECUPtr remote(processor()->device_db().get_ecu_by_address(packet.sa(),bus_name));

  if (packet.pgn() == PGN_TP_CM)
  {
    switch (packet.data()[0])
    {
    case CTS:
    case EOM:
      {
        std::lock_guard<Mutex> lock(_mutex);
        TransportSessionPtr session = _session_stack[eTransmit].get_active(local, remote, bus_name);
        if (session)
          session->pgn_received(packet);
      }
      break;

    case Abort:
      switch (packet.data()[1])
      {
      case AbortCTSWhileSending:
      case AbortTimeout:
        {
          std::lock_guard<Mutex> lock(_mutex);
          TransportSessionPtr session = _session_stack[eTransmit].get_active(local, remote, bus_name);
          if (session)
            session->abort(AbortIgnoreMessage);
        }
        break;
      default:
        {
          std::lock_guard<Mutex> lock(_mutex);
          TransportSessionPtr session = _session_stack[eReceive].get_active(remote, local, bus_name);
          if (session)
            session->abort(AbortIgnoreMessage);
        }
        break;
      }

      break;

    case RTS:
    case BAM:
      {
        std::lock_guard<Mutex> lock(_mutex);
        TransportSessionPtr session = _session_stack[eReceive].get_active(remote, local, bus_name);
        if (session)
          // Ignoring this session
          break;
        _session_stack[eReceive].add(RxSessionPtr(processor(), &_mutex, remote, local, bus_name, packet));
      }
      break;

    default:
      break;
    }
  }
  else if (packet.pgn() == PGN_TP_DT)
  {
    std::lock_guard<Mutex> lock(_mutex);
    TransportSessionPtr session = _session_stack[eReceive].get_active(remote, local, bus_name);
    if (session)
      session->pgn_received(packet);
  }
}

} // can
} // brt


