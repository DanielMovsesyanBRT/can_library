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

allocator<CanMessage,255*7>* CanMessage::_big_msg_allocator = nullptr;
allocator<CanMessage,8>* CanMessage::_small_msg_allocator = nullptr;


/**
 * \fn  delete
 *
 * @param  ptr : void* 
 * @return  void CanMessage::operator
 */
void CanMessage::operator delete(void* ptr)
{
  if ((_big_msg_allocator == nullptr) || (_small_msg_allocator == nullptr))
    throw std::runtime_error("Library is not properly initialized");

  if (!_small_msg_allocator->free(ptr) && 
      !_big_msg_allocator->free(ptr))
  {
    ::free(ptr);
  }
}


/**
 * \fn  constructor CanMessagePtr::CanMessagePtr
 *
 * @param  data :  const std::initializer_list<uint8_t>& 
 * @param  pgn :  uint32_t 
 * @param  priority  :  uint8_t 
 * @param   cback :  CanMessage::ConfirmationCallback
 */
CanMessagePtr::CanMessagePtr(const std::initializer_list<uint8_t>& data, uint32_t pgn, 
                        uint8_t priority /*= DEFAULT_CAN_PRIORITY*/,
                        CanMessage::ConfirmationCallback cback /*= CanMessage::ConfirmationCallback()*/)
{ 
  if ((CanMessage::_big_msg_allocator == nullptr) || (CanMessage::_small_msg_allocator == nullptr))
    throw std::runtime_error("Library is not properly initialized");

  CanMessage* msg = nullptr;
  if (data.size() <= 8)
  {
    msg = reinterpret_cast<CanMessage*>(CanMessage::_small_msg_allocator->allocate());
    if (msg == nullptr)
      msg = reinterpret_cast<CanMessage*>(::malloc(sizeof(CanMessage) + 8));
  }
  else if (data.size() <= (255*7))
  {
    msg = reinterpret_cast<CanMessage*>(CanMessage::_big_msg_allocator->allocate());
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
 * @param  data :  const uint8_t* 
 * @param  length : uint32_t 
 * @param  pgn :  uint32_t 
 * @param  priority :  uint8_t 
 * @param   cback :  CanMessage::ConfirmationCallback
 */
CanMessagePtr::CanMessagePtr(const uint8_t* data,uint32_t length, uint32_t pgn, 
                        uint8_t priority/* = DEFAULT_CAN_PRIORITY*/,
                        CanMessage::ConfirmationCallback cback /*= CanMessage::ConfirmationCallback()*/)
{ 
  if ((CanMessage::_big_msg_allocator == nullptr) || (CanMessage::_small_msg_allocator == nullptr))
    throw std::runtime_error("Library is not properly initialized");

  CanMessage* msg = nullptr;
  if (length <= 8)
  {
    msg = reinterpret_cast<CanMessage*>(CanMessage::_small_msg_allocator->allocate());
    if (msg == nullptr)
      msg = reinterpret_cast<CanMessage*>(::malloc(sizeof(CanMessage) + 8));
  }
  else if (length <= (255*7))
  {
    msg = reinterpret_cast<CanMessage*>(CanMessage::_big_msg_allocator->allocate());
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
 * @param  length :  uint32_t 
 * @param  pgn :  uint32_t 
 * @param  priority :  uint8_t 
 * @param   cback :  CanMessage::ConfirmationCallback
 */
CanMessagePtr::CanMessagePtr(uint32_t length, uint32_t pgn, 
                        uint8_t priority/* = DEFAULT_CAN_PRIORITY*/,
                        CanMessage::ConfirmationCallback cback /*= CanMessage::ConfirmationCallback()*/)
{ 
  if ((CanMessage::_big_msg_allocator == nullptr) || (CanMessage::_small_msg_allocator == nullptr))
    throw std::runtime_error("Library is not properly initialized");

  CanMessage* msg = nullptr;
  if (length <= 8)
  {
    msg = reinterpret_cast<CanMessage*>(CanMessage::_small_msg_allocator->allocate());
    if (msg == nullptr)
      msg = reinterpret_cast<CanMessage*>(::malloc(sizeof(CanMessage) + 8));
  }
  else if (length <= (255*7))
  {
    msg = reinterpret_cast<CanMessage*>(CanMessage::_big_msg_allocator->allocate());
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

