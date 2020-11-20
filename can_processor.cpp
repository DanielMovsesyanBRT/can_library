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
  _device_db.initialize();
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

  // Check buses
  for (auto& bus : _bus_map)
  {
    if (bus.second._status == eBusActivating)
    {
      if ((time_tick - bus.second._time_tag) >= CAN_ADDRESS_CLAIMED_WAITING_TIME)
      {
        bus.second._status = eBusActive;
        for (auto iter = bus.second._bus_callbacks.begin(); iter != bus.second._bus_callbacks.end(); )
        {
          if ((*iter) && (*iter)(bus.first, bus.second._status))
            iter = bus.second._bus_callbacks.erase(iter);
          else
            iter++;
        }

        while (!bus.second._msg_fifo.empty())
        {
          send_raw_message(bus.second._msg_fifo.front(), bus.first);
          bus.second._msg_fifo.pop_front();
        }
      }
    }
  }

  // Call all updaters
  for (auto updater : _updaters)
  {
    if (updater)
      updater();
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
  bus._initial_msg_id = msg->packet_id();

  auto result = _bus_map.insert(std::unordered_map<std::string,Bus>::value_type(bus_name,bus));
  if (!result.second)
    return false;
  
  _device_db.create_bus(bus_name);

  // Register callback for this messgae to process BUS activation 
  _confirm_callbacks.push_back(MessageConfirmation(msg,
           [&](CanMessagePtr message,CanMessageConfirmation status) 
      {
        for (auto& bus : _bus_map)
        {
          if ((bus.second._status == eBusWaitForSuccesfullTX) && 
              (bus.second._initial_msg_id == message->packet_id()))
          {
            if (status == eMessageSent)
              bus.second._status = eBusActivating;
            else
              bus.second._status = eBusInactive;
              
            bus.second._time_tag = _cback->get_time_tick();
            for (auto iter = bus.second._bus_callbacks.begin(); iter != bus.second._bus_callbacks.end(); )
            {
              if ((*iter) && (*iter)(bus.first, bus.second._status))
                iter = bus.second._bus_callbacks.erase(iter);
              else
                iter++;
            }
            break;
          }
        }
      }));

  _cback->send_can_frame(bus_name, msg);
  return true;
}

/**
 * \fn  CanProcessor::get_bus_status
 *
 * @param  & bus : const std::string
 * @return  CanBusStatus
 */
CanBusStatus CanProcessor::get_bus_status(const std::string& bus) const
{
  auto iter = _bus_map.find(bus);
  if (iter == _bus_map.end())
    return eBusInactive;

  return iter->second._status;
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
 * @param  status : CanMessageConfirmation 
 */
void CanProcessor::can_frame_confirm(uint64_t message_id,CanMessageConfirmation status)
{
  auto iter = std::find_if(_confirm_callbacks.begin(), _confirm_callbacks.end(),[message_id](const MessageConfirmation& cfrm)->bool
  {
    return cfrm._message->packet_id() == message_id;
  });

  if ((iter != _confirm_callbacks.end()) && iter->_callback)
  {
    iter->_callback(iter->_message, status);
    _confirm_callbacks.erase(iter);
  }
}

/**
 * \fn  CanProcessor::can_frame_confirm
 *
 * @param  message : CanMessagePtr 
 */
void CanProcessor::can_frame_confirm(CanMessagePtr message,CanMessageConfirmation status)
{
  auto iter = std::find_if(_confirm_callbacks.begin(), _confirm_callbacks.end(),[message](const MessageConfirmation& cfrm)->bool
  {
    return cfrm._message == message;
  });

  if ((iter != _confirm_callbacks.end()) && iter->_callback)
  {
    iter->_callback(iter->_message, status);
    _confirm_callbacks.erase(iter);
  }
}

/**
 * \fn  CanProcessor::send_raw_message
 *
 * @param  message : CanMessagePtr 
 * @param  bus_name : const std::string&
 * @param  fn :  ConfirmationCallback 
 * @return  bool
 */
bool CanProcessor::send_raw_message(CanMessagePtr message,const std::string& bus_name,
                      ConfirmationCallback fn/* = ConfirmationCallback()*/)
{
  if (_cback == nullptr)
    return false;

  auto bus = _bus_map.find(bus_name);
  if (bus == _bus_map.end())
    return false;

  if (bus->second._status == eBusInactive)
    return false;

  if (fn)
    _confirm_callbacks.push_back(MessageConfirmation(message, fn));

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
 * @param  fn :  PGNCallback 
 */
void CanProcessor::register_pgn_receiver(uint32_t pgn, PGNCallback fn)
{
  _pgn_receivers.insert(std::unordered_map<uint32_t,PGNCallback>::value_type(pgn,fn));
}

/**
 * \fn  CanProcessor::register_updater
 *
 * @param  fn : UpdateCallback 
 */
void CanProcessor::register_updater(UpdateCallback fn)
{
  _updaters.push_back(fn);
}

/**
 * \fn  CanProcessor::register_bus_callback
 *
 * @param  bus_name : const std::string&
 * @param  fn :  BusStatusCallback 
 */
void CanProcessor::register_bus_callback(const std::string& bus_name, BusStatusCallback fn)
{
  auto bus = _bus_map.find(bus_name);
  if (bus == _bus_map.end())
    return;
  
  bus->second._bus_callbacks.push_back(fn);
}


} // can
} // brt

