/**
 * can_transport_action.hpp
 * 
 */

#pragma once

#include <memory>
#include "../can_message.hpp"


namespace brt {
namespace can {

class TransportSession;
/**
 * \class Action
 *
 */
class Action : public std::enable_shared_from_this<Action>
{
public:
  Action(TransportSession* session) : _session(session) {}
  virtual ~Action() {}

  virtual void                    update() = 0;
  virtual void                    pgn_received(const CanPacket& packet) = 0;
          
          TransportSession*        session() { return _session; }
          std::shared_ptr<Action> getptr() { return shared_from_this(); }

private:
  TransportSession*                _session;
};

typedef std::shared_ptr<Action> ActionPtr;


/************************************
 * 
 *       Transmitting Actions
 * 
 ***********************************/

/**
 * \class SendBAM
 *
 * Inherited from :
 *             Action 
 */
class SendBAM : public Action
{
public:
  SendBAM(TransportSession* session) : Action(session) {}
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
  SendData(TransportSession* session,std::pair<uint8_t,uint8_t> range);
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
  SendRTS(TransportSession* session) : Action(session){ }
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
  WaitCTS(TransportSession* session);
  virtual ~WaitCTS() {}

  virtual void                    update();
  virtual void                    pgn_received(const CanPacket& packet);
protected:
  uint64_t                        _time_tag;
};

/**
 * \class WaitEOM
 *
 * Inherited from :
 *             Action 
 */
class WaitEOM : public WaitCTS
{
public:
  WaitEOM(TransportSession* session) : WaitCTS(session) {}
  virtual ~WaitEOM() {}

  virtual void                    update();
  virtual void                    pgn_received(const CanPacket& packet);
};


} // can
} // brt

