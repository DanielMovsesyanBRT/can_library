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

  bool                            initialize(Callback*);
  void                            update();

  bool                            init_bus(const std::string& bus);
  
  // Send Raw message - mostly called from within library
  template<typename R>
  bool                            send_raw_message(CanMessagePtr message,const std::string& bus,R && fn)
  {
    if (_cback == nullptr)
      return false;
  
    using namespace std::placeholders;
    WaitingFrame frm;
    frm._message = message;
    frm._time_tag = _cback->get_time_tick();
    frm._callback = std::bind(std::forward<R>(fn), _1, _2);
    _waiting_stack.insert(frm);

    _cback->send_can_frame(bus,message);
    return true;
  }





  bool                            send_message(CanMessagePtr message,LocalECUPtr local,RemoteECUPtr remote = RemoteECUPtr());
  bool                            received_can_frame(CanMessagePtr message,const std::string& bus);
  
  void                            can_frame_confirm(uint64_t message_id);
  void                            can_frame_confirm(CanMessagePtr message);

  CanDeviceDatabase&              device_db() { return _device_db; }
  const CanDeviceDatabase&        device_db() const { return _device_db; }

  // void                            confirmation_wait(CanMessagePtr,ConfirmationCallback && callback,void* param);
  


  template<typename R,typename... ArgsT>
  void                            register_pgn_receiver(uint32_t pgn, R && fn,ArgsT&&... args)
  {
    _pgn_receivers.insert(std::unordered_map<uint32_t,PGNCallback>::value_type(pgn,
          std::bind(std::forward<R>(fn),std::forward<ArgsT>(args)...)));
  }

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
   * \struct MsgFifoItem
   *
   */
  struct MsgFifoItem
  {
    MsgFifoItem(CanMessagePtr msg,LocalECUPtr local,RemoteECUPtr remote)
    : _message(msg)
    , _local_ecu(local)
    , _remote_ecu(remote)
    { }

    CanMessagePtr                   _message;
    LocalECUPtr                     _local_ecu;
    RemoteECUPtr                    _remote_ecu;
  };

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

    std::deque<MsgFifoItem>         _msg_fifo;
  };

  std::unordered_map<std::string,Bus> _bus_map;
  std::unordered_map<uint32_t,PGNCallback> _pgn_receivers;
};

} // can
} // brt

