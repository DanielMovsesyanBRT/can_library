/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/23/2020 11:47:59
 * File : can_transport_protocol.hpp
 *
 */

#pragma once

#include "can_protocol.hpp"
#include "can_message.hpp"
#include "can_ecu.hpp"
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

  virtual bool                    send_message(const CanMessagePtr& message,const LocalECUPtr& local,
                                            const RemoteECUPtr& remote, const ConstantString& bus_name);

private:
          bool                    on_update();
          void                    on_pgn_callback(const CanPacket&,const ConstantString&);

private:
  typedef fifo<TransportSessionPtr,64>    SessionQueue;
  typedef std::pair<size_t,SessionQueue>  HashPair;
  typedef fixed_list<HashPair,32>         SessionMap;
  
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
      size_t hash = TransportSession::hash(session);
      auto iter = _session_queue.find_if([hash](const HashPair& pair)->bool
      {
        return pair.first == hash;
      });

      if (iter == _session_queue.end())
        iter = _session_queue.push(HashPair(hash,SessionQueue()));

      if (iter != _session_queue.end())
        iter->second.push(session);
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
        iter->second.front()->update();
        if (iter->second.front()->is_complete())
          iter->second.pop();

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
    TransportSessionPtr get_active(const CanECUPtr& source,const CanECUPtr& destination, const ConstantString& bus_name)
    {
      size_t hash = TransportSession::hash(source, destination, bus_name);
      auto iter = _session_queue.find_if([hash](const HashPair& pair)->bool
      {
        return pair.first == hash;
      });

      if (iter == _session_queue.end())
        return TransportSessionPtr();

      return iter->second.front();
    }
    
    SessionMap                    _session_queue;
  }                               _session_stack[eNumDirections];

  RecursiveMutex                 _mutex;
};

} // can
} // brt

