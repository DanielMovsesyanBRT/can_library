/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 11:59:45
 * File : can_processor.cpp
 *
 */
    
#include "can_processor.hpp"  

namespace brt {
namespace can {


CanProcessor CanProcessor::_object;

/**
 * \fn  constructor CanProcessor::CanProcessor
 *
 */
CanProcessor::CanProcessor()
: _cback(nullptr)
, _device_db(this)
{

}

/**
 * \fn  destructor CanProcessor::~CanProcessor
 *
 */
CanProcessor::~CanProcessor()
{

}


/**
 * \fn  CanProcessor::initialize
 *
 * @param  cback : Callback* 
 * @return  bool
 */
bool CanProcessor::initialize(Callback* cback)
{
  if (cback == nullptr)
    return false;

  _cback = cback;
  return true;
}

/**
 * \fn  CanProcessor::update
 *
 */
void CanProcessor::update()
{
  if (_cback == nullptr)
    return;

  uint64_t time_tick = _cback->get_time_tick();

  // Check Confirmation packets
  for (auto iter = _waiting_stack.begin(); iter != _waiting_stack.end(); )
  {
    if ((time_tick - (*iter)._time_tag) > CAN_MESSAGE_MAX_CONFIRMATION_TIME)
    {
      if ((*iter)._callback)
        (*iter)._callback((*iter)._message, eMessageFailed);

      iter = _waiting_stack.erase(iter);      
    }
    else
      iter++;
  }

  // Check buses
  for (auto& bus : _bus_map)
  {
    if (bus.second._status == eBusActivating)
    {
      if ((time_tick - bus.second._time_tag) >= CAN_ADDRESS_CLAIMED_WAITING_TIME)
      {
        bus.second._status = eBusActive;
        // todo: send messages from waiting queue
      }
    }
  }
}

/**
 * \fn  CanProcessor::init_bus
 *
 * @param  bus : const std::string&
 * @return  bool
 */
bool CanProcessor::init_bus(const std::string& bus_name)
{
  if (_cback == nullptr)
    return false;

  if (_bus_map.find(bus_name) != _bus_map.end())
    return false;

  // Request for address Claimed
  CanMessagePtr msg({00,0xEE,00} , PGN_Request, BROADCATS_CAN_ADDRESS, NULL_CAN_ADDRESS);

  Bus bus;
  bus._bus_name       = bus_name;
  bus._status         = eBusWaitForSuccesfullTX;
  bus._time_tag       = _cback->get_time_tick();
  bus._initial_msg_id = msg->id();

  auto result = _bus_map.insert(std::unordered_map<std::string,Bus>::value_type(bus_name,bus));
  if (!result.second)
    return false;
  
  _device_db.create_bus(bus_name);
  return send_raw_message(msg, bus_name, [&](CanMessagePtr message,CanMessageConfirmation status)
  {
    for (auto& bus : _bus_map)
    {
      if ((bus.second._status == eBusWaitForSuccesfullTX) && 
          (bus.second._initial_msg_id == message->id()))
      {
        bus.second._status = eBusActivating;
        bus.second._time_tag = _cback->get_time_tick();
        break;
      }
    }
  });

  /*
  confirmation_wait(msg, [](void* processor, CanMessagePtr message,CanMessageConfirmation status)
  {
    CanProcessor* proc = reinterpret_cast<CanProcessor*>(processor);
    for (auto& bus : proc->_bus_map)
    {
      if ((bus.second._status == eBusWaitForSuccesfullTX) && 
          (bus.second._initial_msg_id == message->id()))
      {
        bus.second._status = eBusActivating;
        bus.second._time_tag = proc->_cback->get_time_tick();
        break;
      }
    }
  }, this);

  _cback->send_can_frame(bus_name, msg);
  return true;   
  */
}

// /**
//  * \fn  CanProcessor::confirmation_wait
//  *
//  * @param  message : CanMessagePtr 
//  * @param  callback : ConfirmationCallback && 
//  * @param  param : void* 
//  */
// void CanProcessor::confirmation_wait(CanMessagePtr message,ConfirmationCallback && callback,void* param)
// {
//   WaitingFrame frm;
//   frm._message = message;
//   frm._time_tag = _cback->get_time_tick();
//   frm._callback = std::move(callback);
//   frm._param = param;

//   _waiting_stack.insert(frm);
// }

// /**
//  * \fn  CanProcessor::send_raw_message
//  *
//  * @param  message : CanMessagePtr 
//  * @return  bool
//  */
// bool CanProcessor::send_raw_message(CanMessagePtr message,ConfirmationCallback && callback)
// {
//   if (_cback == nullptr)
//     return false;
  
//   WaitingFrame frm;
//   frm._message = message;
//   frm._time_tag = _cback->get_time_tick();
//   frm._callback = std::move(callback);
//   frm._param = param;
//   _waiting_stack.insert(frm);
// }

/**
 * \fn  CanProcessor::send_message
 *
 * @param  message : CanMessagePtr 
 * @param  local : LocalECUPtr 
 * @param  remote : RemoteECUPtr 
 * @return  bool
 */
bool CanProcessor::send_message(CanMessagePtr message,LocalECUPtr local,RemoteECUPtr remote /*= CanECUPtr()*/)
{
  if (!local)
    return false;

  std::unordered_map<std::string,uint8_t> sa_map = local->get_addresses();

  if (!remote)
  {
    // broadcast
    for (auto bus_iter : _bus_map)
    {
      auto sa = sa_map.find(bus_iter.first);
      if (sa == sa_map.end())
        continue;

      if (bus_iter.second._status == eBusInactive)
        continue;

      if (bus_iter.second._status != eBusActive)
      {
        bus_iter.second._msg_fifo.push_back(MsgFifoItem(message, local, remote));   
        continue;
      }

      if (message->length() <= 8)
      {
        CanMessagePtr msg(message->data(),message->length(),message->pgn(),BROADCATS_CAN_ADDRESS,sa->second,message->priority());
        _cback->send_can_frame(bus_iter.first, msg);
      }
    }
  }
  else
  {
    std::vector<std::string> buses = _device_db.get_ecu_bus(remote->name());
    if (buses.empty())
      return false;

    auto bus_iter = _bus_map.find(buses[0]);
    if (bus_iter == _bus_map.end())
      return false;

    auto sa = sa_map.find(buses[0]);
    if (sa == sa_map.end())
      return false;

    if (bus_iter->second._status == eBusInactive)
      return false;

    if (bus_iter->second._status != eBusActive)
    {
      bus_iter->second._msg_fifo.push_back(MsgFifoItem(message, local, remote));   
      return true;
    }

    if (message->length() <= 8)
    {
      CanMessagePtr msg(message->data(),message->pgn(),remote->get_address(),sa->second,message->priority());
      _cback->send_can_frame(bus_iter->first, msg);
    }
  }

  return true;
}

/**
 * \fn  CanProcessor::received_can_frame
 *
 * @param  message : CanMessagePtr 
 * @param  bus : const std::string&
 * @return  bool
 */
bool CanProcessor::received_can_frame(CanMessagePtr message,const std::string& bus)
{
  if (_cback == nullptr)
    return false;

  auto pgn_receiver = _pgn_receivers.find(message->pgn());
  if (pgn_receiver != _pgn_receivers.end())
  {
    pgn_receiver->second(message, bus);
    return true;
  }

  LocalECUPtr   local;
  RemoteECUPtr  remote;
  // Is this our message
  if (!message->is_broadcast())
  {
    local = std::dynamic_pointer_cast<LocalECU>(_device_db.get_ecu_by_address(message->da(), bus));
    if (!local)
      return false; // Not our message
  }

  if (message->sa() < NULL_CAN_ADDRESS)
    remote = std::dynamic_pointer_cast<RemoteECU>(_device_db.get_ecu_by_address(message->sa(), bus));

  _cback->message_received(message, local, remote);
  return true;
}

/**
 * \fn  CanProcessor::can_frame_confirm
 *
 * @param  message_id : uint64_t 
 */
void CanProcessor::can_frame_confirm(uint64_t message_id)
{
  auto iter = std::find_if(_waiting_stack.begin(), _waiting_stack.end(),[message_id](const WaitingFrame& frm)->bool
  {
    return frm._message->id() == message_id;
  });

  if ((iter != _waiting_stack.end()) && iter->_callback)
    iter->_callback(iter->_message, eMessageSent);
}

/**
 * \fn  CanProcessor::can_frame_confirm
 *
 * @param  message : CanMessagePtr 
 */
void CanProcessor::can_frame_confirm(CanMessagePtr message)
{
  auto iter = std::find_if(_waiting_stack.begin(), _waiting_stack.end(),[message](const WaitingFrame& frm)->bool
  {
    return frm._message == message;
  });

  if ((iter != _waiting_stack.end()) && iter->_callback)
    iter->_callback(iter->_message, eMessageSent);
}


} // can
} // brt

