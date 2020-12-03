
/**
 * can_transport_session.hpp
 * 
 */

#pragma once

#include "can_utils.hpp"
#include "local_ecu.hpp"
#include "remote_ecu.hpp"
#include "can_message.hpp"

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
   * @param   message :  const CanMessagePtr&
   * @param   local :  const CanECUPtr&
   * @param   remote : const CanECUPtr&
   * @param   bus_name : const ConstantString&
   */
  TransportSession(CanProcessor* processor, Mutex* mutex, const CanMessagePtr& message,
                            const CanECUPtr& local,const CanECUPtr& remote,const ConstantString& bus_name)
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
          const CanString&        bus_name() const { return _bus_name; }
          bool                    is_broadcast() const { return !_destination; }
          
  virtual void                    update() = 0;
  virtual void                    pgn_received(const CanPacket& packet) = 0;
  virtual bool                    is_complete() const = 0;
  virtual void                    on_abort() = 0;

          void                    abort(uint8_t reason);
          CanMessagePtr           message() const { return _message; }

  virtual LocalECUPtr             local() = 0;
  virtual RemoteECUPtr            remote() = 0;


  static  size_t                  hash(const CanECUPtr& local,const CanECUPtr& remote,const ConstantString& bus_name)
  {
    size_t hash = std::hash<CanECU*>()(local.get());
    if (remote)
      hash ^= std::hash<CanECU*>()(remote.get());
    else
      hash ^= std::hash<uint32_t>()(0xFFFFFFFF);
            
    hash ^= std::_Hash_impl::hash(bus_name.data(), bus_name.length());
    return hash;
  }

  static  size_t                  hash(const shared_pointer<TransportSession>& session)
  {
    return hash(session->_source, session->_destination, session->_bus_name);
  }

protected:
  CanProcessor*                   _processor;
  Mutex*                          _mutex;

  CanMessagePtr                   _message;
  CanECUPtr                       _source;
  CanECUPtr                       _destination;
  CanString                       _bus_name;
};

typedef shared_pointer<TransportSession> TransportSessionPtr;


} // can
} // brt

