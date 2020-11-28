/**
 * can_transport_rxsession.hpp
 * 
 */

#pragma once

#include "can_transport_session.hpp"

namespace brt {
namespace can {

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
          
  virtual LocalECUPtr             local() { return dynamic_shared_cast<LocalECU>(destination_ecu()); }
  virtual RemoteECUPtr            remote() { return dynamic_shared_cast<RemoteECU>(source_ecu()); }
  
  virtual void                    update();
  virtual void                    pgn_received(const CanPacket& packet);
  virtual bool                    is_complete() const {return _complete; }

          size_t                  num_sequences() const { return  (message()->length() - 1) / 7 + 1; }
          bool                    sequence_received(uint8_t sequence, const uint8_t[7]);
          bool                    is_range_complete(const range& range) const;
          
          bool                    send_cts();
          bool                    send_eom();
                    
          void                    message_complete();

          void* operator new( size_t count )  { return _allocator.allocate(); }
          void operator delete  ( void* ptr )  {  _allocator.free(ptr); }

private:
  static  allocator<RxSession,0,32> _allocator;

private:
  std::array<bool,MAX_TP_PACKETS> _received_map;
  uint8_t                         _max_packets;

  range                           _range;
  uint32_t                        _current;
  uint64_t                        _time_tag;
  uint64_t                        _timeout_value;
  int                             _attempts;

  bool                            _complete;
};

typedef shared_pointer<RxSession> RxSessionPtr;


} // can
} // brt

