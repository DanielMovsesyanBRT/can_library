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
/**
 * \class LocalECU
 *
 */
class LocalECU : public CanECU
{
public:
friend CanProcessor;
friend CanDeviceDatabase;
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

  LocalECU(CanProcessor*,const CanName& name = CanName());
  virtual ~LocalECU();

          void* operator new( size_t count )
          { 
            return _allocator.allocate(); 
          }

          void operator delete  ( void* ptr )
          { 
            _allocator.free(ptr); 
          }

private:
          void                    claim_address(uint8_t address,const std::string& bus_name);
          void                    disable_device(const std::string& bus_name);

          bool                    send_message(CanMessagePtr message,RemoteECUPtr remote,const std::string& bus_name);

private:
  static  allocator<LocalECU>     _allocator;

private:
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

//typedef std::shared_ptr<LocalECU>   LocalECUPtr;
typedef shared_pointer<LocalECU>   LocalECUPtr;


} // can
} // brt
