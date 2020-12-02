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
friend bool can_library_init(const LibraryConfig&);
friend bool can_library_release();

public:
  std::function<bool(const shared_pointer<CanTranscoder>&)> RequestCallback;

  virtual ~RemoteECU();
        
          void                    operator delete  ( void* ptr );

          shared_pointer<CanTranscoder> get_requested_pgn(uint32_t pgn) const;
          bool                    on_message_received(const CanMessagePtr& msg);

          bool                    is_ready() const 
          { 
            std::lock_guard<Mutex> l(_mutex);
            return _status_ready; 
          }

          bool                    queue_message(CanMessagePtr message, LocalECUPtr local, const std::string& bus_name);

private:
  RemoteECU(CanProcessor*,const CanName& name = CanName());
          
          void                    init_status();


  static  allocator<RemoteECU>*   _allocator;
  
  shared_pointer<CanTranscoder>   _sid;
  shared_pointer<CanTranscoder>   _eid;

  mutable RecoursiveMutex         _mutex;
  
  // status
  uint64_t                        _status_timer;
  bool                            _status_ready;

  struct MsgQueue
  {
    MsgQueue() = default;
    MsgQueue(CanMessagePtr message, CanECUPtr local, const std::string& bus_name)
    : _message(message), _local(local) 
    {
      strncpy(_bus_name, bus_name.c_str(), sizeof(_bus_name) - 1);
    }

    CanMessagePtr                   _message;
    CanECUPtr                       _local;
    char                            _bus_name[32];
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


