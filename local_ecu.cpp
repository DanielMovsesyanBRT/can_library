/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 15:0:44
 * File : local_ecu.cpp
 *
 */
    
#include "local_ecu.hpp"  
#include "can_processor.hpp"  

namespace brt {
namespace can {

/**
 * \fn  constructor LocalECU::LocalECU
 *
 * @param  processor : CanProcessor* 
 * @param  name : const CanName& 
 */
LocalECU::LocalECU(CanProcessor* processor,const CanName& name /*= CanName()*/)
: CanECU(processor,name)
{

}

/**
 * \fn  destructor LocalECU::~LocalECU
 *
 */
LocalECU::~LocalECU()
{

}

/**
 * \fn  LocalECU::claim_address
 *
 * @param  address : uint8_t 
 * @param  & bus : const std::string
 */
void LocalECU::claim_address(uint8_t address,const std::string& bus)
{
  auto stat = _bus_status.find(bus);
  if (stat == _bus_status.end())
  {
    auto pair = _bus_status.insert(StatusMap::value_type(bus, BusStatus()));
    if (!pair.second)
      return; // Assert
    
    stat = pair.first;
  }

  CanMessagePtr msg(name().data(), sizeof(uint64_t), PGN_AddressClaimed, BROADCATS_CAN_ADDRESS, address);
  if (address == NULL_CAN_ADDRESS)
  {
    // Claiming NULL address means we are sending Cannot Claim Address message,
    // which automatically deactivates the device
    disable_device(bus);
  }
  else
  {
    // With Normal address claimed sent we should put the device into waiting state for
    // 250 ms according to ISO 11783-5 requirements
    stat->second._status = eWaiting;
    stat->second._time_tag = processor()->get_time_tick();
    
    processor()->register_updater([&]()->bool
    {
      if (stat->second._status != eWaiting)
        return true; // remove from updaters

      uint64_t cur_time = processor()->get_time_tick();
      if ((cur_time - stat->second._time_tag) >= CAN_ADDRESS_CLAIMED_WAITING_TIME)
      {
        stat->second._status = eActive;
        while (!stat->second._fifo.empty())
        {
          Queue& queue = stat->second._fifo.front();
          send_message(queue._message, queue._remote);
          stat->second._fifo.pop_front();
        }

        return true;
      }
      return false;
    });
  }

  processor()->send_raw_message(msg,bus);
}

/**
 * \fn  LocalECU::send_message
 *
 * @param  message : CanMessagePtr 
 * @param  remote : RemoteECUPtr 
 */
bool LocalECU::send_message(CanMessagePtr message,RemoteECUPtr remote)
{
  std::unordered_map<std::string,uint8_t> sa_map = get_addresses();

  if (!remote)
  {
    // broadcast
    for (auto stat : _bus_status)
    {
      auto sa = sa_map.find(stat.first);
      if (sa == sa_map.end())
        continue;

      if (stat.second._status == eInavtive)
        continue;

      if (stat.second._status == eWaiting)
      {
        stat.second._fifo.push_back(Queue(message, remote));
        continue;
      }

      if (message->length() <= 8)
      {
        CanMessagePtr msg(message->data(),message->length(),message->pgn(),BROADCATS_CAN_ADDRESS,sa->second,message->priority());
        processor()->send_raw_message(msg, stat.first);
      }
    }
  }
  else
  {
    std::vector<std::string> buses = processor()->device_db().get_ecu_bus(remote->name());
    if (buses.empty())
      return false;

    auto stat = _bus_status.find(buses[0]);
    if (stat == _bus_status.end())
      return false;

    auto sa = sa_map.find(buses[0]);
    if (sa == sa_map.end())
      return false;

    if (stat->second._status == eInavtive)
      return false;

    if (stat->second._status == eWaiting)
    {
      stat->second._fifo.push_back(Queue(message, remote));
      return true;
    }

    if (message->length() <= 8)
    {
      CanMessagePtr msg(message->data(),message->pgn(),remote->get_address(),sa->second,message->priority());
      if (!processor()->send_raw_message(msg, buses[0]))
        return false;
    }
  }

  return true;
}

/**
 * \fn  LocalECU::disable_device
 *
 * @param  & bus : const std::string
 */
void LocalECU::disable_device(const std::string& bus)
{
  auto stat = _bus_status.find(bus);
  if (stat != _bus_status.end())
    stat->second._status = eInavtive;
}

} // can
} // brt
