/**
 * can_tp_transmit_actions.hpp
 * 
 */

#include "can_transport_actions.hpp"
#include "can_transport_session.hpp"
#include "can_transport_protocol.hpp"

#include "../can_processor.hpp"

namespace brt {
namespace can {


/**
 * \fn  SendBAM::update
 *
 */
void SendBAM::update()
{
  uint16_t total_size = static_cast<uint16_t>(session()->message()->length());
  uint8_t  num_packets = static_cast<uint8_t>((total_size - 1) / 7 + 1);

  auto me = getptr();

  CanMessagePtr msg( 
    {
      static_cast<uint8_t>(BAM), 
      static_cast<uint8_t>(total_size & 0xFF),
      static_cast<uint8_t>((total_size >> 8) & 0xFF),
      num_packets,
      0xFF,
      static_cast<uint8_t>(session()->message()->pgn() & 0xFF),
      static_cast<uint8_t>((session()->message()->pgn() >> 8) & 0xFF),
      static_cast<uint8_t>((session()->message()->pgn() >> 16) & 0xFF),
    }, PGN_TP_CM, 7,
    [me, num_packets](uint64_t,const std::string& bus_name,bool success)
    {
      if (success)
      {
        me->session()->change_action(me, std::make_shared<SendData>(me->session(),
                  std::pair<uint8_t,uint8_t>(static_cast<uint8_t>(0), static_cast<uint8_t>(num_packets))));
      }   
      else
        me->session()->change_action(me, ActionPtr());

    });


  if (!session()->processor()->send_can_message(msg, session()->local(), RemoteECUPtr(), std::vector<std::string>(1, session()->bus_name())))
    session()->change_action(me, ActionPtr());
}

/**
 *************************** SendData
 */

/**
 * \fn  constructor SendData::SendData
 *
 * @param  session : TransportSession* 
 * @param  <uint8_t : std::pair
 * @param  range : uint8_t> 
 */
SendData::SendData(TransportSession* session,std::pair<uint8_t,uint8_t> range) 
  : Action(session), _range(range), _current(range.first) 
{
  _time_tag = session->processor()->get_time_tick();
}

/**
 * \fn  SendData::update
 *
 */
void SendData::update()
{
  if (session()->is_broadcast())
  {
    if ((session()->processor()->get_time_tick() - _time_tag) < BAM_TP_MINIMUM_TIMEOUT)
      return;
  }

  auto me = getptr();
  uint32_t offset = (_current * 7);
  uint32_t num_bytes = std::min(session()->message()->length() - offset, 7U);

  std::vector<uint8_t> data(8, 0xFF);
  
  data[0] = (_current + 1);
  memcpy(&data[1], &session()->message()->data()[offset], num_bytes);

  CanMessagePtr msg(data, PGN_TP_DT, 7, 
    [me](uint64_t,const std::string& bus_name,bool success)
    {
      // Callback for message sent
      if (success)
        me->session()->update();
      else
        me->session()->change_action(me, ActionPtr());

    });

  _time_tag = session()->processor()->get_time_tick();

  if (++_current >= _range.second)
  {
    if (session()->is_broadcast())
    {
      session()->change_action(me, ActionPtr());
    }
    else
    {
      uint16_t total_size = static_cast<uint16_t>(session()->message()->length());
      uint8_t  num_packets = static_cast<uint8_t>((total_size - 1) / 7 + 1);
      
      if (_current < num_packets)
        session()->change_action(getptr(), std::make_shared<WaitCTS>(session()));
      else
        session()->change_action(getptr(), std::make_shared<WaitEOM>(session()));
    }
  }

  if (!session()->processor()->send_can_message(msg, session()->local(), session()->remote(), std::vector<std::string>(1, session()->bus_name())))
    session()->change_action(me, ActionPtr());
}

/**
 * \fn  SendData::pgn_received
 *
 * @param  packet : const CanPacket& 
 * @return  ActionPtr
 */
void SendData::pgn_received(const CanPacket& packet)
{
  if (session()->is_broadcast())
    return; // Not expecting anything at all

  if (packet.data()[0] == CTS)
    session()->abort(AbortCTSWhileSending);

}

/**
 *************************** SendRTS
 */

/**
 * \fn  SendRTS::update
 *
 */
void SendRTS::update()
{
  uint16_t total_size = static_cast<uint16_t>(session()->message()->length());
  uint8_t  num_packets = static_cast<uint8_t>((total_size - 1) / 7 + 1);

  auto me = getptr();
  CanMessagePtr msg( 
    {
      static_cast<uint8_t>(RTS), 
      static_cast<uint8_t>(total_size & 0xFF),
      static_cast<uint8_t>((total_size >> 8) & 0xFF),
      num_packets,
      0xFF,
      static_cast<uint8_t>(session()->message()->pgn() & 0xFF),
      static_cast<uint8_t>((session()->message()->pgn() >> 8) & 0xFF),
      static_cast<uint8_t>((session()->message()->pgn() >> 16) & 0xFF),
    }, PGN_TP_CM, 7,
    [me, num_packets](uint64_t,const std::string& bus_name,bool success)
    {
      if (success)
        me->session()->change_action(me, std::make_shared<WaitCTS>(me->session()));
      else
        me->session()->change_action(me, ActionPtr());
      
      me->session()->update();
    });

  if (!session()->processor()->send_can_message(msg, session()->local(), session()->remote()))
    session()->change_action(getptr(), ActionPtr());
}

/**
 *************************** WaitCTS
 */

/**
 * \fn  constructor WaitCTS::WaitCTS
 *
 * @param  session : TransportSession* 
 */
WaitCTS::WaitCTS(TransportSession* session) 
: Action(session)
, _time_tag(session->processor()->get_time_tick()) 
{  }
/**
 * \fn  WaitCTS::update
 *
 */
void WaitCTS::update()
{
  if ((session()->processor()->get_time_tick() - _time_tag) > TRANSPORT_TIMEOUT_T3)
    session()->abort(AbortTimeout);

}

/**
 * \fn  WaitCTS::pgn_received
 *
 * @param  packet : const CanPacket& 
 */
void WaitCTS::pgn_received(const CanPacket& packet)
{
  if (packet.data()[0] == CTS)
  {
    uint32_t pgn = packet.data()[5] | (packet.data()[6] << 8) | (packet.data()[7] << 8);
    if (pgn != session()->message()->pgn())
    {
      // todo: abort
      session()->change_action(getptr(), ActionPtr());
      return;
    }

    uint8_t num_packets = packet.data()[1];
    uint8_t next_packet = packet.data()[2];

    session()->change_action(getptr(), std::make_shared<SendData>(session(),
              std::pair<uint8_t,uint8_t>(static_cast<uint8_t>(num_packets), static_cast<uint8_t>(num_packets + next_packet))));
  }
  else if (packet.data()[0] == Abort)
  {
    session()->change_action(getptr(), ActionPtr());
  }
}

/**
 *************************** TransportSession
 */


/**
 * \fn  WaitEOM::update
 *
 */
void WaitEOM::update()
{
  if ((session()->processor()->get_time_tick() - _time_tag) > TRANSPORT_TIMEOUT_T3)
  {
    session()->change_action(getptr(), ActionPtr());
  }
}

/**
 * \fn  WaitEOM::pgn_received
 *
 * @param  packet : const CanPacket& 
 */
void WaitEOM::pgn_received(const CanPacket& packet)
{
  if (packet.data()[0] == CTS)
    WaitCTS::pgn_received(packet);
  else if (packet.data()[0] == EOM)
    session()->change_action(getptr(), ActionPtr());
}


} // can
} // brt

