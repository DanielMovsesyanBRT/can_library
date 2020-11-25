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
class TxSession;
class RxSession;

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
          
          TransportSession*       session() { return _session; }
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
 * \class TxAction
 *
 * Inherited from :
 *             Action 
 */
class TxAction : public Action
{
public:
  TxAction(TxSession* session);
  virtual ~TxAction() {}

          TxSession*              session();
          std::shared_ptr<TxAction> getptr() { return std::dynamic_pointer_cast<TxAction>(Action::getptr()); }
};


/**
 * \class SendBAM
 *
 * Inherited from :
 *             Action 
 */
class SendBAM : public TxAction
{
public:
  SendBAM(TxSession* session) : TxAction(session) {}
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
class SendData : public TxAction
{
public:
  SendData(TxSession* session,std::pair<uint8_t,uint8_t> range);
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
class SendRTS : public TxAction
{
public:
  SendRTS(TxSession* session) : TxAction(session){ }
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
class WaitCTS : public TxAction
{
public:
  WaitCTS(TxSession* session);
  virtual ~WaitCTS() {}

  virtual void                    update();
  virtual void                    pgn_received(const CanPacket& packet);
protected:
  uint64_t                        _time_tag;
  uint64_t                        _timeout_value;
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
  WaitEOM(TxSession* session) : WaitCTS(session) {}
  virtual ~WaitEOM() {}

  virtual void                    update();
  virtual void                    pgn_received(const CanPacket& packet);
};


/************************************
 * 
 *       Receiving Actions
 * 
 ***********************************/


/**
 * \class RxAction
 *
 * Inherited from :
 *             Action 
 */
class RxAction : public Action
{
public:
  RxAction(RxSession* session);
  virtual ~RxAction() {}

          RxSession*              session();
          std::shared_ptr<RxAction> getptr() { return std::dynamic_pointer_cast<RxAction>(Action::getptr()); }
};


/**
 * \class ReceiveData
 *
 * Inherited from :
 *             RxAction
 */
class ReceiveData : public RxAction
{
public:
  ReceiveData(RxSession* session,std::pair<uint8_t,uint8_t> range);
  virtual ~ReceiveData() {}

  virtual void                    update();
  virtual void                    pgn_received(const CanPacket& packet);

private:
  std::pair<uint8_t,uint8_t>      _range;
  uint32_t                        _current;
  uint64_t                        _time_tag;
  uint64_t                        _timeout_value;
  int                             _attempts;
};

} // can
} // brt

