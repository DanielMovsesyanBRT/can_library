/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 11:59:45
 * File : can_processor.cpp
 *
 */
    
#include "can_processor.hpp"  
#include "can_transport_protocol.hpp"

#include <mutex>

namespace brt {
namespace can {

/**
 * \fn  constructor CanProcessor::CanProcessor
 *
 * @param  cback : Callback* 
 */
CanProcessor::CanProcessor(Callback* cback)
: _cback(cback)
, _mutex(this)
, _device_db(this)
{
  _transport_stack.push_back(std::make_shared<SimpleTransport>(this));
  _transport_stack.push_back(std::make_shared<CanTransportProtocol>(this));
}

/**
 * \fn  destructor CanProcessor::~CanProcessor
 *
 */
CanProcessor::~CanProcessor()
{
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

  // Call all updaters
  for (auto updater : _updaters)
  {
    if (updater)
      updater();
  }

  // Check buses
  std::lock_guard<RecoursiveMutex> l(_mutex);
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
      }
    }

    if (bus.second._status == eBusActive)
    {
      while (!bus.second._packet_fifo.empty())
      {
        _cback->send_can_packet(bus.first, bus.second._packet_fifo.front());
        bus.second._packet_fifo.pop_front();
      }
    }
  }
}

/**
 * \fn  CanProcessor::register_can_bus
 *
 * @param  bus : const std::string&
 * @return  bool
 */
