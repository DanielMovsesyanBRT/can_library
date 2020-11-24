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

// ISO 11783-3:2018 5.13.3:
// The required time interval between packets of a multi-packet broadcast message is 10 ms to 200 ms
#define BAM_TP_MINIMUM_TIMEOUT              (10) // ms

// ISO 11783-3:2018 timeouts
#define TRANSPORT_TIMEOUT_Tr                (200) // ms
#define TRANSPORT_TIMEOUT_Th                (500) // ms
#define TRANSPORT_TIMEOUT_T1                (750) // ms
#define TRANSPORT_TIMEOUT_T2                (1250) // ms
#define TRANSPORT_TIMEOUT_T3                (1250) // ms
#define TRANSPORT_TIMEOUT_T4                (1050) // ms



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
 * @param  session : TransmitSession* 
 * @param  <uint8_t : std::pair
 * @param  range : uint8_t> 
 */
SendData::SendData(TransmitSession* session,std::pair<uint8_t,uint8_t> range) 
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
 * @param  session : TransmitSession* 
 */
WaitCTS::WaitCTS(TransmitSession* session) 
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
 *************************** TransmitSession
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
 *************************** TransmitSession
 */

/**
 * \fn  TransmitSession::update
 *
 * @return  bool
 */
bool TransmitSession::update()
{
  std::lock_guard<Mutex> lock(*_mutex);
  if (!_action)
    return true;

  _action->update();
  return !_action;
}

/**
 * \fn  TransmitSession::pgn_received
 *
 * @param  packet : const CanPacket& 
 * @return  bool
 */
bool TransmitSession::pgn_received(const CanPacket& packet)
{
  std::lock_guard<Mutex> lock(*_mutex);
  if (!_action)
    return true;

  _action->pgn_received(packet);
  return !_action;
}

/**
 * \fn  TransmitSession::change_action
 *
 * @param  new_action : ActionPtr 
 */
void TransmitSession::change_action(ActionPtr old_action,ActionPtr new_action)
{
  std::lock_guard<Mutex> lock(*_mutex);
  if (_action == old_action)
    _action = new_action;
}

/**
 * \fn  TransmitSession::abort
 *
 * @param  reason : uint8_t 
 */
void TransmitSession::abort(uint8_t reason)
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
  { on_pg_callback(packet,bus_name); } );

  processor->register_pgn_receiver(PGN_TP_DT, [this](const CanPacket& packet,const std::string& bus_name) 
  { on_pg_callback(packet,bus_name); } );
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
  TransmitSessionPtr new_session = std::make_shared<TransmitSession>(processor(), &_mutex,  message,local, remote, bus_name);
  if (_active_session.find(new_session) == _active_session.end())
    _active_session.insert(new_session);
  else
    _session_queue.push_back(new_session);
  
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

  std::vector<TransmitSessionPtr>  _removed;
  for (auto iter = _active_session.begin(); iter != _active_session.end(); )
  {
    if ((*iter)->update())
    {
      _removed.push_back(*iter);
      iter = _active_session.erase(iter);
    }
    else
      iter++;
  }

  while (!_removed.empty())
  {
    for (auto queue = _session_queue.begin(); queue != _session_queue.end();)
    {
      if(TransmitSession::hash()(*queue) == TransmitSession::hash()(_removed.front()))
      {
        _active_session.insert(*queue);
        queue = _session_queue.erase(queue);
        break;
      }
      else
        queue++;
    }
    _removed.erase(_removed.begin());
  }
  return false;
}

/**
 * \fn  CanTransportProtocol::on_pg_callback
 *
 * @param  packet : const CanPacket& 
 * @param  & bus_name : const std::string
 */
void CanTransportProtocol::on_pg_callback(const CanPacket& packet,const std::string& bus_name)
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
        auto iter = std::find_if(_active_session.begin(),_active_session.end(),
          [&](TransmitSessionPtr session)->bool
          {
            return TransmitSession::hash()(session) == TransmitSession::hash()(local,remote,bus_name);
          });
        
        if (iter != _active_session.end())
          (*iter)->pgn_received(packet);
      }
      break;

    default:
      break;
    }
  }
}

} // can
} // brt


