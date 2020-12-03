
/**
 * can_transport_txsession.hpp
 * 
 */

#pragma once

#include "can_transport_session.hpp"

namespace brt {
namespace can {


class TxSessionPtr;

/**
 * \class TxSession
 *
 * Inherited from :
 *             TransportSession 
 */
class TxSession : public TransportSession
{
friend TxSessionPtr;
friend bool can_library_init(const LibraryConfig&);
friend bool can_library_release();

  /**
   * \fn  constructor TxSession
   *
   * @param  processor : CanProcessor*
   * @param  mutex :  Mutex*
   * @param   message : const CanMessagePtr&
   * @param   source : const CanECUPtr&
   * @param   destination :  const CanECUPtr&
   * @param   bus_name : const ConstantString&
   */
  TxSession(CanProcessor* processor, Mutex* mutex,const CanMessagePtr& message,const CanECUPtr& source,
                    const CanECUPtr& destination,const ConstantString& bus_name)
  : TransportSession(processor, mutex, message,  source, destination, bus_name)
  , _range(), _current(0), _time_tag(0), _timeout_value(0)
  {
    _state = (is_broadcast()) ? SendBAM : SendRTS;
  }

public:
  virtual ~TxSession()  {}

  virtual LocalECUPtr             local() { return LocalECUPtr(source_ecu()); }
  virtual RemoteECUPtr            remote() { return RemoteECUPtr(destination_ecu()); }

  virtual void                    update();
  virtual void                    pgn_received(const CanPacket& packet);
  virtual bool                    is_complete() const  { return (_state == None); }
  virtual void                    on_abort() { _state = None; }

          bool                    send_bam(CanMessage::ConfirmationCallback = CanMessage::ConfirmationCallback());
          bool                    send_data(uint8_t sequence, CanMessage::ConfirmationCallback = CanMessage::ConfirmationCallback());
          bool                    send_rts( CanMessage::ConfirmationCallback = CanMessage::ConfirmationCallback());

          void operator delete  ( void* ptr );

private:
  static allocator<TxSession>*    _allocator;

  enum TxStates
  {
    SendBAM,
    SendData,
    SendRTS,
    WaitCTS,
    WaitEOM,
    WaitDriverConfirmation,
    None
  }                               _state;

  range                           _range;
  uint32_t                        _current;
  uint64_t                        _time_tag;
  uint64_t                        _timeout_value;
};


/**
 * \class TxSessionPtr
 *
 * Inherited from :
 *             shared_pointer 
 *             TxSession 
 */
class TxSessionPtr : public shared_pointer<TxSession>
{
public:
  TxSessionPtr() {}
  TxSessionPtr(CanProcessor* processor, Mutex* mutex, const CanMessagePtr& message, const CanECUPtr& source,
                  const CanECUPtr& destination,const ConstantString& bus_name);
};

} // can
} // brt

