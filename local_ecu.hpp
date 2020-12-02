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
    eInitialized,
    eInavtive,
    eWaiting,
    eActive
  };

  virtual ~LocalECU();

          void operator delete  ( void* ptr );

private:
  LocalECU(CanProcessor*,const CanName& name, const std::string* buses, size_t num_buses);
          void                    claim_address(uint8_t address,const std::string& bus_name);
          void                    disable_device(const std::string& bus_name);

          bool                    send_message(const CanMessagePtr& message,const RemoteECUPtr& remote,const std::string& bus_name);

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

    explicit Queue(const CanMessagePtr& message,const RemoteECUPtr& remote, const std::string& bus_name)
    : _message(message)
    , _remote(remote)
    , _bus_name(bus_name)
    {}

    CanMessagePtr                   _message;
    RemoteECUPtr                    _remote;
    std::string                     _bus_name;
  };

  struct Container
  {
    Container() : _status(eWaiting), _time_tag(0ULL) {}
    ECUStatus                       _status;
    uint64_t                        _time_tag;
    fifo<Queue>                     _fifo;
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
  
  template<size_t _Size>
  explicit LocalECUPtr(CanProcessor* processor,const CanName& name, const fixed_list<std::string,_Size>& buses)
  {
    if (LocalECU::_allocator == nullptr)
      throw std::runtime_error("Library is not properly initialized");
  
    std::string bus_array[_Size];
    size_t num_buses = 0;
    for (auto iter = buses.begin(); ((iter != buses.end()) && (num_buses < _Size)); ++iter)
      bus_array[num_buses++] = (*iter);

    LocalECU* ecu = reinterpret_cast<LocalECU*>(LocalECU::_allocator->allocate());
    if (ecu == nullptr)
      ecu = reinterpret_cast<LocalECU*>(::malloc(sizeof(LocalECU)));

    ::new (ecu) LocalECU(processor,name,bus_array,num_buses);
    reset(ecu);
  }

  explicit LocalECUPtr(CanECUPtr ecu)
  {  reset(dynamic_cast<LocalECU*>(ecu.get())); }
};


} // can
} // brt
