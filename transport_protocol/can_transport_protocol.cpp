/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/23/2020 11:47:59
 * File : can_transport_protocol.cpp
 *
 */
    
#include "can_transport_protocol.hpp"  
#include "can_processor.hpp"

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
  
  processor->register_pgn_receiver(PGN_TP_CM, [this](const CanPacket& packet,const std::string& bus_name) 
  { on_pgn_callback(packet,bus_name); } );

  processor->register_pgn_receiver(PGN_TP_DT, [this](const CanPacket& packet,const std::string& bus_name) 
  { on_pgn_callback(packet,bus_name); } );
}

/**
 * \fn  destructor CanTransportProtocol::~CanTransportProtocol
 *
 * \brief <description goes here>
 */
CanTransportProtocol::~CanTransportProtocol()
{

}

/**
 * \fn  CanTransportProtocol::send_message
 *
 * @param  message : CanMessagePtr 
 * @param  local :  LocalECUPtr 
 * @param  remote :  RemoteECUPtr 
 * @param  & bus_name :  const std::string
 * @return  bool
 */
bool CanTransportProtocol::send_message(CanMessagePtr message, LocalECUPtr local, RemoteECUPtr remote, const std::string& bus_name)
{
  if ((message->length() <= 8) || (message->length() > 1785))
    return false;

  std::lock_guard<Mutex> lock(_mutex);
  _session_stack[eTransmit].add(std::make_shared<TransportSession>(processor(), &_mutex, message, local, remote, bus_name));
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
 * \fn  CanTransportProtocol::on_pg_callback
 *
 * @param  packet : const CanPacket& 
 * @param  & bus_name : const std::string
 */
void CanTransportProtocol::on_pgn_callback(const CanPacket& packet,const std::string& bus_name)
{
  if (packet.dlc() < 8)
    return;

  LocalECUPtr local = std::dynamic_pointer_cast<LocalECU>(processor()->device_db().get_ecu_by_address(packet.da(),bus_name));
  RemoteECUPtr remote = std::dynamic_pointer_cast<RemoteECU>(processor()->device_db().get_ecu_by_address(packet.da(),bus_name));

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

    default:
      break;
    }
  }
}

} // can
} // brt


