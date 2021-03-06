/**
 * can_library.hpp
 * 
 */ 

#pragma once

#include "can_constants.hpp"
#include "can_message.hpp"
#include "local_ecu.hpp"
#include "remote_ecu.hpp"
#include "can_transcoder.hpp"
#include "can_utils.hpp"


namespace brt {
namespace can {


/**
 * \class CanInterface
 *
 */
class CanInterface
{
public:
    typedef std::function<void(const CanTranscoderPtr&)> RequestCallback;

  /**
   * \class Callback
   *
   */
  class Callback
  {
  public:
    virtual ~Callback() {}
    virtual uint64_t                get_time_tick_nanoseconds() const = 0;
    virtual void                    message_received(const CanMessagePtr& message,const LocalECUPtr& local,const RemoteECUPtr& remote,const ConstantString& bus_name) = 0;
    virtual void                    send_can_packet(const ConstantString& bus, const CanPacket& packet) = 0;
    virtual void                    on_remote_ecu(const RemoteECUPtr& remote,const ConstantString& bus_name) = 0;

    virtual uint32_t                create_mutex() = 0;
    virtual void                    delete_mutex(uint32_t mutex_id) = 0;
    virtual void                    lock_mutex(uint32_t mutex_id) = 0;
    virtual void                    unlock_mutex(uint32_t mutex_id) = 0;
    virtual uint32_t                get_current_thread_id() const = 0;
  };

public:  
  virtual ~CanInterface() {}

  virtual bool                    register_can_bus(const ConstantString& bus) = 0;
  virtual void                    update() = 0;
  virtual LocalECUPtr             create_local_ecu(const CanName& name) = 0;
  virtual RemoteECUPtr            register_abstract_remote_ecu(uint8_t address,const ConstantString& bus) = 0;

  virtual bool                    destroy_local_ecu(const LocalECUPtr&) = 0;
  virtual bool                    destroy_local_ecu(const CanName& name) = 0;

  virtual bool                    received_can_packet(const CanPacket& packet,const ConstantString& bus) = 0;
  virtual bool                    send_can_message(const CanMessagePtr& message,const LocalECUPtr& local,const RemoteECUPtr& remote,
                                                        const std::initializer_list<ConstantString>& buses = std::initializer_list<ConstantString>()) = 0;


  virtual void                    can_packet_confirm(uint64_t packet_id,CanMessageConfirmation) = 0;
  virtual void                    can_packet_confirm(const CanPacket& packet,CanMessageConfirmation) = 0;

  virtual void                    get_remote_ecus(fixed_list<RemoteECUPtr>& list, 
                                                const std::initializer_list<ConstantString>& buses = std::initializer_list<ConstantString>()) = 0;

  virtual bool                    request_pgn(uint32_t pgn,const LocalECUPtr& local,const RemoteECUPtr& remote,const RequestCallback& callback) = 0;

protected:
  CanInterface(Callback*);
          Callback*               cback() const { return _cback; }

private:
  Callback*                       _cback;
};

bool can_library_init(const LibraryConfig& cfg = LibraryConfig());
bool can_library_release();

CanInterface* create_can_interface(CanInterface::Callback*);
void delete_can_interface(CanInterface*);

} // can
} // brt


