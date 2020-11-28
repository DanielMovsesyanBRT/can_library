/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 12:25:5
 * File : can_message.cpp
 *
 */
    
#include "can_message.hpp"  

namespace brt {
namespace can {

std::atomic_uint64_t CanPacket::_unique_counter(0LLU);
std::atomic_uint64_t CanMessage::_unique_counter(0LLU);

CanMessagePtr::big_packet_allocator CanMessagePtr::_big_packet_allocator;
CanMessagePtr::small_packet_allocator CanMessagePtr::_small_packet_allocator;


void CanMessage::operator delete(void* ptr)
{
  if (!CanMessagePtr::_big_packet_allocator.free(ptr) && 
      !CanMessagePtr::_small_packet_allocator.free(ptr))
  {
    std::runtime_error("Invalid packet being freed");
  }
}


} // can
} // brt

