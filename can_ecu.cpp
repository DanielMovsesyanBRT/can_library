/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 13:33:44
 * File : can_ecu.cpp
 *
 */
    
#include "can_ecu.hpp"  
#include "can_processor.hpp"

namespace brt {
namespace can {

/**
 * \fn  constructor CanECU::CanECU
 *
 * @param  processor : CanProcessor* 
 * @param  name : const CanName& 
 */
CanECU::CanECU(CanProcessor* processor,const CanName& name /*= CanName()*/)
: _processor(processor)
, _name(name)
{

}

/**
 * \fn  destructor CanECU::~CanECU
 *
 */
CanECU::~CanECU()
{

}

/**
 * \fn  CanECU::get_address
 *
 * @param  & bus_name : const std::string
 * @return  uint8_t
 */
uint8_t CanECU::get_address(const std::string& bus_name) const
{
  return _processor->device_db().get_ecu_address(name(), bus_name);
}

/**
 * \fn  CanECU::set_pgn_transcoder
 *
 * @param  tcoder : const CanTranscoderPtr& 
 * @return  bool
 */
bool CanECU::set_pgn_transcoder(const CanTranscoderPtr& tcoder)
{
  auto iter = _trans_map.find_if([tcoder](const CanTranscoderPtr& __t)->bool
  {
    return (__t && (__t->pgn() == tcoder->pgn()));
  });

  if (iter != _trans_map.end())
    *iter = tcoder;
  else if (_trans_map.push(tcoder) == _trans_map.end())
    return false;

  return true;
}

/**
 * \fn  CanECU::get_pgn_transcoder
 *
 * @param  pgn : uint32_t 
 * @return  CanTranscoderPtr
 */
CanTranscoderPtr CanECU::get_pgn_transcoder(uint32_t pgn) const
{
  auto iter = _trans_map.find_if([pgn](const CanTranscoderPtr& __t)->bool
  {
    return (__t && (__t->pgn() == pgn));
  });

  if (iter == _trans_map.end())
    return CanTranscoderPtr();
  
  return (*iter);
}


} // can
} // brt
