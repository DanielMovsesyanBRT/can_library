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
        while (!bus.second._msg_fifo.empty())
        {
          send_raw_message(bus.second._msg_fifo.front(), bus.first);
          bus.second._msg_fifo.pop_front();
        }
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
        if (status == eMessageSent)
          bus.second._status = eBusActivating;
        else
          bus.second._status = eBusInactive;
          
        bus.second._time_tag = _cback->get_time_tick();
        break;
      }
    }
  });
}

/**
 * \fn  CanProcessor::get_all_buses
 *
 * @return  std::vector<std::string
 */
std::vector<std::string> CanProcessor::get_all_buses() const
{
  std::vector<std::string> result;
  for (auto bus : _bus_map)
    result.push_back(bus.first);

  return result;
}

/**
 * \fn  CanProcessor::create_local_ecu
 *
 * @param  name : const CanName& 
 * @param  desired_address :  uint8_t 
 * @param  desired_buses :  const std::vector<std::string>&
 * @return  LocalECUPtr
 */
LocalECUPtr CanProcessor::create_local_ecu(const CanName& name,
                                            uint8_t desired_address /*= BROADCATS_CAN_ADDRESS*/,
                                            const std::vector<std::string>& desired_buses /*= std::vector<std::string>()*/)
{
  CanECUPtr ecu = _device_db.get_ecu_by_name(name);
  if (ecu)
    return std::dynamic_pointer_cast<LocalECU>(ecu);

  bool result = false;
  LocalECUPtr local = std::make_shared<LocalECU>(this, name);
  if (desired_buses.empty()) // all buses
  {
    for (auto bus : _bus_map)
    {
      if (!_device_db.add_local_ecu(local, bus.first, desired_address))
      {
        // Unable to register device on the Bus
        // So we are going to disable it
        local->disable_device(bus.first);
      }
      else
        result = true;
    }
  }
  else
  {
    for (auto bus : desired_buses)
    {
      if (_bus_map.find(bus) == _bus_map.end())
        continue;

      if (!_device_db.add_local_ecu(local, bus, desired_address))
      {
        // Unable to register device on the Bus
        // So we are going to disable it
        local->disable_device(bus);
      }
      else
        result = true;
    }
  }

  if (!result)
    return LocalECUPtr();

  return local;
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

/**
 * \fn  CanProcessor::send_raw_message
 *
 * @param  message : CanMessagePtr 
 * @param  bus_name : const std::string&
 * @param  fn :  ConfirmationCallback && 
 * @return  bool
 */
bool CanProcessor::send_raw_message(CanMessagePtr message,const std::string& bus_name,
                      ConfirmationCallback && fn/* = ConfirmationCallback()*/)
{
  if (_cback == nullptr)
    return false;

  auto bus = _bus_map.find(bus_name);
  if (bus == _bus_map.end())
    return false;

  if (bus->second._status == eBusInactive)
    return false;

  using namespace std::placeholders;
  WaitingFrame frm;
  frm._message = message;
  frm._time_tag = _cback->get_time_tick();
  frm._callback = std::bind(std::forward<ConfirmationCallback>(fn), _1, _2);

  _waiting_stack.insert(frm);

  if (bus->second._status != eBusActive)
  {
    // There is a potential danger that remote device or
    // local device will change its address on the bus while
    // bus is in waiting state, however for this moment we consider
    // the message is already on the bus - outside of address negotiation logic
    bus->second._msg_fifo.push_back(message);   
  }
  else
  {
    _cback->send_can_frame(bus_name,message);
  }
  return true;

}

/**
 * \fn  CanProcessor::register_pgn_receiver
 *
 * @param  pgn : uint32_t 
 * @param  fn :  PGNCallback && 
 */
void CanProcessor::register_pgn_receiver(uint32_t pgn, PGNCallback && fn)
{
  using namespace std::placeholders;
  _pgn_receivers.insert(std::unordered_map<uint32_t,PGNCallback>::value_type(pgn,
                  std::bind(std::forward<PGNCallback>(fn), _1, _2)));

}

/**
 * \fn  CanProcessor::register_updater
 *
 * @param  fn : UpdateCallback && 
 */
void CanProcessor::register_updater(UpdateCallback && fn)
{
  _updaters.push_back(std::bind(std::forward<UpdateCallback>(fn)));
}


} // can
} // brt

