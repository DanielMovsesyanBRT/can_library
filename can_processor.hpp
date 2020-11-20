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
  typedef std::function<bool(const std::string&,CanBusStatus)>      BusStatusCallback;

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
          CanBusStatus            get_bus_status(const std::string& bus) const;
          std::vector<std::string> get_all_buses() const;
  
          bool                    received_can_frame(CanMessagePtr message,const std::string& bus);
          LocalECUPtr             create_local_ecu(const CanName& name,
                                            uint8_t desired_address = BROADCATS_CAN_ADDRESS,
                                            const std::vector<std::string>& desired_buses = std::vector<std::string>());
  
          void                    can_frame_confirm(uint64_t message_id,CanMessageConfirmation);
          void                    can_frame_confirm(CanMessagePtr message,CanMessageConfirmation);

          CanDeviceDatabase&      device_db() { return _device_db; }
          const CanDeviceDatabase& device_db() const { return _device_db; }

          bool                    send_raw_message(CanMessagePtr message,const std::string& bus,ConfirmationCallback fn = ConfirmationCallback());
          void                    register_pgn_receiver(uint32_t pgn, PGNCallback fn);
          void                    register_updater(UpdateCallback fn);
          void                    register_bus_callback(const std::string& bus_name, BusStatusCallback fn);

private:
  static  CanProcessor            _object;
  Callback*                       _cback;
  CanDeviceDatabase               _device_db;

  /**
   * \struct MessageConfirmation
   *
   */
  struct MessageConfirmation
  {
    MessageConfirmation(CanMessagePtr message,ConfirmationCallback cback)
    : _message(message)
    , _callback(cback)
    { }

    CanMessagePtr                 _message;
    ConfirmationCallback          _callback;
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

    std::deque<CanMessagePtr>       _msg_fifo;
    std::list<BusStatusCallback>    _bus_callbacks;
  };

  std::unordered_map<std::string,Bus> _bus_map;
  std::unordered_map<uint32_t,PGNCallback> _pgn_receivers;
  
  std::list<MessageConfirmation>  _confirm_callbacks;
  std::list<UpdateCallback>       _updaters;
};

} // can
} // brt

