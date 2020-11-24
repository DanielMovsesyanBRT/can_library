
/**
 * can_transport_session.hpp
 * 
 */

#pragma once

#include "../can_utils.hpp"
#include "../local_ecu.hpp"
#include "../remote_ecu.hpp"

#include "can_transport_actions.hpp"

namespace brt {
namespace can {

class CanProcessor;

/**
 * \class TransportSession
 *
 */
class TransportSession
{
public:
  /**
   * \fn  constructor TransportSession
   *
   * @param  message : CanMessagePtr 
   * @param  local : LocalECUPtr 
   * @param  remote : RemoteECUPtr 
   */
  TransportSession(CanProcessor* processor, Mutex* mutex, CanMessagePtr message,LocalECUPtr local,RemoteECUPtr remote,const std::string& bus_name)
  : _processor(processor), _mutex(mutex), _message(message), _local(local), _remote(remote), _bus_name(bus_name)
  { 
    if (is_broadcast())
      _action = std::make_shared<SendBAM>(this);
    else
      _action = std::make_shared<SendRTS>(this);
  }

  TransportSession(const TransportSession& session) = default;
  TransportSession& operator=(const TransportSession& session) = default;

  /**
   * \fn  destructor TransportSession
   *
   */
  ~TransportSession() 
  { }

          CanProcessor*           processor() { return _processor; }
          CanMessagePtr           message() const { return _message; }
          LocalECUPtr             local() const { return _local; }
          RemoteECUPtr            remote() const { return _remote; }
          std::string             bus_name() const { return _bus_name; }
          bool                    is_broadcast() const { return !_remote; }
          
          bool                    update();
          bool                    pgn_received(const CanPacket& packet);
          void                    change_action(ActionPtr old_action,ActionPtr new_action);
          void                    abort(uint8_t reason);
  
  static  size_t                  hash(LocalECUPtr local,RemoteECUPtr remote,const std::string& bus_name)
  {
    size_t hash = std::hash<LocalECU*>()(local.get());
    if (remote)
      hash ^= std::hash<RemoteECU*>()(remote.get());
    else
      hash ^= std::hash<uint32_t>()(0xFFFFFFFF);
            
    hash ^= std::hash<std::string>()(bus_name);
    return hash;
  }

  static  size_t                  hash(const TransportSession& session)
  {
    return hash(session._local, session._remote, session._bus_name);
  }

protected:
  CanProcessor*                   _processor;
  Mutex*                          _mutex;

  CanMessagePtr                   _message;
  LocalECUPtr                     _local;
  RemoteECUPtr                    _remote;
  std::string                     _bus_name;

  ActionPtr                       _action;
};


typedef std::shared_ptr<TransportSession> TransportSessionPtr;


} // can
} // brt

