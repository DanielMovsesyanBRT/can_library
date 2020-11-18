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

#include <deque>
#include <unordered_map>

namespace brt {
namespace can {

/**
 * \class LocalECU
 *
 */
class LocalECU : public CanECU
{
public:
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

          void                    claim_address(uint8_t address,const std::string& bus);
          bool                    send_message(CanMessagePtr message,RemoteECUPtr remote);
private:

  /**
   * \struct Queue
   *
   */
  struct Queue
  {
    Queue(CanMessagePtr message,RemoteECUPtr remote)
    : _message(message)
    , _remote(remote)
    {}  
    CanMessagePtr                   _message;
    RemoteECUPtr                    _remote;
  };

  struct BusStatus
  {
    BusStatus() : _status(eInavtive), _time_tag(0ULL) {}
    ECUStatus                       _status;
    uint64_t                        _time_tag;
    std::deque<Queue>               _fifo;
  };
  
  typedef std::unordered_map<std::string,BusStatus> StatusMap;
  StatusMap                       _bus_status;
};


typedef std::shared_ptr<LocalECU>   LocalECUPtr;


} // can
} // brt
