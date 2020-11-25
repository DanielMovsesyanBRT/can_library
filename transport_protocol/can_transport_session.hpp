
/**
 * can_transport_session.hpp
 * 
 */

#pragma once

#include "../can_utils.hpp"
#include "../local_ecu.hpp"
#include "../remote_ecu.hpp"

#include "can_transport_actions.hpp"
#include "can_transport_defines.hpp"

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
   * @param  processor : CanProcessor* 
   * @param  mutex :  Mutex* 
   * @param  local :  LocalECUPtr 
   * @param  remote : RemoteECUPtr 
   * @param  & bus_name : const std::string
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
          
          bool                    update();
          bool                    pgn_received(const CanPacket& packet);
          void                    change_action(ActionPtr old_action,ActionPtr new_action);
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

  static  size_t                  hash(const TransportSession& session)
  {
    return hash(session._source, session._destination, session._bus_name);
  }

protected:
  CanProcessor*                   _processor;
  Mutex*                          _mutex;

  CanMessagePtr                   _message;
  CanECUPtr                       _source;
  CanECUPtr                       _destination;
  std::string                     _bus_name;

  ActionPtr                       _action;
};

typedef std::shared_ptr<TransportSession> TransportSessionPtr;


/**
 * \class TxSession
 *
 * Inherited from :
 *             TransportSession 
 */
class TxSession : public TransportSession
{
public:
  TxSession(CanProcessor* processor, Mutex* mutex, CanMessagePtr message, CanECUPtr source,CanECUPtr destination,const std::string& bus_name)
  : TransportSession(processor, mutex, message,  source, destination, bus_name)
   {
    if (is_broadcast())
      _action = std::make_shared<SendBAM>(this);
    else
      _action = std::make_shared<SendRTS>(this);
  }

  virtual ~TxSession()  {}

  virtual LocalECUPtr             local() { return std::dynamic_pointer_cast<LocalECU>(source_ecu()); }
  virtual RemoteECUPtr            remote() { return std::dynamic_pointer_cast<RemoteECU>(destination_ecu()); }

          bool                    send_bam(CanMessage::ConfirmationCallback = CanMessage::ConfirmationCallback());
          bool                    send_data(uint8_t sequence, CanMessage::ConfirmationCallback = CanMessage::ConfirmationCallback());
          bool                    send_rts( CanMessage::ConfirmationCallback = CanMessage::ConfirmationCallback());
};


/**
 * \class RxSession
 *
 * Inherited from :
 *             TransportSession 
 */
class RxSession : public TransportSession
{
public:
  RxSession(CanProcessor* processor, Mutex* mutex, CanECUPtr source,CanECUPtr destination,
                              const std::string& bus_name, const CanPacket& packet);
  virtual ~RxSession() {}
          
  virtual LocalECUPtr             local() { return std::dynamic_pointer_cast<LocalECU>(destination_ecu()); }
  virtual RemoteECUPtr            remote() { return std::dynamic_pointer_cast<RemoteECU>(source_ecu()); }
  
          bool                    sequence_received(uint8_t sequence, const uint8_t[7]);
          bool                    is_complete() const;
          bool                    is_range_complete(const std::pair<uint8_t,uint8_t>& range) const;
          
          bool                    send_cts();
          bool                    send_eom();

          void                    message_complete();
private:
  std::array<bool,MAX_TP_PACKETS> _received_map;
  uint8_t                         _max_packets;
};



} // can
} // brt