bool CanProcessor::register_can_bus(const std::string& bus_name)
{
  if (_cback == nullptr)
    return false;

  {
    std::lock_guard<RecoursiveMutex> l(_mutex);
    if (_bus_map.find(bus_name) != _bus_map.end())
      return false;
  }

  // Request for address Claimed
  CanPacket packet({00,0xEE,00}, PGN_Request, BROADCATS_CAN_ADDRESS, NULL_CAN_ADDRESS);

  Bus bus;
  bus._bus_name       = bus_name;
  bus._status         = eBusWaitForSuccesfullTX;
  bus._time_tag       = _cback->get_time_tick();
  bus._initial_packet_id = packet.unique_id();

  {
    std::lock_guard<RecoursiveMutex> l(_mutex);
    auto result = _bus_map.insert(std::unordered_map<std::string,Bus>::value_type(bus_name,bus));
    if (!result.second)
      return false;
 
    // Register callback for this message to process BUS activation 
    _confirm_callbacks.push_back(PacketConfirmation(packet.unique_id(),
           [this](uint64_t packet_id,CanMessageConfirmation status) 
      {
        std::lock_guard<RecoursiveMutex> l(_mutex);
        for (auto& bus : _bus_map)
        {
          if ((bus.second._status == eBusWaitForSuccesfullTX) && 
              (bus.second._initial_packet_id == packet_id))
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

    if (_bus_map.size() == 1)
    {
      register_pgn_receiver(PGN_Request,[this](const CanPacket& packet,const std::string& bus_name)
      {  on_request(packet,bus_name); });
    }
  }

  _device_db.create_bus(bus_name);
  _cback->send_can_packet(bus_name, packet);
  return true;
}

/**
 * \fn  CanProcessor::on_request
 *
 * @param  packet : const CanPacket& 
 * @param  & bus_name : const std::string
 */
void CanProcessor::on_request(const CanPacket& packet,const std::string& bus_name)
{
  if (packet.is_broadcast())
  {
    std::vector<LocalECUPtr> ecus = _device_db.get_local_ecus(bus_name);
    for (auto ecu : ecus)
    {
      if (ecu)
        ecu->claim_address(ecu->get_address(bus_name), bus_name);
    }
  }
  else
  {
    LocalECUPtr ecu = std::dynamic_pointer_cast<LocalECU>(_device_db.get_ecu_by_address(packet.da(), bus_name));
    if (ecu)
      ecu->claim_address(ecu->get_address(bus_name), bus_name);
  }
}

/**
 * \fn  CanProcessor::get_bus_status
 *
 * @param  & bus : const std::string
 * @return  CanBusStatus
 */
CanBusStatus CanProcessor::get_bus_status(const std::string& bus) const
{
  std::lock_guard<RecoursiveMutex> l(_mutex);
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
  std::lock_guard<RecoursiveMutex> l(_mutex);
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
    std::vector<std::string> buses;
    {
      std::lock_guard<RecoursiveMutex> l(_mutex);
      for (auto bus : _bus_map)
        buses.push_back(bus.first);
    }

    for (auto bus_name : buses)
    {
      if (!_device_db.add_local_ecu(local, bus_name, desired_address))
      {
        // Unable to register device on the Bus
        // So we are going to disable it
        local->disable_device(bus_name);
      }
      else
        result = true;
    }
  }
  else
  {
    for (auto bus : desired_buses)
    {
      {
        std::lock_guard<RecoursiveMutex> l(_mutex);
        if (_bus_map.find(bus) == _bus_map.end())
          continue;
      }

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
 * \fn  CanProcessor::destroy_local_ecu
 *
 * @param  local : LocalECUPtr 
 * @return  bool
 */
bool CanProcessor::destroy_local_ecu(LocalECUPtr local)
{
  return device_db().remove_local_ecu(local->name());
}

/**
 * \fn  CanProcessor::destroy_local_ecu
 *
 * @param  name : const CanName& 
 * @return  bool
 */
bool CanProcessor::destroy_local_ecu(const CanName& name)
{
  return device_db().remove_local_ecu(name);
}

/**
 * \fn  CanProcessor::received_can_packet
 *
 * @param  packet : const CanPacket& 
 * @param  bus_name : const std::string&
 * @return  bool
 */
bool CanProcessor::received_can_packet(const CanPacket& packet,const std::string& bus_name)
{
  if (_cback == nullptr)
    return false;

  LocalECUPtr   local;
  if (!packet.is_broadcast())
  {
    // First we need to check whether this packet is sent to any of our local devices
    local = std::dynamic_pointer_cast<LocalECU>(_device_db.get_ecu_by_address(packet.da(), bus_name));
    if (!local)
      return false; // Not our message
  }

  {
    // Now check whether the PGN belongs to one of the listeners e.g Request Address Claimed, TP, ETP
    std::lock_guard<RecoursiveMutex> l(_mutex);
    auto pgn_receiver = _pgn_receivers.find(packet.pgn());
    if (pgn_receiver != _pgn_receivers.end())
    {
      pgn_receiver->second(packet, bus_name);
      return true;
    }
  }

  RemoteECUPtr  remote;
  if (packet.sa() < NULL_CAN_ADDRESS)
    remote = std::dynamic_pointer_cast<RemoteECU>(_device_db.get_ecu_by_address(packet.sa(), bus_name));

  _cback->message_received(CanMessagePtr(packet.data(), packet.dlc(), packet.pgn(), packet.priority()), local, remote);
  return true;
}

/**
 * \fn  CanProcessor::send_can_message
 *
 * @param  message : CanMessagePtr 
 * @param  local : LocalECUPtr 
 * @param  remote : RemoteECUPtr 
 * @return  bool
 */
bool CanProcessor::send_can_message(CanMessagePtr message,LocalECUPtr local,RemoteECUPtr remote)
{
  if (!local)
    return false;

  if (!remote)
    return send_can_message(message,local,get_all_buses());

  for (auto transport : _transport_stack)
  {
    if (transport->send_message(message, local, remote))
      return true;
  }
  return false;
}

/**
 * \fn  CanProcessor::send_can_message
 *
 * @param  message : CanMessagePtr 
 * @param  local : LocalECUPtr 
 * @param  buses : const std::vector<std::string>& 
 * @return  bool
 */
bool CanProcessor::send_can_message(CanMessagePtr message,LocalECUPtr local,const std::vector<std::string>& buses)
{
  if (!local)
    return false;

  for (auto transport : _transport_stack)
  {
    if (transport->send_message(message, local, buses))
      return true;
  }
  return false;
}

/**
 * \fn  CanProcessor::SimpleTransport::send_message
 *
 * @param  message : CanMessagePtr 
 * @param  local :  LocalECUPtr 
 * @param  remote :  RemoteECUPtr 
 * @return  bool
 */
bool CanProcessor::SimpleTransport::send_message(CanMessagePtr message, LocalECUPtr local, RemoteECUPtr remote)
{
  if (message->length() > 8)
    return false;

  local->send_message(message, remote);
  return true;
}

/**
 * \fn  CanProcessor::SimpleTransport::send_message
 *
 * @param  message : CanMessagePtr 
 * @param  local :  LocalECUPtr 
 * @param  >& buses :  const std::vector<std::string
 * @return  bool
 */
bool CanProcessor::SimpleTransport::send_message(CanMessagePtr message, LocalECUPtr local, const std::vector<std::string>& buses)
{
  if (message->length() > 8)
    return false;

  for (auto bus_name : buses)
    local->send_message(message, bus_name);

  return true;
}


/**
 * \fn  CanProcessor::can_packet_confirm
 *
 * @param  packet_id : uint64_t 
 * @param  status : CanMessageConfirmation 
 */
void CanProcessor::can_packet_confirm(uint64_t packet_id,CanMessageConfirmation status)
{
  std::lock_guard<RecoursiveMutex> l(_mutex);
  auto iter = std::find_if(_confirm_callbacks.begin(), _confirm_callbacks.end(),[packet_id](const PacketConfirmation& cfrm)->bool
  {
    return cfrm._packet_id == packet_id;
  });

  if ((iter != _confirm_callbacks.end()) && iter->_callback)
  {
    iter->_callback(packet_id, status);
    _confirm_callbacks.erase(iter);
  }
}

/**
 * \fn  CanProcessor::can_packet_confirm
 *
 * @param  packet : const CanPacket& 
 * @param  status : CanMessageConfirmation 
 */
void CanProcessor::can_packet_confirm(const CanPacket& packet,CanMessageConfirmation status)
{
  can_packet_confirm(packet.unique_id(), status);
}

/**
 * \fn  CanProcessor::send_raw_packet
 *
 * @param  packet : const CanPacket& 
 * @param  bus_name : const std::string&
 * @param  fn :  ConfirmationCallback 
 * @return  bool
 */
bool CanProcessor::send_raw_packet(const CanPacket& packet,const std::string& bus_name,
                      ConfirmationCallback fn/* = ConfirmationCallback()*/)
{
  if (_cback == nullptr)
    return false;

  std::lock_guard<RecoursiveMutex> l(_mutex);
  auto bus = _bus_map.find(bus_name);
  if (bus == _bus_map.end())
    return false;

  if (bus->second._status == eBusInactive)
    return false;

  if (fn)
    _confirm_callbacks.push_back(PacketConfirmation(packet.unique_id(), fn));

  bus->second._packet_fifo.push_back(packet);

  // if (bus->second._status != eBusActive)
  // {
  //   // There is a potential danger that remote device or
  //   // local device will change its address on the bus while
  //   // bus is in waiting state, however for this moment we consider
  //   // the message is already on the bus - outside of address negotiation logic
  // }
  // else
  // {
  //   _cback->send_can_packet(bus_name,packet);
  // }

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
  std::lock_guard<RecoursiveMutex> l(_mutex);
  _pgn_receivers.insert(std::unordered_map<uint32_t,PGNCallback>::value_type(pgn,fn));
}

/**
 * \fn  CanProcessor::register_updater
 *
 * @param  fn : UpdateCallback 
 */
void CanProcessor::register_updater(UpdateCallback fn)
{
  std::lock_guard<RecoursiveMutex> l(_mutex);
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
  std::lock_guard<RecoursiveMutex> l(_mutex);
  auto bus = _bus_map.find(bus_name);
  if (bus == _bus_map.end())
    return;
  
  bus->second._bus_callbacks.push_back(fn);
}


} // can
} // brt

