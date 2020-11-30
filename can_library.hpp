/**
 * can_library.hpp
 * 
 */ 

#pragma once

#include "can_constants.hpp"
#include "can_message.hpp"
#include "local_ecu.hpp"
#include "remote_ecu.hpp"


namespace brt {
namespace can {

/**
 * \class CanInterface
 *
 */
class CanInterface
{
public:
  /**
   * \class Callback
   *
   */
  class Callback
  {
  public:
    virtual ~Callback() {}
    virtual uint64_t                get_time_tick_nanoseconds() const = 0;
    virtual void                    message_received(CanMessagePtr message,LocalECUPtr local, RemoteECUPtr remote,const std::string& bus_name) = 0;
    virtual void                    send_can_packet(const std::string& bus, const CanPacket& packet) = 0;
    virtual void                    on_remote_ecu(RemoteECUPtr remote,const std::string& bus_name) = 0;

    virtual uint32_t                create_mutex() = 0;
    virtual void                    delete_mutex(uint32_t) = 0;
    virtual void                    lock_mutex(uint32_t) = 0;
    virtual void                    unlock_mutex(uint32_t) = 0;
    virtual uint32_t                get_current_thread_id() const = 0;
  };

public:  
  virtual ~CanInterface() {}

  virtual bool                    register_can_bus(const std::string& bus) = 0;
  virtual void                    update() = 0;
  virtual LocalECUPtr             create_local_ecu(const CanName& name,
                                            uint8_t desired_address = BROADCATS_CAN_ADDRESS,
                                            const std::vector<std::string>& desired_buses = std::vector<std::string>()) = 0;

  virtual RemoteECUPtr            register_abstract_remote_ecu(uint8_t address,const std::string& bus) = 0;

  virtual bool                    destroy_local_ecu(LocalECUPtr) = 0;
  virtual bool                    destroy_local_ecu(const CanName& name) = 0;

  virtual bool                    received_can_packet(const CanPacket& packet,const std::string& bus) = 0;
  virtual bool                    send_can_message(CanMessagePtr message,LocalECUPtr local,RemoteECUPtr remote,
                                                        const std::initializer_list<std::string>& buses = std::initializer_list<std::string>()) = 0;


  virtual void                    can_packet_confirm(uint64_t packet_id,CanMessageConfirmation) = 0;
  virtual void                    can_packet_confirm(const CanPacket& packet,CanMessageConfirmation) = 0;

  virtual void                    get_remote_ecus(fixed_list<RemoteECUPtr>& list, 
                                                const std::initializer_list<std::string>& buses = std::initializer_list<std::string>()) = 0;

          Callback*               cback() const { return _cback; }
protected:
  CanInterface(Callback*);

private:
  Callback*                       _cback;
};

CanInterface* create_can_interface(CanInterface::Callback*);

} // can
} // brt


