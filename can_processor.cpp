/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 11:59:45
 * File : can_processor.cpp
 *
 */
    
#include "can_processor.hpp"  
#include "can_transport_protocol.hpp"
#include "can_transcoder_ack.hpp"

#include <mutex>

namespace brt {
namespace can {

/**
 * \fn  constructor CanProcessor::CanProcessor
 *
 * @param  cback : Callback* 
 */
CanProcessor::CanProcessor(Callback* cback)
: CanInterface(cback)
, _mutex(this)
, _device_db(this)
, _remote_name_counter(0)
{
  _transport_stack.push(CanProtocolPtr(new SimpleTransport(this)));
  _transport_stack.push(CanProtocolPtr(new CanTransportProtocol(this)));
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
  uint64_t time_tick = get_time_tick();

  {
    std::lock_guard<RecursiveMutex> l(_mutex);
    // Call all updaters
    for (auto updater : _updaters)
    {
      if (updater)
        updater();
    }

    // Check buses
    for (auto& bus : _bus_map)
    {
      if (bus._status == eBusActivating)
      {
        if ((time_tick - bus._time_tag) >= CAN_ADDRESS_CLAIMED_WAITING_TIME)
        {
          bus._status = eBusActive;
          for (auto iter = bus._bus_callbacks.begin(); iter != bus._bus_callbacks.end(); )
          {
            if ((*iter) && (*iter)(bus._bus_name, bus._status))
              iter = bus._bus_callbacks.erase(iter);
            else
              iter++;
          }
        }
      }

      if (bus._status == eBusActive)
      {
        while (!bus._packet_fifo.empty())
        {
          cback()->send_can_packet(bus._bus_name, bus._packet_fifo.front());
          bus._packet_fifo.pop();
        }
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
bool CanProcessor::register_can_bus(const ConstantString& bus_name)
{
  {
    std::lock_guard<RecursiveMutex> l(_mutex);
    if (_bus_map.find_if([bus_name](const Bus& bus)->bool
             { return bus._bus_name == bus_name;}) != _bus_map.end())
    {
      return false;
    }
  }

  // Request for address Claimed
  CanPacket packet({00,0xEE,00}, PGN_Request, BROADCAST_CAN_ADDRESS, NULL_CAN_ADDRESS);

  Bus bus;
  bus._bus_name       = bus_name;
  bus._status         = eBusWaitForSuccesfullTX;
  bus._time_tag       = get_time_tick();
  bus._initial_packet_id = packet.unique_id();

  {
    std::lock_guard<RecursiveMutex> l(_mutex);
    auto result = _bus_map.push(bus);
    if (result == _bus_map.end())
      return false;
 
    // Register callback for this message to process BUS activation 
    _confirm_callbacks.push(PacketConfirmation(packet.unique_id(),
           [this](uint64_t packet_id,CanMessageConfirmation status) 
      {
        std::lock_guard<RecursiveMutex> l(_mutex);
        for (auto& bus : _bus_map)
        {
          if ((bus._status == eBusWaitForSuccesfullTX) &&
              (bus._initial_packet_id == packet_id))
          {
            if (status == eMessageSent)
              bus._status = eBusActivating;
            else
              bus._status = eBusInactive;
              
            bus._time_tag = get_time_tick();
            for (auto iter = bus._bus_callbacks.begin(); iter != bus._bus_callbacks.end(); )
            {
              if ((*iter) && (*iter)(bus._bus_name, bus._status))
                iter = bus._bus_callbacks.erase(iter);
              else
                iter++;
            }
            break;
          }
        }
      }));

    if (_bus_map.size() == 1)
    {
      register_pgn_receiver(PGN_Request,[this](const CanPacket& packet,const ConstantString& bus_name)
      {  on_request(packet,bus_name); });
    }
  }

  _device_db.create_bus(bus_name);
  cback()->send_can_packet(bus_name, packet);
  return true;
}

/**
 * \fn  CanProcessor::on_request
 *
 * @param  packet : const CanPacket& 
 * @param  bus_name : const ConstantString&
 */
void CanProcessor::on_request(const CanPacket& packet,const ConstantString& bus_name)
{
  if (packet.dlc() < 3)
    return;
  
  uint32_t pgn = can_unpack24(packet.data());
  // Requested Address Claimed
  if (pgn == PGN_AddressClaimed)
  {
    if (packet.is_broadcast())
    {
      fixed_list<LocalECUPtr> ecus;
      _device_db.get_local_ecus(ecus, { bus_name });
      for (auto ecu : ecus)
      {
        if (ecu)
          ecu->claim_address(ecu->get_address(bus_name), bus_name);
      }
    }
    else
    {
      LocalECUPtr ecu(_device_db.get_ecu_by_address(packet.da(), bus_name));
      if (ecu)
        ecu->claim_address(ecu->get_address(bus_name), bus_name);
    }
  }
  else
  {
    RemoteECUPtr remote(_device_db.get_ecu_by_address(packet.sa(), bus_name));
    if (!remote)
      remote = register_abstract_remote_ecu(packet.sa(), bus_name);
    
    if (!remote)
      return; // 

    if (packet.is_broadcast())
    {
      fixed_list<LocalECUPtr> ecus;
      _device_db.get_local_ecus(ecus, { bus_name });
      for (auto local : ecus)
      {
        CanMessagePtr msg = local->request_pgn(pgn);
        if (msg)
          send_can_message(msg, local, remote, { bus_name });
      }
    }
    else
    {
      LocalECUPtr local(_device_db.get_ecu_by_address(packet.da(), bus_name));
      if (!local)
        return;
      
      CanMessagePtr msg = local->request_pgn(pgn);
      if (!msg)
        msg = CanTranscoderAck(CanTranscoderAck::Nack, packet.sa(), pgn).create_message();

      if (msg)
        send_can_message(msg, local, remote, { bus_name });
    }
  }
}

/**
 * \fn  CanProcessor::get_bus_status
 *
 * @param   bus_name : const ConstantString&
 * @return  CanBusStatus
 */
CanBusStatus CanProcessor::get_bus_status(const ConstantString& bus_name) const
{
  std::lock_guard<RecursiveMutex> l(_mutex);
  auto iter = _bus_map.find_if([bus_name](const Bus& bus)->bool
      { return bus._bus_name == bus_name;});

  if (iter == _bus_map.end())
    return eBusInactive;

  return iter->_status;
}

/**
 * \fn  CanProcessor::create_local_ecu
 *
 * @param  name : const CanName& 
 * @return  LocalECUPtr
 */
LocalECUPtr CanProcessor::create_local_ecu(const CanName& name)
{
  CanECUPtr ecu = _device_db.get_ecu_by_name(name,"");
  if (ecu)
    return LocalECUPtr(ecu);

  return LocalECUPtr(this, name);
}

/**
 * \fn  CanProcessor::activate_local_ecu
 *
 * @param   ecu : const LocalECUPtr&
 * @param   bus_name :  const ConstantString&
 * @param  desired_address  :  uint8_t
 * @return  bool
 */
bool CanProcessor::activate_local_ecu(const LocalECUPtr& ecu, const ConstantString& bus_name,
                        uint8_t desired_address /*= BROADCAST_CAN_ADDRESS*/)
{
  {
    std::lock_guard<RecursiveMutex> l(_mutex);
    if (_bus_map.find_if([bus_name](const Bus& bus)->bool
            { return bus._bus_name == bus_name; }) == _bus_map.end())
    {
      return false;
    }
  }

  return _device_db.add_local_ecu(ecu, bus_name, desired_address);
}

/**
 * \fn  CanProcessor::register_abstract_remote_ecu
 *
 * @param  address : uint8_t 
 * @param   bus : const ConstantString&
 * @return  RemoteECUPtr
 */
RemoteECUPtr CanProcessor::register_abstract_remote_ecu(uint8_t address,const ConstantString& bus)
{
  CanName name(_remote_name_counter++);
  RemoteECUPtr remote(this, name);
  if (!_device_db.add_remote_abstract_ecu(remote, bus, address))
    return RemoteECUPtr();

  return remote;
}

/**
 * \fn  CanProcessor::destroy_local_ecu
 *
 * @param  local : LocalECUPtr 
 * @return  bool
 */
bool CanProcessor::destroy_local_ecu(const LocalECUPtr& local)
{
  return destroy_local_ecu(local->name());
}

/**
 * \fn  CanProcessor::destroy_local_ecu
 *
 * @param  name : const CanName& 
 * @return  bool
 */
bool CanProcessor::destroy_local_ecu(const CanName& name)
{
  size_t num = 0;
  
  std::lock_guard<RecursiveMutex> l(_mutex);
  for (auto bus : _bus_map)
  {
    if (device_db().remove_local_ecu(name,bus._bus_name))
      num++;
  }
  return (num != 0);
}

/**
 * \fn  CanProcessor::received_can_packet
 *
 * @param   packet : const CanPacket&
 * @param   bus_name : const ConstantString&
 * @return  bool
 */
bool CanProcessor::received_can_packet(const CanPacket& packet,const ConstantString& bus_name)
{
  LocalECUPtr   local;
  if (!packet.is_broadcast())
  {
    // First we need to check whether this packet is sent to any of our local devices
    local = LocalECUPtr(_device_db.get_ecu_by_address(packet.da(), bus_name));
    if (!local)
      return false; // Not our message
  }

  {
    // Now check whether the PGN belongs to one of the listeners e.g Request Address Claimed, TP, ETP
    std::lock_guard<RecursiveMutex> l(_mutex);
    auto pgn_receiver = _pgn_receivers.find_if([packet](const PgnReceiver& receiver)->bool
        { return packet.pgn() == receiver.first; });

    if (pgn_receiver != _pgn_receivers.end())
    {
      pgn_receiver->second(packet, bus_name);
      return true;
    }
  }

  RemoteECUPtr  remote;
  if (packet.sa() < NULL_CAN_ADDRESS)
    remote = RemoteECUPtr(_device_db.get_ecu_by_address(packet.sa(), bus_name));

  message_received(CanMessagePtr(packet.data(), packet.dlc(), packet.pgn(), packet.priority()), local, remote, bus_name);
  return true;
}

/**
 * \fn  CanProcessor::send_can_message
 *
 * @param   message : const CanMessagePtr&
 * @param   local : const LocalECUPtr&
 * @param   remote : const RemoteECUPtr&
 * @param  buses  :  const std::initializer_list<ConstantString>&
 * @return  bool
 */
bool CanProcessor::send_can_message(const CanMessagePtr& message,const LocalECUPtr& local,const RemoteECUPtr& remote,
                            const std::initializer_list<ConstantString>& buses /*= std::initializer_list<ConstantString> buses()*/)
{
  if (!local)
    return false;

  fixed_list<ConstantString,32> bss(buses);

  if (bss.empty())
    get_all_buses(bss);

  bool result = false;
  for (auto bus_name : bss)
  {
    if (remote && remote->queue_message(message, local, bus_name))
      continue;

    for (auto transport : _transport_stack)
    {
      if (transport->send_message(message, local, remote, bus_name))
      {
        result = true;
        break;
      }
    }
  }

  return result;
}

/**
 * \fn  CanProcessor::message_received
 *
 * @param   message : const CanMessagePtr&
 * @param   local : const LocalECUPtr&
 * @param   remote :  const RemoteECUPtr&
 * @param   bus_name : const ConstantString&
 */
void CanProcessor::message_received(const CanMessagePtr& message,const LocalECUPtr& local,
                                   const RemoteECUPtr& remote,const ConstantString& bus_name)
{
  if (remote && remote->on_message_received(message))
  {
    auto req = remote->get_requested_pgn(message->pgn());
    if (req)
    {
      std::lock_guard<RecursiveMutex> l(_mutex);
      for (auto iter = _requested_pgns.begin(); iter != _requested_pgns.end(); )
      {
        if ((iter->_pgn == message->pgn()) && (iter->_ecu == remote))
        {
          iter->_callback(req);
          iter = _requested_pgns.erase(iter);
        }
        else
          ++iter;
      }
    }     
  }
  else
    cback()->message_received(message, local, remote, bus_name);
}

/**
 * \fn  CanProcessor::SimpleTransport::send_message
 *
 * @param   message : const CanMessagePtr&
 * @param   local : const LocalECUPtr&
 * @param   remote :  const RemoteECUPtr&
 * @param   bus_name : const ConstantString&
 * @return  bool
 */
bool CanProcessor::SimpleTransport::send_message(const CanMessagePtr& message,const LocalECUPtr& local, 
                                                      const RemoteECUPtr& remote,const ConstantString& bus_name)
{
  if (message->length() > 8)
    return false;

  local->send_message(message, remote, bus_name);
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
  std::lock_guard<RecursiveMutex> l(_mutex);
  auto iter = _confirm_callbacks.find_if([packet_id](const PacketConfirmation& cfrm)->bool
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
 * \fn  CanProcessor::request_pgn
 *
 * @param  pgn : uint32_t 
 * @param  local : const LocalECUPtr& 
 * @param  remote : const RemoteECUPtr& 
 * @param  callback : const RequestCallback& 
 * @return  bool
 */
bool CanProcessor::request_pgn(uint32_t pgn,const LocalECUPtr& local,const RemoteECUPtr& remote,const RequestCallback& callback)
{
  if (!remote)
    return false;

  auto req_pgn = remote->get_requested_pgn(pgn);
  if (req_pgn)
    callback(req_pgn);
  else
  {
    std::lock_guard<RecursiveMutex> l(_mutex);
    _requested_pgns.push(RequestedPGNs(pgn,remote,callback));
    send_can_message(CanMessagePtr(can_pack24(pgn),PGN_Request), local, remote);
  }

  return true;
}

/**
 * \fn  CanProcessor::send_raw_packet
 *
 * @param   packet : const CanPacket&
 * @param   bus_name : const ConstantString&
 * @param   fn :  const ConfirmationCallback&
 * @return  bool
 */
bool CanProcessor::send_raw_packet(const CanPacket& packet,const ConstantString& bus_name,
                      const ConfirmationCallback& fn/* = ConfirmationCallback()*/)
{
  std::lock_guard<RecursiveMutex> l(_mutex);
  auto bus = _bus_map.find_if([bus_name](const Bus& bus)->bool
      { return bus._bus_name == bus_name; });

  if (bus == _bus_map.end())
    return false;

  if (bus->_status == eBusInactive)
    return false;

  if (fn)
    _confirm_callbacks.push(PacketConfirmation(packet.unique_id(), fn));

  if (bus->_status != eBusActive)
  {
    // There is a potential danger that remote device or
    // local device will change its address on the bus while
    // bus is in waiting state, however for this moment we consider
    // the message is already on the bus - outside of address negotiation logic
    bus->_packet_fifo.push(packet);
  }
  else
    cback()->send_can_packet(bus_name,packet);

  return true;
}

/**
 * \fn  CanProcessor::register_pgn_receiver
 *
 * @param  pgn : uint32_t 
 * @param  fn :  PGNCallback 
 */
void CanProcessor::register_pgn_receiver(uint32_t pgn, const PGNCallback& fn)
{
  std::lock_guard<RecursiveMutex> l(_mutex);
  _pgn_receivers.push(PgnReceiver(pgn,fn));
}

/**
 * \fn  CanProcessor::register_updater
 *
 * @param  fn : UpdateCallback 
 */
void CanProcessor::register_updater(const UpdateCallback& fn)
{
  std::lock_guard<RecursiveMutex> l(_mutex);
  _updaters.push(fn);
}

/**
 * \fn  CanProcessor::register_bus_callback
 *
 * @param  bus_name : const std::string&
 * @param  fn :  BusStatusCallback 
 */
void CanProcessor::register_bus_callback(const ConstantString& bus_name, const BusStatusCallback& fn)
{
  std::lock_guard<RecursiveMutex> l(_mutex);
  auto bus = _bus_map.find_if([bus_name](const Bus& bus)->bool
      { return bus._bus_name == bus_name; });

  if (bus == _bus_map.end())
    return;
  
  bus->_bus_callbacks.push(fn);
}

/**
 * \fn  CanProcessor::get_time_tick
 *
 * @return  uint64_t
 */
uint64_t CanProcessor::get_time_tick() const
{
  return cback()->get_time_tick_nanoseconds() / 1000000llu;
}

} // can
} // brt

