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

#include "can_message.hpp"
#include "local_ecu.hpp"
#include "remote_ecu.hpp"
#include "can_device_database.hpp"


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
  CanProcessor();
  virtual ~CanProcessor();

public:
  typedef std::function<void(CanMessagePtr,CanMessageConfirmation)> ConfirmationCallback;
  typedef std::function<void(CanMessagePtr,const std::string&)>     PGNCallback;
  typedef std::function<bool()>                                     UpdateCallback;

  static  CanProcessor*           get() { return &_object; }
  
  /**
   * \class Callback
   *
   */
  class Callback
  {
  public:
    virtual ~Callback() {}
    virtual uint64_t                get_time_tick() = 0;
    virtual void                    message_received(CanMessagePtr message,LocalECUPtr local, RemoteECUPtr remote) = 0;
    virtual void                    send_can_frame(const std::string& bus, CanMessagePtr message) = 0;
  };

          bool                    initialize(Callback*);
          void                    update();
          uint64_t                get_time_tick() const { return (_cback != nullptr)?_cback->get_time_tick():0ULL;}

          bool                    init_bus(const std::string& bus);
          std::vector<std::string> get_all_buses() const;
  

          bool                    received_can_frame(CanMessagePtr message,const std::string& bus);
  
          void                    can_frame_confirm(uint64_t message_id);
          void                    can_frame_confirm(CanMessagePtr message);

          CanDeviceDatabase&      device_db() { return _device_db; }
          const CanDeviceDatabase& device_db() const { return _device_db; }

          bool                    send_raw_message(CanMessagePtr message,const std::string& bus,ConfirmationCallback && fn = ConfirmationCallback());
          void                    register_pgn_receiver(uint32_t pgn, PGNCallback && fn);
          void                    register_updater(UpdateCallback && fn);

private:
  static  CanProcessor            _object;
  Callback*                       _cback;

  CanDeviceDatabase               _device_db;

  /**
   * \struct WaitingFrame
   *
   */
  struct WaitingFrame
  {
    CanMessagePtr                   _message;
    uint64_t                        _time_tag;
    
    ConfirmationCallback           _callback;
    void*                          _param;

    bool operator==(const WaitingFrame& frm) const { return frm._message == _message; }
  };

  /**
   * \struct WaitingHash
   *
   */
  struct WaitingHash
  {
    size_t operator()(const WaitingFrame& stack) const
    { return std::hash<CanMessage*>()(stack._message.get()); }
  };

  std::unordered_set<WaitingFrame,WaitingHash> _waiting_stack;

  /**
   * \struct Bus
   *
   */
  struct Bus
  {
    std::string                     _bus_name;
    CanBusStatus                    _status;
    uint64_t                        _time_tag;
    uint64_t                        _initial_msg_id;

    std::deque<CanMessagePtr>       _msg_fifo;
  };

  std::unordered_map<std::string,Bus> _bus_map;
  std::unordered_map<uint32_t,PGNCallback> _pgn_receivers;
  std::vector<UpdateCallback>         _updaters;
};

} // can
} // brt

