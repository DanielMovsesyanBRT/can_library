/**
 * can_transport_rxsession.hpp
 * 
 */

#pragma once

#include "can_transport_session.hpp"

namespace brt {
namespace can {

class RxSessionPtr;
/**
 * \class RxSession
 *
 * Inherited from :
 *             TransportSession 
 */
class RxSession : public TransportSession
{
friend RxSessionPtr;
  RxSession(CanProcessor* processor, Mutex* mutex, CanECUPtr source,CanECUPtr destination,
                              const std::string& bus_name, const CanPacket& packet);

public:
  virtual ~RxSession() {}
          
  virtual LocalECUPtr             local() { return LocalECUPtr(destination_ecu()); }
  virtual RemoteECUPtr            remote() { return RemoteECUPtr(source_ecu()); }
  
  virtual void                    update();
  virtual void                    pgn_received(const CanPacket& packet);
  virtual bool                    is_complete() const {return _complete; }
  virtual void                    on_abort() { _complete = true; }

          size_t                  num_sequences() const { return  (message()->length() - 1) / 7 + 1; }
          bool                    sequence_received(uint8_t sequence, const uint8_t[7]);
          bool                    is_range_complete(const range& range) const;
          
          bool                    send_cts();
          bool                    send_eom();
                    
          void                    message_complete();

          void operator delete  ( void* ptr );

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

/**
 * \class RxSessionPtr
 *
 * Inherited from :
 *             shared_pointer 
 *             RxSession 
 */
class RxSessionPtr : public shared_pointer<RxSession>
{
public:
  RxSessionPtr() {}
  RxSessionPtr(CanProcessor* processor, Mutex* mutex, CanECUPtr source,CanECUPtr destination,
                              const std::string& bus_name, const CanPacket& packet);
};


} // can
} // brt

