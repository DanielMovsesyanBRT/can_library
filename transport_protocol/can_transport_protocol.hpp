/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/23/2020 11:47:59
 * File : can_transport_protocol.hpp
 *
 */

#pragma once

#include "../can_protocol.hpp"
#include "can_transport_session.hpp"

#include <unordered_set>
#include <deque>
#include <memory>


// ISO 11783-3:2018 5.13.3:
// The required time interval between packets of a multi-packet broadcast message is 10 ms to 200 ms
#define BAM_TP_MINIMUM_TIMEOUT              (10) // ms

// ISO 11783-3:2018 timeouts
#define TRANSPORT_TIMEOUT_Tr                (200) // ms
#define TRANSPORT_TIMEOUT_Th                (500) // ms
#define TRANSPORT_TIMEOUT_T1                (750) // ms
#define TRANSPORT_TIMEOUT_T2                (1250) // ms
#define TRANSPORT_TIMEOUT_T3                (1250) // ms
#define TRANSPORT_TIMEOUT_T4                (1050) // ms


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

/**
 * \enum TPAbortReason
 *
 */
enum TPAbortReason
{
  AbortDuplicateConnection = 1, // Already in one or more connection-managed sessions and cannot support another
  AbortScarseResources = 2,     // System resources were needed for another task so this connection managed session was terminated
  AbortTimeout = 3,             // A timeout occurred and this is the connection abort to close the session
  AbortCTSWhileSending = 4,     // CTS messages received when data transfer is in progress
  AbortMaxTxRequestLimit = 5,   // Maximum retransmit request limit reached
  AbortUnexpectedDT = 6,        // Unexpected data transfer packet
  AbortBadSequenceNumber = 7,   // Bad sequence number (and software is not able to recover)
  AbortDupSequenceNumber = 8,   // Duplicate sequence number (and software is not able to recover)
  AbortSizeToBig = 9,           // “Total message size” is greater than 1785 byte
  AbortUnknown = 250            // If a Connection Abort reason is identified that is not listed in the table use code 250
};


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

  virtual bool                    send_message(CanMessagePtr message, LocalECUPtr local, RemoteECUPtr remote, const std::string& bus_name);

private:
          bool                    on_update();
          void                    on_pgn_callback(const CanPacket&,const std::string&);

private:
  
  typedef std::deque<TransportSessionPtr>         SessionQueue;
  typedef std::unordered_map<size_t,SessionQueue> SessionMap;
  
  enum StackDirection
  {
    eTransmit = 0,
    eReceive = 1,

    eNumDirections
  };

  /**
   * \struct SessonStack
   *
   */
  struct SessonStack
  {
    /**
     * \fn  add
     *
     * @param  session : TransportSessionPtr
     */
    void add(TransportSessionPtr session)
    {
      size_t hash = TransportSession::hash(*session);
      _session_queue[hash].push_back(session);
    }

    /**
     * \fn  update
     *
     * @param removed : std::vector<size_t>&
     */
    void update()
    {
      for (auto iter = _session_queue.begin(); iter != _session_queue.end(); )
      {
        if (iter->second.front()->update())
          iter->second.pop_front();

        if (iter->second.empty())
          iter = _session_queue.erase(iter);
        else
          iter++;
      }
    }

    /**
     * \fn  get_active
     *
     * @param  local : LocalECUPtr 
     * @param  remote :  RemoteECUPtr 
     * @param  & bus_name :  const std::string
     * @return  TransportSessionPtr
     */
    TransportSessionPtr get_active(LocalECUPtr local, RemoteECUPtr remote, const std::string& bus_name)
    {
      size_t hash = TransportSession::hash(local, remote, bus_name);
      auto iter = _session_queue.find(hash);
      if (iter == _session_queue.end())
        return TransportSessionPtr();

      return iter->second.front();
    }
    

    SessionMap                    _session_queue;
  }                               _session_stack[eNumDirections];


  RecoursiveMutex                 _mutex;
};

} // can
} // brt

