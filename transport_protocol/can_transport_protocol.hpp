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
#include "can_transport_defines.hpp"

#include <unordered_set>
#include <deque>
#include <memory>


namespace brt {
namespace can {

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
     * @param  source : CanECUPtr 
     * @param  destination :  CanECUPtr 
     * @param  bus_name :  const std::string&
     * @return  TransportSessionPtr
     */
    TransportSessionPtr get_active(CanECUPtr source, CanECUPtr destination, const std::string& bus_name)
    {
      size_t hash = TransportSession::hash(source, destination, bus_name);
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

