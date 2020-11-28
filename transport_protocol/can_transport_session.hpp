
/**
 * can_transport_session.hpp
 * 
 */

#pragma once

#include "../can_utils.hpp"
#include "../local_ecu.hpp"
#include "../remote_ecu.hpp"

// #include "can_transport_actions.hpp"
#include "can_transport_defines.hpp"

namespace brt {
namespace can {

class CanProcessor;

/**
 * \class TransportSession
 *
 */
class TransportSession : public shared_class<TransportSession>
{
public:
  typedef std::pair<uint8_t,uint8_t>  range;
  /**
   * \fn  constructor TransportSession
   *
   * @param  processor : CanProcessor* 
   * @param  mutex :  Mutex* 
   * @param  local :  LocalECUPtr 
   * @param  remote : RemoteECUPtr 
   * @param  bus_name : const std::string&
   */
  TransportSession(CanProcessor* processor, Mutex* mutex, CanMessagePtr message, CanECUPtr local,CanECUPtr remote,const std::string& bus_name)
  : _processor(processor), _mutex(mutex), _message(message), _source(local), _destination(remote), _bus_name(bus_name)
  { 
  }

  TransportSession(const TransportSession& session) = default;
  TransportSession& operator=(const TransportSession& session) = default;

  /**
   * \fn  destructor TransportSession
   *
   */
  virtual ~TransportSession() 
  { }

          CanProcessor*           processor() { return _processor; }
          CanECUPtr               source_ecu() const { return _source; }
          CanECUPtr               destination_ecu() const { return _destination; }
          std::string             bus_name() const { return _bus_name; }
          bool                    is_broadcast() const { return !_destination; }
          
  virtual void                    update() = 0;
  virtual void                    pgn_received(const CanPacket& packet) = 0;
  virtual bool                    is_complete() const = 0;
  virtual void                    on_abort() = 0;

          void                    abort(uint8_t reason);
          CanMessagePtr           message() const { return _message; }

  virtual LocalECUPtr             local() = 0;
  virtual RemoteECUPtr            remote() = 0;


  static  size_t                  hash(CanECUPtr local,CanECUPtr remote,const std::string& bus_name)
  {
    size_t hash = std::hash<CanECU*>()(local.get());
    if (remote)
      hash ^= std::hash<CanECU*>()(remote.get());
    else
      hash ^= std::hash<uint32_t>()(0xFFFFFFFF);
            
    hash ^= std::hash<std::string>()(bus_name);
    return hash;
  }

  static  size_t                  hash(shared_pointer<TransportSession> session)
  {
    return hash(session->_source, session->_destination, session->_bus_name);
  }

protected:
  CanProcessor*                   _processor;
  Mutex*                          _mutex;

  CanMessagePtr                   _message;
  CanECUPtr                       _source;
  CanECUPtr                       _destination;
  std::string                     _bus_name;
};

typedef shared_pointer<TransportSession> TransportSessionPtr;


} // can
} // brt

