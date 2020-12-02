/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 14:56:47
 * File : remote_ecu.cpp
 *
 */
    
#include "remote_ecu.hpp"  
#include "can_constants.hpp" 
#include "can_processor.hpp" 

#include "transcoders/can_transcoder_ack.hpp"
#include "transcoders/can_transcoder_ecu_id.hpp"
#include "transcoders/can_transcoder_software_id.hpp"

namespace brt {
namespace can {

allocator<RemoteECU>* RemoteECU::_allocator = nullptr;
/**
 * \fn  constructor RemoteECU::RemoteECU
 *
 * @param  processor : CanProcessor* 
 * @param  name : const CanName& 
 */
RemoteECU::RemoteECU(CanProcessor* processor,const CanName& name /*= CanName()*/)
: CanECU(processor,name)
, _status_timer(processor->get_time_tick())
, _status_ready(false)
, _mutex(processor)
, _queue()
{

}

/**
 * \fn  destructor RemoteECU::~RemoteECU
 *
 */
RemoteECU::~RemoteECU()
{

}

/**
 * \fn  delete
 *
 * @param  ptr : void* 
 * @return  void RemoteECU::operator
 */
void RemoteECU::operator delete  ( void* ptr )
{ 
  if (_allocator == nullptr)
    throw std::runtime_error("Library is not properly initialized");

  if (!_allocator->free(ptr))
    ::free(ptr);
}

/**
 * \fn  RemoteECU::on_message_received
 *
 * @param  msg : const CanMessagePtr& 
 * @return  bool
 */
bool RemoteECU::on_message_received(const CanMessagePtr& msg)
{
  uint32_t pgn = msg->pgn();
  if (pgn == PGN_AckNack)
    pgn = CanTranscoderAck(msg).pgn();

  std::lock_guard<RecoursiveMutex> l(_mutex);
  switch(pgn)
  {
  case PGN_SoftwareID:
    set_pgn_transcoder(CanTranscoderSoftwareId::Decoder(msg).decode());
    break;

  case PGN_ECUID:
    set_pgn_transcoder(CanTranscoderEcuId::Decoder(msg).decode());
    break;

  default:
    return false;
  }

  return true;
}

/**
 * \fn  RemoteECU::get_requested_pgn
 *
 * @param  pgn : uint32_t 
 * @return  shared_pointer<CanTranscoder
 */
shared_pointer<CanTranscoder> RemoteECU::get_requested_pgn(uint32_t pgn) const
{
  std::lock_guard<RecoursiveMutex> l(_mutex);
  switch(pgn)
  {
  case PGN_SoftwareID:
  case PGN_ECUID:
    return get_pgn_transcoder(pgn);

  default:
    break;
  }

  return shared_pointer<CanTranscoder>();
}

/**
 * \fn  RemoteECU::init_status
 *
 */
void RemoteECU::init_status()
{
  std::lock_guard<RecoursiveMutex> l(_mutex);
  _status_ready = false;
  _status_timer = processor()->get_time_tick();
  
  processor()->register_updater([this]()->bool
  {
    std::lock_guard<RecoursiveMutex> l(_mutex);
    if (!_status_ready)
    {
      if ((processor()->get_time_tick() - _status_timer) > CAN_ADDRESS_CLAIMED_WAITING_TIME)
      {
        _status_ready = true;
        while (!_queue.empty())
        {
          MsgQueue& msg = _queue.front();
          processor()->send_can_message(msg._message, LocalECUPtr(msg._local), RemoteECUPtr(getptr()), { msg._bus_name });
          _queue.pop();
        }

        return true;
      }
    }
    return false;
  });
}

/**
 * \fn  RemoteECU::queue_message
 *
 * @param  remote : CanMessagePtr 
 * @param  local :  LocalECUPtr 
 * @param  & bus_name :  const std::string
 */
bool RemoteECU::queue_message(CanMessagePtr message, LocalECUPtr local, const std::string& bus_name)
{
  std::lock_guard<RecoursiveMutex> l(_mutex);
  if (_status_ready)
    return false;
  
  _queue.push(MsgQueue(message, local, bus_name));
  return true;
}

/**
 * \fn  constructor RemoteECUPtr::RemoteECUPtr
 *
 * @param  <empty> : CanProcessor*
 * @param  name : const CanName& 
 */
RemoteECUPtr::RemoteECUPtr(CanProcessor* processor,const CanName& name /*= CanName()*/)
{
  if (RemoteECU::_allocator == nullptr)
    throw std::runtime_error("Library is not properly initialized");

  RemoteECU* ecu = reinterpret_cast<RemoteECU*>(RemoteECU::_allocator->allocate());
  if (ecu == nullptr)
    ecu = reinterpret_cast<RemoteECU*>(::malloc(sizeof(RemoteECU)));

  ::new (ecu) RemoteECU(processor,name);
  reset(ecu);
}

} // can
} // brt

