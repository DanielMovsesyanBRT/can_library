/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/23/2020 11:47:59
 * File : can_transport_protocol.hpp
 *
 */

#pragma once

#include "can_protocol.hpp"

#include <unordered_set>
#include <deque>
#include <memory>

namespace brt {
namespace can {

/**
 * \enum TPControlByte
 *
 */
enum TPControlByte
{
  RTS = 16,
  CTS = 17,
  EOM = 19, // End of message ACK
  Abort = 255,
  BAM = 32
};


class TransmitSession;
/**
 * \class Action
 *
 */
class Action : public std::enable_shared_from_this<Action>
{
public:
  Action(TransmitSession* session) : _session(session) {}
  virtual ~Action() {}

  virtual void                    update() = 0;
  virtual void                    pgn_received(const CanPacket& packet) = 0;
          
          TransmitSession*        session() { return _session; }
          std::shared_ptr<Action> getptr() { return shared_from_this(); }

private:
  TransmitSession*                _session;
};

typedef std::shared_ptr<Action> ActionPtr;

/**
 * \class SendBAM
 *
 * Inherited from :
 *             Action 
 */
class SendBAM : public Action
{
public:
  SendBAM(TransmitSession* session) : Action(session) {}
  virtual ~SendBAM() {}

  virtual void                    update();
  virtual void                    pgn_received(const CanPacket& packet) { }
};

/**
 * \class SendData
 *
 * Inherited from :
 *             Action 
 */
class SendData : public Action
{
public:
  SendData(TransmitSession* session,std::pair<uint8_t,uint8_t> range);
  virtual ~SendData() {}

  virtual void                    update();
  virtual void                    pgn_received(const CanPacket& packet);

private:
  std::pair<uint8_t,uint8_t>      _range;
  uint32_t                        _current;
  uint64_t                        _time_tag;
};

/**
 * \class SendRTS
 *
 * Inherited from :
 *             Action 
 */
class SendRTS : public Action
{
public:
  SendRTS(TransmitSession* session) : Action(session){ }
  virtual ~SendRTS() {}

  virtual void                    update();
  virtual void                    pgn_received(const CanPacket& packet) {}
};

/**
 * \class WaitCTS
 *
 * Inherited from :
 *             Action 
 */
class WaitCTS : public Action
{
public:
  WaitCTS(TransmitSession* session);
  virtual ~WaitCTS() {}

  virtual void                    update();
  virtual void                    pgn_received(const CanPacket& packet);
private:
  uint64_t                        _time_tag;
};

/**
 * \class WaitEOM
 *
 * Inherited from :
 *             Action 
 */
class WaitEOM : public Action
{
public:
  WaitEOM(TransmitSession* session);
  virtual ~WaitEOM() {}

  virtual void                    update();
  virtual void                    pgn_received(const CanPacket& packet);
private:
  uint64_t                        _time_tag;
};


/**
 * \class TransmitSession
 *
 */
class TransmitSession
{
public:
  /**
   * \fn  constructor TransmitSession
   *
   * @param  message : CanMessagePtr 
   * @param  local : LocalECUPtr 
   * @param  remote : RemoteECUPtr 
   */
  TransmitSession(CanProcessor* processor, CanMessagePtr message,LocalECUPtr local,RemoteECUPtr remote,const std::string& bus_name)
  : _processor(processor), _message(message), _local(local), _remote(remote), _bus_name(bus_name)
  { 
    if (is_broadcast())
      _action = std::make_shared<SendBAM>(this);
    else
      _action = std::make_shared<SendRTS>(this);
  }

  TransmitSession(const TransmitSession& session) = default;
  TransmitSession& operator=(const TransmitSession& session) = default;

  /**
   * \fn  destructor TransmitSession
   *
   */
  ~TransmitSession() 
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
  
public:

  /**
   * \struct hash
   *
   */
  struct hash
  {
  public:
    size_t operator()(std::shared_ptr<TransmitSession> session) const
    {
      size_t hash = std::hash<LocalECU*>()(session->_local.get());
      if (session->_remote)
        hash ^= std::hash<RemoteECU*>()(session->_remote.get());
      else
        hash ^= std::hash<uint32_t>()(0xFFFFFFFF);
              
      hash ^= std::hash<std::string>()(session->_bus_name);
      return hash;
    }

    size_t operator()(LocalECUPtr local,RemoteECUPtr remote,const std::string& bus_name) const
    {
      size_t hash = std::hash<LocalECU*>()(local.get());
      if (remote)
        hash ^= std::hash<RemoteECU*>()(remote.get());
      else
        hash ^= std::hash<uint32_t>()(0xFFFFFFFF);
              
      hash ^= std::hash<std::string>()(bus_name);
      return hash;
    }

  };

protected:
  CanProcessor*                   _processor;
  CanMessagePtr                   _message;
  LocalECUPtr                     _local;
  RemoteECUPtr                    _remote;
  std::string                     _bus_name;

  ActionPtr                       _action;
};

typedef std::shared_ptr<TransmitSession> TransmitSessionPtr;

/**
 * \class CanTransportProtocol
 *
 * Inherited from :
 *             CanProtocol 
 */
class CanTransportProtocol  : public CanProtocol
{
public:
  CanTransportProtocol(CanProcessor* processor);
  virtual ~CanTransportProtocol();

  virtual bool                    send_message(CanMessagePtr message, LocalECUPtr local, RemoteECUPtr remote);
  virtual bool                    send_message(CanMessagePtr message, LocalECUPtr local, const std::vector<std::string>& buses);

private:
          bool                    on_update();
          void                    on_pg_callback(const CanPacket&,const std::string&);

private:
  
  typedef std::unordered_set<TransmitSessionPtr,TransmitSession::hash> ActiveSessions;
  ActiveSessions                  _active_session;
  std::deque<TransmitSessionPtr>  _session_queue;
  Mutex                           _mutex;
};

} // can
} // brt

