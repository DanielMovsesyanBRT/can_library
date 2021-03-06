/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 14:56:47
 * File : remote_ecu.hpp
 *
 */

#pragma once

#include "can_ecu.hpp"
#include "can_utils.hpp"
#include "transcoders/can_transcoder.hpp"

#include <functional>
#include <mutex>

namespace brt {
namespace can {


class RemoteECUPtr;
class LocalECUPtr;
class CanDeviceDatabase;
/**
 * \class RemoteECU
 *
 */
class RemoteECU  : public CanECU
{
friend RemoteECUPtr;
friend CanDeviceDatabase;
friend CanProcessor;
friend bool can_library_init(const LibraryConfig&);
friend bool can_library_release();
friend shared_pointer<RemoteECU>;

public:
  virtual ~RemoteECU();
      

private:
  RemoteECU(CanProcessor*,const CanName& name = CanName());
          
          void                    init_status();
          shared_pointer<CanTranscoder> get_requested_pgn(uint32_t pgn) const;
          bool                    queue_message(const CanMessagePtr& message,const LocalECUPtr& local, const ConstantString& bus_name);
          bool                    on_message_received(const CanMessagePtr& msg);

          bool                    is_ready() const 
          { 
            std::lock_guard<Mutex> l(_mutex);
            return _status_ready; 
          }

          void                    operator delete  ( void* ptr );


  static  allocator<RemoteECU>*   _allocator;
  mutable RecursiveMutex         _mutex;
  
  // status
  uint64_t                        _status_timer;
  bool                            _status_ready;

  struct MsgQueue
  {
    MsgQueue() = default;
    MsgQueue(const CanMessagePtr& message,const CanECUPtr& local, const ConstantString& bus_name)
    : _message(message), _local(local), _bus_name(bus_name)
    {   }

    CanMessagePtr                   _message;
    CanECUPtr                       _local;
    CanString                       _bus_name;
  };

  fifo<MsgQueue>                  _queue;
};

/**
 * \class RemoteECUPtr
 *
 * Inherited from :
 *             shared_pointer 
 *             RemoteECU 
 */
class RemoteECUPtr : public shared_pointer<RemoteECU>
{
public:
  RemoteECUPtr() {}
  explicit RemoteECUPtr(CanProcessor*,const CanName& name = CanName());
  explicit RemoteECUPtr(CanECUPtr ecu)
  { reset( dynamic_cast<RemoteECU*>(ecu.get())); }
};

} // can
} // brt


