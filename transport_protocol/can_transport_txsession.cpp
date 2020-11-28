/**
 * can_transport_txsession.cpp
 * 
 */

#include "can_transport_txsession.hpp"
#include "../can_processor.hpp"

#include <mutex>

namespace brt {
namespace can {

allocator<TxSession,0,32> TxSession::_allocator;
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

/**
 * \fn  TxSession::update
 *
 * @return  bool
 */
void TxSession::update()
{
  switch (_state)
  {
  case SendBAM:
    {
      uint16_t total_size = static_cast<uint16_t>(message()->length());
      uint8_t  num_packets = static_cast<uint8_t>((total_size - 1) / 7 + 1);

      auto me = dynamic_shared_cast<TxSession>(getptr());
      if (!send_bam([me, num_packets](uint64_t,const std::string& bus_name,bool success)
      /// Lambda begin
            {
              std::lock_guard<Mutex>   l(*(me->_mutex));
              if (me->_state != WaitDriverConfirmation) 
                return;

              if (success)
              {
                me->_range.first = static_cast<uint8_t>(0);
                me->_range.second = num_packets;
                me->_time_tag = me->processor()->get_time_tick();
                me->_state = SendData; 
              }   
              else
                me->_state = None;
            }))
      /// Lambda end
      {
        _state = None;
      }
      else
      {
        _time_tag = processor()->get_time_tick();
        _state = WaitDriverConfirmation;
      }
    }
    break;

  case SendData:
    {
      if (is_broadcast())
      {
        if ((processor()->get_time_tick() - _time_tag) < BAM_TP_MINIMUM_TIMEOUT)
          break;
      }

      auto me = dynamic_shared_cast<TxSession>(getptr());
      if (!send_data(_current, [me](uint64_t,const std::string& bus_name,bool success)
      /// Lambda begin
            {
              std::lock_guard<Mutex>   l(*(me->_mutex));
              if (me->_state != SendData) 
                return;

              // Callback for message sent
              if (success)
                me->update();
              else
                me->_state = None;
            }))
      /// Lambda end
      {
        _state = None;
      }
      else
      {
        _time_tag = processor()->get_time_tick();

        if (++_current >= _range.second)
        {
          if (is_broadcast())
            _state = None;
          else
          {
            uint16_t total_size = static_cast<uint16_t>(message()->length());
            uint8_t  num_packets = static_cast<uint8_t>((total_size - 1) / 7 + 1);
            
            _time_tag = processor()->get_time_tick();
            _timeout_value = TRANSPORT_TIMEOUT_T3;

            if (_current < num_packets)
              _state = WaitCTS;
            else
              _state = WaitEOM;
          }
        }
      }
    }
    break;

  case SendRTS:
    {
      auto me = dynamic_shared_cast<TxSession>(getptr());
      if (!send_rts([me](uint64_t,const std::string& bus_name,bool success)
      /// Lambda begin
            {
              std::lock_guard<Mutex>   l(*(me->_mutex));
              if (me->_state != WaitDriverConfirmation) 
                return;

              if (success)
              {
                me->_time_tag = me->processor()->get_time_tick();
                me->_timeout_value = TRANSPORT_TIMEOUT_T3;
                me->_state = WaitCTS;
              }
              else
                me->_state = None;
              
              me->update();
            }))
      /// Lambda end
      {
        _state = None;
      }
      else
      {
        _time_tag = processor()->get_time_tick();
        _state = WaitDriverConfirmation;
      }
    }
    break;

  case WaitCTS:
    if ((processor()->get_time_tick() - _time_tag) > _timeout_value)
      abort(AbortTimeout);
    break;

  case WaitEOM:
    if ((processor()->get_time_tick() - _time_tag) > _timeout_value)
      _state = None;
    break;

  case WaitDriverConfirmation:
    if ((processor()->get_time_tick() - _time_tag) >= 1000) // 1 second
      _state = None; // Abort
    break;

  default:
    break;
  }
}

/**
 * \fn  TxSession::pgn_received
 *
 * @param  packet : const CanPacket& 
 */
void TxSession::pgn_received(const CanPacket& packet)
{
  switch (_state)
  {
  case SendData:
    {
      if (is_broadcast())
      {
        if ((processor()->get_time_tick() - _time_tag) < BAM_TP_MINIMUM_TIMEOUT)
          break;
      }

      auto me = dynamic_shared_cast<TxSession>(getptr());

      if (!send_data(_current, [me](uint64_t,const std::string& bus_name,bool success)
      /// Lambda begin
            {
              std::lock_guard<Mutex>   l(*(me->_mutex));
              if (me->_state != SendData) 
                return;

              // Callback for message sent
              if (success)
                me->update();
              else
                me->_state = None;
            }))
      /// Lambda end
      {
        _state = None;
      }
      else
      {
        _time_tag = processor()->get_time_tick();
        if (++_current >= _range.second)
        {
          if (is_broadcast())
            _state = None;
          else
          {
            uint16_t total_size = static_cast<uint16_t>(message()->length());
            uint8_t  num_packets = static_cast<uint8_t>((total_size - 1) / 7 + 1);
            
            _timeout_value = TRANSPORT_TIMEOUT_T3;
            if (_current < num_packets)
              _state = WaitCTS;
            else
              _state = WaitEOM;
          }
        }
      }
    }
    break;
  case WaitCTS:
  case WaitEOM:
    { 
      if (packet.data()[0] == CTS)
      {
        uint32_t pgn = packet.data()[5] | (packet.data()[6] << 8) | (packet.data()[7] << 8);
        if (pgn != message()->pgn())
        {
          _state = None;
          abort(AbortDuplicateConnection);
          break;
        }

        uint8_t num_packets = packet.data()[1];
        if (num_packets == 0)
        {
          // Receiver is stalling transmission
          _time_tag = processor()->get_time_tick();
          _timeout_value = TRANSPORT_TIMEOUT_T4;
        }
        else
        {
          uint8_t next_packet = packet.data()[2];
          _range.first = static_cast<uint8_t>(num_packets);
          _range.second = static_cast<uint8_t>(num_packets + next_packet);
          _state = SendData;
        }
      }
      else if ((packet.data()[0] == Abort) || ((packet.data()[0] == EOM) && (_state == WaitEOM)))
      {
        _state = None;
      }
    }
    break;
  
  default:
    break;
  }
}


} // can
} // brt


