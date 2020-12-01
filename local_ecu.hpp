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
    eInavtive,
    eWaiting,
    eActive
  };

  virtual ~LocalECU();

          void operator delete  ( void* ptr );

private:
  LocalECU(CanProcessor*,const CanName& name = CanName());
          void                    claim_address(uint8_t address,const std::string& bus_name);
          void                    disable_device(const std::string& bus_name);

          bool                    send_message(CanMessagePtr message,RemoteECUPtr remote,const std::string& bus_name);

private:
  static  allocator<LocalECU>*    _allocator;
  Mutex                           _mutex;
  /**
   * \struct Queue
   *
   */
  struct Queue
  {
    explicit Queue(CanMessagePtr message,RemoteECUPtr remote)
    : _message(message)
    , _remote(remote)
    {}  

    explicit Queue(CanMessagePtr message,const std::string& bus_name)
    : _message(message)
    , _bus_name(bus_name)
    {}

    CanMessagePtr                   _message;
    RemoteECUPtr                    _remote;
    std::string                     _bus_name;
  };

  struct Container
  {
    Container() : _status(eInavtive), _time_tag(0ULL) {}
    ECUStatus                       _status;
    uint64_t                        _time_tag;
    std::deque<Queue>               _fifo;
  };
  
  typedef std::unordered_map<std::string,Container> ContainerMap;
  ContainerMap                    _container_map;
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
  explicit LocalECUPtr(CanProcessor* processor,const CanName& name = CanName());
  explicit LocalECUPtr(CanECUPtr ecu)
  {  reset(dynamic_cast<LocalECU*>(ecu.get())); }
};


} // can
} // brt
