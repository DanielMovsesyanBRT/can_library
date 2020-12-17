/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 15:0:44
 * File : local_ecu.hpp
 *
 */

#pragma once

#include "can_ecu.hpp"
#include "remote_ecu.hpp"
#include "can_message.hpp"
#include "can_utils.hpp"
#include "can_transcoder.hpp"

#include <deque>
#include <unordered_map>

namespace brt {
namespace can {

class CanDeviceDatabase;
class LocalECUPtr;
/**
 * \class LocalECU
 *
 */
class LocalECU : public CanECU
{
public:
friend CanProcessor;
friend CanDeviceDatabase;
friend LocalECUPtr;
friend bool can_library_init(const LibraryConfig&);
friend bool can_library_release();

  /**
   * \enum ECUStatus
   *
   */
  enum ECUStatus
  {
    eInactive,
    eWaiting,
    eActive
  };

  virtual ~LocalECU();

          void operator delete  ( void* ptr );

          bool                    set_transcoder(const CanTranscoderPtr&);
          void                    activate(uint8_t desired_address, 
                                      const std::initializer_list<ConstantString>& buses = std::initializer_list<ConstantString>());

private:
  LocalECU(CanProcessor*,const CanName& name);
          void                    claim_address(uint8_t address,const ConstantString& bus_name);
          void                    disable_device(const ConstantString& bus_name);

          bool                    send_message(const CanMessagePtr& message,const RemoteECUPtr& remote,
                                              const ConstantString& bus_name,bool check_status = true);
          CanMessagePtr           request_pgn(uint32_t pgn);

private:
  static  allocator<LocalECU>*    _allocator;
  Mutex                           _mutex;
  /**
   * \struct Queue
   *
   */
  struct Queue
  {
    Queue() = default;

    explicit Queue(const CanMessagePtr& message,const RemoteECUPtr& remote)
    : _message(message)
    , _remote(remote)
    {}

    CanMessagePtr                   _message;
    RemoteECUPtr                    _remote;
  };

  struct Container
  {
    Container(const ConstantString& bus_name = ConstantString()) : _bus_name(bus_name), _status(eInactive), _time_tag(0ULL)  {}
    
    CanString                       _bus_name;
    ECUStatus                       _status;
    uint64_t                        _time_tag;
    fifo<Queue>                     _fifo;
  };
  
  fixed_list<Container,32>        _container_map;
};


/**
 * \class LocalECUPtr
 *
 * Inherited from :
 *             shared_pointer 
 *             LocalECU 
 */
class LocalECUPtr : public shared_pointer<LocalECU>
{
public:
  LocalECUPtr() {}
  
  explicit LocalECUPtr(CanProcessor* processor,const CanName& name)
  {
    if (LocalECU::_allocator == nullptr)
      throw std::runtime_error("Library is not properly initialized");
  
    LocalECU* ecu = reinterpret_cast<LocalECU*>(LocalECU::_allocator->allocate());
    if (ecu == nullptr)
      ecu = reinterpret_cast<LocalECU*>(::malloc(sizeof(LocalECU)));

    ::new (ecu) LocalECU(processor,name);
    reset(ecu);
  }

  explicit LocalECUPtr(CanECUPtr ecu)
  {  reset(dynamic_cast<LocalECU*>(ecu.get())); }
};


} // can
} // brt
