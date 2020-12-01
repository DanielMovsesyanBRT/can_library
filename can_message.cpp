/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 12:25:5
 * File : can_message.cpp
 *
 */
    
#include "can_processor.hpp"  
#include "can_message.hpp"  

namespace brt {
namespace can {

std::atomic_uint64_t CanPacket::_unique_counter(0LLU);
std::atomic_uint64_t CanMessage::_unique_counter(0LLU);

/**
 * \fn  delete
 *
 * @param  ptr : void* 
 * @return  void CanMessage::operator
 */
void CanMessage::operator delete(void* ptr)
{
  if (!allocator<CanMessage,8>::free(ptr) && 
      !allocator<CanMessage,255*7>::free(ptr))
  {
    ::free(ptr);
  }
}


/**
 * \fn  constructor CanMessagePtr::CanMessagePtr
 *
 * @param  iface : CanInterface* 
 * @param  data :  const std::initializer_list<uint8_t>& 
 * @param  pgn :  uint32_t 
 * @param  priority  :  uint8_t 
 * @param   cback :  CanMessage::ConfirmationCallback
 */
CanMessagePtr::CanMessagePtr(CanInterface* iface, const std::initializer_list<uint8_t>& data, uint32_t pgn, 
                        uint8_t priority /*= DEFAULT_CAN_PRIORITY*/,
                        CanMessage::ConfirmationCallback cback /*= CanMessage::ConfirmationCallback()*/)
{ 
  CanProcessor* processor = dynamic_cast<CanProcessor*>(iface);
  if (processor == nullptr)
    throw std::runtime_error("Interface pointer is missing or wrong");

  CanMessage* msg = nullptr;
  if (data.size() <= 8)
  {
    msg = reinterpret_cast<CanMessage*>(processor->cfg().small_msg_allocator().allocate());
    if (msg == nullptr)
      msg = reinterpret_cast<CanMessage*>(::malloc(sizeof(CanMessage) + 8));
  }
  else if (data.size() <= (255*7))
  {
    msg = reinterpret_cast<CanMessage*>(processor->cfg().big_msg_allocator().allocate());
    if (msg == nullptr)
      msg = reinterpret_cast<CanMessage*>(::malloc(sizeof(CanMessage) + (255*7)));
  }

  if (msg == nullptr)
    throw std::bad_alloc();

  ::new (msg) CanMessage(data.begin(), data.size(), pgn, priority, cback);
  reset(msg);
}

/**
 * \fn  constructor CanMessagePtr::CanMessagePtr
 *
 * @param  iface : CanInterface* 
 * @param  data :  const uint8_t* 
 * @param  length : uint32_t 
 * @param  pgn :  uint32_t 
 * @param  priority :  uint8_t 
 * @param   cback :  CanMessage::ConfirmationCallback
 */
CanMessagePtr::CanMessagePtr(CanInterface* iface, const uint8_t* data,uint32_t length, uint32_t pgn, 
                        uint8_t priority/* = DEFAULT_CAN_PRIORITY*/,
                        CanMessage::ConfirmationCallback cback /*= CanMessage::ConfirmationCallback()*/)
{ 
  CanProcessor* processor = dynamic_cast<CanProcessor*>(iface);
  if (processor == nullptr)
    throw std::runtime_error("Interface pointer is missing or wrong");

  CanMessage* msg = nullptr;
  if (length <= 8)
  {
    msg = reinterpret_cast<CanMessage*>(processor->cfg().small_msg_allocator().allocate());
    if (msg == nullptr)
      msg = reinterpret_cast<CanMessage*>(::malloc(sizeof(CanMessage) + 8));
  }
  else if (length <= (255*7))
  {
    msg = reinterpret_cast<CanMessage*>(processor->cfg().big_msg_allocator().allocate());
    if (msg == nullptr)
      msg = reinterpret_cast<CanMessage*>(::malloc(sizeof(CanMessage) + (255*7)));
  }

  if (msg == nullptr)
    throw std::bad_alloc();

  ::new (msg) CanMessage(data, length, pgn, priority, cback);
  reset(msg);
}

/**
 * \fn  constructor CanMessagePtr::CanMessagePtr
 *
 * @param  iface : CanInterface* 
 * @param  length :  uint32_t 
 * @param  pgn :  uint32_t 
 * @param  priority :  uint8_t 
 * @param   cback :  CanMessage::ConfirmationCallback
 */
CanMessagePtr::CanMessagePtr(CanInterface* iface, uint32_t length, uint32_t pgn, 
                        uint8_t priority/* = DEFAULT_CAN_PRIORITY*/,
                        CanMessage::ConfirmationCallback cback /*= CanMessage::ConfirmationCallback()*/)
{ 
  CanProcessor* processor = dynamic_cast<CanProcessor*>(iface);
  if (processor == nullptr)
    throw std::runtime_error("Interface pointer is missing or wrong");

  CanMessage* msg = nullptr;
  if (length <= 8)
  {
    msg = reinterpret_cast<CanMessage*>(processor->cfg().small_msg_allocator().allocate());
    if (msg == nullptr)
      msg = reinterpret_cast<CanMessage*>(::malloc(sizeof(CanMessage) + 8));
  }
  else if (length <= (255*7))
  {
    msg = reinterpret_cast<CanMessage*>(processor->cfg().big_msg_allocator().allocate());
    if (msg == nullptr)
      msg = reinterpret_cast<CanMessage*>(::malloc(sizeof(CanMessage) + (255*7)));
  }

  if (msg == nullptr)
    throw std::bad_alloc();

  ::new (msg) CanMessage(nullptr, length, pgn, priority, cback);
  reset(msg);
}


} // can
} // brt

