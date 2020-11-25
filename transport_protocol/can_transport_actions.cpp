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
 * \fn  constructor TxAction::TxAction
 *
 * @param  session : TxSession* 
 */
TxAction::TxAction(TxSession* session) : Action(session) 
{

}

/**
 * \fn  TxAction::session
 *
 * @return  TxSession*
 */
TxSession* TxAction::session()
{ 
  return dynamic_cast<TxSession*>(Action::session()); 
}

/**
 * \fn  SendBAM::update
 *
 */
void SendBAM::update()
{
  uint16_t total_size = static_cast<uint16_t>(session()->message()->length());
  uint8_t  num_packets = static_cast<uint8_t>((total_size - 1) / 7 + 1);

  auto me = getptr();
  if (!session()->send_bam([me, num_packets](uint64_t,const std::string& bus_name,bool success)
    {
      if (success)
      {
        me->session()->change_action(me, std::make_shared<SendData>(me->session(),
                  std::pair<uint8_t,uint8_t>(static_cast<uint8_t>(0), num_packets)));
      }   
      else
        me->session()->change_action(me, ActionPtr());

    }))
  {
    session()->change_action(me, ActionPtr());
  }
}

/**
 *************************** SendData
 */

/**
 * \fn  constructor SendData::SendData
 *
 * @param  session : TransportSession* 
 * @param  range : std::pair<uint8_t,uint8_t>
 */
SendData::SendData(TxSession* session,std::pair<uint8_t,uint8_t> range) 
  : TxAction(session), _range(range), _current(range.first) 
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

  if (!session()->send_data(_current, [me](uint64_t,const std::string& bus_name,bool success)
    {
      // Callback for message sent
      if (success)
        me->session()->update();
      else
        me->session()->change_action(me, ActionPtr());
    }))
  {
    session()->change_action(me, ActionPtr());
  }
  else
  {
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
  }
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
  auto me = getptr();
  if (!session()->send_rts([me](uint64_t,const std::string& bus_name,bool success)
    {
      if (success)
        me->session()->change_action(me, std::make_shared<WaitCTS>(me->session()));
      else
        me->session()->change_action(me, ActionPtr());
      
      me->session()->update();
    }))
  {
    session()->change_action(getptr(), ActionPtr());
  }
}

/**
 *************************** WaitCTS
 */

/**
 * \fn  constructor WaitCTS::WaitCTS
 *
 * @param  session : TransportSession* 
 */
WaitCTS::WaitCTS(TxSession* session) 
: TxAction(session)
, _time_tag(session->processor()->get_time_tick()) 
, _timeout_value(TRANSPORT_TIMEOUT_T3)
{  }
/**
 * \fn  WaitCTS::update
 *
 */
void WaitCTS::update()
{
  if ((session()->processor()->get_time_tick() - _time_tag) > _timeout_value)
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
    if (num_packets == 0)
    {
      // Receiver is stalling transmission
      _time_tag = session()->processor()->get_time_tick();
      _timeout_value = TRANSPORT_TIMEOUT_T4;
    }
    else
    {
      uint8_t next_packet = packet.data()[2];
      session()->change_action(getptr(), std::make_shared<SendData>(session(),
                std::pair<uint8_t,uint8_t>(static_cast<uint8_t>(num_packets), static_cast<uint8_t>(num_packets + next_packet))));
    }
  }
  else if (packet.data()[0] == Abort)
  {
    session()->change_action(getptr(), ActionPtr());
  }
}

/**
 *************************** WaitEOM
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


/**
 *************************** RxAction
 */

/**
 * \fn  constructor RxAction::RxAction
 *
 * @param  session : RxSession* 
 */
RxAction::RxAction(RxSession* session) 
: Action(session) 
{

}

/**
 * \fn  RxAction::session
 *
 * @return  RxSession*
 */
RxSession* RxAction::session()
{ 
  return dynamic_cast<RxSession*>(Action::session()); 
}

/**
 *************************** ReceiveData
 */



/**
 * \fn  constructor ReceiveData::ReceiveData
 *
 * @param  session : RxSession* 
 * @param  range : std::pair<uint8_t,uint8_t> 
 */
ReceiveData::ReceiveData(RxSession* session,std::pair<uint8_t,uint8_t> range)
: RxAction(session)
, _range(range)
, _current(range.first)
, _time_tag(session->processor()->get_time_tick())
, _timeout_value(TRANSPORT_TIMEOUT_T2)
, _attempts(0)
{  

}

/**
 * \fn  ReceiveData::update
 *
 */
void ReceiveData::update()
{
  if ((session()->processor()->get_time_tick() - _time_tag) > _timeout_value)
  {
    // Timeout occured we are Aborting reception
    if (session()->is_broadcast())
      session()->change_action(getptr(), ActionPtr());
    else if (++_timeout_value > MAX_CTS_ATTEMPTS)
      session()->abort(AbortMaxTxRequestLimit);
    else
    {
      session()->send_cts();
      _time_tag = session()->processor()->get_time_tick();
      _timeout_value = TRANSPORT_TIMEOUT_T2;
    }
  }
}

/**
 * \fn  ReceiveData::pgn_received
 *
 * @param  packet : const CanPacket& 
 */
void ReceiveData::pgn_received(const CanPacket& packet)
{
  uint8_t sequence_number = packet.data()[0];
  uint8_t data_bytes[7];
  memcpy(data_bytes, &packet.data()[1], 7);

  _timeout_value = TRANSPORT_TIMEOUT_T1;

  if (session()->sequence_received(sequence_number, data_bytes))
  {
    if (session()->is_complete())
    {
      if (!session()->is_broadcast())
        session()->send_eom();

      session()->message_complete();
    }
    else 
    {
      if (!session()->is_broadcast() && session()->is_range_complete(_range))
      {
        session()->send_cts();
        _timeout_value = TRANSPORT_TIMEOUT_T2;
      }
    }
  }

  _time_tag = session()->processor()->get_time_tick();
}


} // can
} // brt

