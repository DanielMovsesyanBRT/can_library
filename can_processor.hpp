/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 11:59:45
 * File : can_processor.hpp
 *
 */

#pragma once

#include <stdint.h>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <deque>
#include <list>

#include "can_utils.hpp"
#include "can_message.hpp"
#include "local_ecu.hpp"
#include "remote_ecu.hpp"
#include "can_device_database.hpp"
#include "can_protocol.hpp"

namespace brt {
namespace can {

/**
 * \enum CanBusStatus
 *
 */
enum CanBusStatus
{
  eBusInactive,
  eBusWaitForSuccesfullTX,
  eBusActivating,
  eBusActive
};


/**
 * \enum CanMessageConfirmation
 *
 */
enum CanMessageConfirmation
{
  eMessageSent,
  eMessageFailed
};

/**
 * \class CanProcessor
 *
 */
class CanProcessor  
{
public:
  typedef std::function<void(uint64_t,CanMessageConfirmation)>      ConfirmationCallback;
  typedef std::function<void(const CanPacket&,const std::string&)>  PGNCallback;
  typedef std::function<bool()>                                     UpdateCallback;
  typedef std::function<bool(const std::string&,CanBusStatus)>      BusStatusCallback;

  /**
   * \class Callback
   *
   */
  class Callback
  {
  public:
    virtual ~Callback() {}
    virtual uint64_t                get_time_tick() = 0;
    virtual void                    message_received(CanMessagePtr message,LocalECUPtr local, RemoteECUPtr remote,const std::string& bus_name) = 0;
    virtual void                    send_can_packet(const std::string& bus, const CanPacket& packet) = 0;
    virtual void                    on_remote_ecu(RemoteECUPtr remote,const std::string& bus_name) = 0;

    virtual uint32_t                create_mutex() = 0;
    virtual void                    delete_mutex(uint32_t) = 0;
    virtual void                    lock_mutex(uint32_t) = 0;
    virtual void                    unlock_mutex(uint32_t) = 0;
    virtual uint32_t                get_current_thread_id() const = 0;
  };

  CanProcessor(Callback*);
  virtual ~CanProcessor();

          void                    update();
          uint64_t                get_time_tick() const { return (_cback != nullptr)?_cback->get_time_tick():0ULL;}

          bool                    register_can_bus(const std::string& bus);
          CanBusStatus            get_bus_status(const std::string& bus) const;
          size_t                  get_all_buses(fixed_list<std::string,10>& buses) const;
  
          bool                    received_can_packet(const CanPacket& packet,const std::string& bus);

          bool                    send_can_message(CanMessagePtr message,LocalECUPtr local,RemoteECUPtr remote,
                                                        const std::initializer_list<std::string>& buses = std::initializer_list<std::string>());


          void                    message_received(CanMessagePtr message,LocalECUPtr local, RemoteECUPtr remote,const std::string& bus_name);

          LocalECUPtr             create_local_ecu(const CanName& name,
                                            uint8_t desired_address = BROADCATS_CAN_ADDRESS,
                                            const std::vector<std::string>& desired_buses = std::vector<std::string>());
          
          bool                    destroy_local_ecu(LocalECUPtr);
          bool                    destroy_local_ecu(const CanName& name);

          void                    on_remote_ecu(RemoteECUPtr remote,const std::string& bus_name)
          { if (_cback) _cback->on_remote_ecu(remote,bus_name); }
  
          void                    can_packet_confirm(uint64_t packet_id,CanMessageConfirmation);
          void                    can_packet_confirm(const CanPacket& packet,CanMessageConfirmation);

          CanDeviceDatabase&      device_db() { return _device_db; }
          const CanDeviceDatabase& device_db() const { return _device_db; }

          bool                    send_raw_packet(const CanPacket& packet,const std::string& bus,ConfirmationCallback fn = ConfirmationCallback());
          void                    register_pgn_receiver(uint32_t pgn, PGNCallback fn);
          void                    register_updater(UpdateCallback fn);
          void                    register_bus_callback(const std::string& bus_name, BusStatusCallback fn);

          uint32_t                create_mutex() { return (_cback != nullptr) ? _cback->create_mutex() : (uint32_t)-1; }
          void                    delete_mutex(uint32_t mtx_id) { if (_cback != nullptr) _cback->delete_mutex(mtx_id); }
          void                    lock_mutex(uint32_t mtx_id) { if (_cback != nullptr) _cback->lock_mutex(mtx_id); }
          void                    unlock_mutex(uint32_t mtx_id) { if (_cback != nullptr) _cback->unlock_mutex(mtx_id); }
          uint32_t                get_current_thread_id() const { return (_cback != nullptr) ?_cback->get_current_thread_id() : (uint32_t)-1; }

private:
          void                    on_request(const CanPacket&,const std::string&);
private:
  
  /**
   * \class SimpleTransport
   *
   * Inherited from :
   *             CanProtocol 
   */
  class SimpleTransport : public CanProtocol
  {
  public:
    SimpleTransport(CanProcessor* processor) : CanProtocol(processor) {}
    virtual ~SimpleTransport() {}

    virtual bool                    send_message(CanMessagePtr message, LocalECUPtr local, RemoteECUPtr remote, const std::string& bus_name);
  };

  Callback*                       _cback;
  mutable RecoursiveMutex         _mutex;
  CanDeviceDatabase               _device_db;

  /**
   * \struct Bus
   *
   */
  struct Bus
  {
    std::string                     _bus_name;
    CanBusStatus                    _status;
    uint64_t                        _time_tag;
    uint64_t                        _initial_packet_id;

    fifo<CanPacket>                 _packet_fifo;
    std::list<BusStatusCallback>    _bus_callbacks;
  };

  std::unordered_map<std::string,Bus> _bus_map;
  std::unordered_map<uint32_t,PGNCallback> _pgn_receivers;
  
  std::list<UpdateCallback>       _updaters;
  std::deque<CanProtocolPtr>      _transport_stack;


  // Real time structures

  /**
   * \struct PacketConfirmation
   *
   */
  struct PacketConfirmation
  {
    PacketConfirmation() = default;
    PacketConfirmation(uint64_t packet_id,ConfirmationCallback cback)
    : _packet_id(packet_id)
    , _callback(cback)
    { }

    uint64_t                      _packet_id;
    ConfirmationCallback          _callback;
  };
  fixed_list<PacketConfirmation>  _confirm_callbacks;

  /**
   * \struct ReceivedMessages
   *
   */
  struct ReceivedMessages
  {
    ReceivedMessages() = default;
    ReceivedMessages(CanMessagePtr message, LocalECUPtr local, RemoteECUPtr remote, const std::string& bus_name)
    : _message(message), _local(local), _remote(remote), _bus_name(bus_name)
    { }
    CanMessagePtr                   _message;
    LocalECUPtr                     _local;
    RemoteECUPtr                    _remote;
    std::string                     _bus_name;
  };
  fifo<ReceivedMessages>          _received_messages;
};



} // can
} // brt

