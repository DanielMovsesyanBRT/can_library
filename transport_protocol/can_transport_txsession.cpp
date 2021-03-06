/**
 * can_transport_txsession.cpp
 * 
 */

#include "can_transport_txsession.hpp"
#include "../can_processor.hpp"

#include <mutex>

namespace brt {
namespace can {

allocator<TxSession>* TxSession::_allocator = nullptr;

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

      _time_tag = processor()->get_time_tick();
      _state = WaitDriverConfirmation;

      auto me = dynamic_shared_cast<TxSession>(getptr());
      if (!send_bam([me, num_packets, this](uint64_t,const ConstantString& bus_name,bool success)
      /// Lambda begin
            {
              std::lock_guard<Mutex>   l(*(me->_mutex));
              if (me->_state != WaitDriverConfirmation) 
                return;

              if (success)
              {
                _range.first = static_cast<uint8_t>(0);
                _range.second = num_packets;
                _time_tag = me->processor()->get_time_tick();
                _state = SendData; 
                update();
              }   
              else
                _state = None;
            }))
      /// Lambda end
      {
        _state = None;
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

      _time_tag = processor()->get_time_tick();
      _state = WaitDriverConfirmation;

      auto me = dynamic_shared_cast<TxSession>(getptr());
      if (!send_data(_current, [me, this](uint64_t,const ConstantString& bus_name,bool success)
      /// Lambda begin
            {
              std::lock_guard<Mutex>   l(*(me->_mutex));
              if (me->_state != WaitDriverConfirmation) 
                return;

              // Callback for message sent
              if (success)
              {
                _state = SendData;
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
                update();
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
      }
    }
    break;

  case SendRTS:
    {
      _time_tag = processor()->get_time_tick();
      _state = WaitDriverConfirmation;

      auto me = dynamic_shared_cast<TxSession>(getptr());
      if (!send_rts([me, this](uint64_t,const ConstantString& bus_name,bool success)
      /// Lambda begin
            {
              std::lock_guard<Mutex>   l(*(me->_mutex));
              if (me->_state != WaitDriverConfirmation) 
                return;

              if (success)
              {
                _time_tag = me->processor()->get_time_tick();
                _timeout_value = TRANSPORT_TIMEOUT_T3;
                _state = WaitCTS;
                update();
              }
              else
                me->_state = None;
            }))
      /// Lambda end
      {
        _state = None;
      }
    }
    break;

  case WaitEOM:
  case WaitCTS:
    if ((processor()->get_time_tick() - _time_tag) > _timeout_value)
      abort(AbortTimeout);
    break;

  case WaitDriverConfirmation:
    if ((processor()->get_time_tick() - _time_tag) >= 1000) // 1 second
      abort(AbortIgnoreMessage);
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

      _time_tag = processor()->get_time_tick();
      _state = WaitDriverConfirmation;

      auto me = dynamic_shared_cast<TxSession>(getptr());

      if (!send_data(_current, [me, this](uint64_t,const ConstantString& bus_name,bool success)
      /// Lambda begin
            {
              std::lock_guard<Mutex>   l(*(me->_mutex));
              if (me->_state != WaitDriverConfirmation) 
                return;

              // Callback for message sent
              if (success)
              {
                _state = SendData;
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
                update();
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

/**
 * \fn  delete
 *
 * @param  ptr : void* 
 * @return  void TxSession::operator
 */
void TxSession::operator delete  ( void* ptr )  
{  
  if (_allocator == nullptr)
    throw std::runtime_error("Library is not properly initialized");

  if (!_allocator->free(ptr))
    ::free(ptr); 
}

/**
 * \fn  constructor TxSessionPtr::TxSessionPtr
 *
 * @param  processor : CanProcessor* 
 * @param  mutex :  Mutex* 
 * @param   message :  const CanMessagePtr&
 * @param   source :  const CanECUPtr&
 * @param   destination : const CanECUPtr&
 * @param   bus_name : const ConstantString&
 */
TxSessionPtr::TxSessionPtr(CanProcessor* processor, Mutex* mutex, const CanMessagePtr& message,
                                  const CanECUPtr& source,const CanECUPtr& destination,const ConstantString& bus_name)
{
  if (TxSession::_allocator == nullptr)
    throw std::runtime_error("Library is not properly initialized");

  TxSession* session = reinterpret_cast<TxSession*>(TxSession::_allocator->allocate());
  if (session == nullptr)
    session = reinterpret_cast<TxSession*>(::malloc(sizeof(TxSession)));

  ::new (session) TxSession(processor, mutex, message, source, destination, bus_name);
  reset(session);
}


} // can
} // brt


