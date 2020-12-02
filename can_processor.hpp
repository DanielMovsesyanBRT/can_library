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
#include <atomic>

#include "can_library.hpp"
#include "can_utils.hpp"
#include "can_device_database.hpp"

#include "can_message.hpp"
#include "local_ecu.hpp"
#include "remote_ecu.hpp"
#include "transport_protocol/can_transport_rxsession.hpp"
#include "transport_protocol/can_transport_txsession.hpp"
#include "transcoders/can_transcoder.hpp"

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
 * \class CanProcessor
 *
 */
class CanProcessor : public CanInterface
{
friend CanInterface* create_can_interface(CanInterface::Callback*);
private:
  CanProcessor(Callback*);

public:
  typedef std::function<void(uint64_t,CanMessageConfirmation)>      ConfirmationCallback;
  typedef std::function<void(const CanPacket&,const std::string&)>  PGNCallback;
  typedef std::function<bool()>                                     UpdateCallback;
  typedef std::function<bool(const std::string&,CanBusStatus)>      BusStatusCallback;

  virtual ~CanProcessor();

  virtual void                    update();

  virtual bool                    register_can_bus(const std::string& bus);
          CanBusStatus            get_bus_status(const std::string& bus) const;
          
          template<size_t _Size>
          size_t                  get_all_buses(fixed_list<std::string,_Size>& buses) const
          {
            buses.clear();
            std::lock_guard<RecoursiveMutex> l(_mutex);
            for (auto bus : _bus_map)
              buses.push(bus.first);

            return buses.size();
          }
  
  virtual bool                    received_can_packet(const CanPacket& packet,const std::string& bus);

  virtual bool                    send_can_message(const CanMessagePtr& message,const LocalECUPtr& local,const RemoteECUPtr& remote,
                                                        const std::initializer_list<std::string>& buses = std::initializer_list<std::string>());

          void                    message_received(const CanMessagePtr& message,const LocalECUPtr& local,const RemoteECUPtr& remote,const std::string& bus_name);

  virtual LocalECUPtr             create_local_ecu(const CanName& name,
                                            uint8_t desired_address = BROADCATS_CAN_ADDRESS,
                                            const std::initializer_list<std::string>& buses = std::initializer_list<std::string>());
  virtual RemoteECUPtr            register_abstract_remote_ecu(uint8_t address,const std::string& bus);

  virtual bool                    destroy_local_ecu(const LocalECUPtr&);
  virtual bool                    destroy_local_ecu(const CanName& name);

          void                    on_remote_ecu(const RemoteECUPtr& remote,const std::string& bus_name)
          { cback()->on_remote_ecu(remote,bus_name); }
  
  virtual void                    can_packet_confirm(uint64_t packet_id,CanMessageConfirmation);
  virtual void                    can_packet_confirm(const CanPacket& packet,CanMessageConfirmation);

  virtual void                    get_remote_ecus(fixed_list<RemoteECUPtr>& list, 
                                                const std::initializer_list<std::string>& buses = std::initializer_list<std::string>())
          { device_db().get_remote_ecus(list, buses); }
          
  virtual bool                    request_pgn(uint32_t pgn,const LocalECUPtr& local,const RemoteECUPtr& remote,const RequestCallback& callback);

          CanDeviceDatabase&      device_db() { return _device_db; }
          const CanDeviceDatabase& device_db() const { return _device_db; }

          bool                    send_raw_packet(const CanPacket& packet,const std::string& bus_name,const ConfirmationCallback& fn = ConfirmationCallback());
          void                    register_pgn_receiver(uint32_t pgn, const PGNCallback& fn);
          void                    register_updater(const UpdateCallback& fn);
          void                    register_bus_callback(const std::string& bus_name,const BusStatusCallback& fn);

          uint64_t                get_time_tick() const;
          uint32_t                create_mutex() { return cback()->create_mutex(); }
          void                    delete_mutex(uint32_t mtx_id) { cback()->delete_mutex(mtx_id); }
          void                    lock_mutex(uint32_t mtx_id) { cback()->lock_mutex(mtx_id); }
          void                    unlock_mutex(uint32_t mtx_id) { cback()->unlock_mutex(mtx_id); }
          uint32_t                get_current_thread_id() const { return cback()->get_current_thread_id(); }


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

    virtual bool                    send_message(const CanMessagePtr& message,const LocalECUPtr& local,
                                              const RemoteECUPtr& remote, const std::string& bus_name);
  };

  mutable RecoursiveMutex         _mutex;
  CanDeviceDatabase               _device_db;
  std::atomic_uint_fast64_t       _remote_name_counter;

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
    PacketConfirmation(uint64_t packet_id,const ConfirmationCallback& cback)
    : _packet_id(packet_id)
    , _callback(cback)
    { }

    uint64_t                      _packet_id;
    ConfirmationCallback          _callback;
  };
  fixed_list<PacketConfirmation>  _confirm_callbacks;

  /**
   * \struct RequestedPGNs
   *
   */
  struct RequestedPGNs
  {
    RequestedPGNs() : _pgn(0) {}
    RequestedPGNs(uint32_t pgn, const CanECUPtr& ecu,const RequestCallback& callback)
    : _pgn(pgn), _ecu(ecu),  _callback(callback) {}
    uint32_t                        _pgn;
    CanECUPtr                       _ecu;
    RequestCallback                 _callback;
  };

  fixed_list<RequestedPGNs>       _requested_pgns;
};



} // can
} // brt

