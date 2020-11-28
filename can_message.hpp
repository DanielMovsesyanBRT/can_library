/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 12:25:5
 * File : can_message.hpp
 *
 */

#pragma once

#include <stdint.h>
#include <string.h>

#include <vector>
#include <memory>
#include <atomic>
#include <algorithm>

#include "can_constants.hpp"
#include "can_utils.hpp"

namespace brt {
namespace can {


class CanPacket
{
public:
  CanPacket() : _id(0), _dlc(0) {}
  /**
   * \fn  constructor CanPacket
   *
   * @param  id : uint32_t 
   * @param  data :  const uint8_t* 
   * @param  dlc :  uint8_t 
   */
  explicit CanPacket(uint32_t id, const uint8_t* data, uint8_t dlc)
  : _id(id)
  , _dlc(std::min(dlc,static_cast<uint8_t>(MAX_CAN_PACKET_SIZE)))
  {
    memcpy(_data, data, _dlc);
    _unique_id = _unique_counter++;
  }

  /**
   * \fn  constructor CanPacket
   *
   * @param  data : const uint8_t* 
   * @param  length :  uint8_t 
   * @param  pgn :  uint32_t 
   * @param  da :  uint8_t 
   * @param  sa :  uint8_t 
   * @param   = DEFAULT_CAN_PRIORITY :  uint8_t priority
   */
  explicit CanPacket(const uint8_t* data, uint8_t length, uint32_t pgn, uint8_t da, uint8_t sa, uint8_t priority = DEFAULT_CAN_PRIORITY)
  : _dlc(std::min(length,static_cast<uint8_t>(MAX_CAN_PACKET_SIZE)))
  {
    memcpy(_data, data, _dlc);
    _unique_id = _unique_counter++;

    _id = ((priority & 7) << 26) | ((pgn & 0x3FFFF) << 8) | sa;
    if (is_pdu1())
    {
      _id &= ~0xFF00;
      _id |= (da << 8);
    }
  }

  /**
   * \fn  constructor CanPacket
   *
   * @param  data : const uint8_t* 
   * @param  length :  uint8_t 
   * @param  pgn :  uint32_t 
   * @param  da :  uint8_t 
   * @param  sa :  uint8_t 
   * @param   = DEFAULT_CAN_PRIORITY :  uint8_t priority
   */
  explicit CanPacket(const std::initializer_list<uint8_t>& data, uint32_t pgn, uint8_t da, uint8_t sa, uint8_t priority = DEFAULT_CAN_PRIORITY)
  : _dlc(std::min(static_cast<uint8_t>(data.size()),static_cast<uint8_t>(MAX_CAN_PACKET_SIZE)))
  {
    memcpy(_data, data.begin(), _dlc);
    _unique_id = _unique_counter++;

    _id = ((priority & 7) << 26) | ((pgn & 0x3FFFF) << 8) | sa;
    if (is_pdu1())
    {
      _id &= ~0xFF00;
      _id |= (da << 8);
    }
  }

            uint32_t              id() const { return _id; }
            uint8_t               dlc() const { return _dlc; }
            const uint8_t*        data() const { return _data; }
            uint64_t              unique_id() const { return _unique_id; }

            uint8_t               pf() const { return static_cast<uint8_t>((_id >> 16) & 0xFF); }
            bool                  is_pdu1() const { return (pf() < 240); }
            bool                  is_pdu2() const { return (pf() >= 240); }

            uint32_t              pgn() const { return is_pdu1() ? ((_id >> 8) & 0x3FF00): ((_id >> 8) & 0x3FFFF); }
            uint8_t               sa() const { return static_cast<uint8_t>(_id & 0xFF); }
            uint8_t               da() const { return static_cast<uint8_t>(is_pdu1() ? ((_id >> 8) & 0xFF): BROADCATS_CAN_ADDRESS); }
            uint8_t               priority() const { return static_cast<uint8_t>((_id >> 26) & 7); }
            bool                  is_broadcast() const { return (da() == BROADCATS_CAN_ADDRESS); }
private:
  uint32_t                        _id;
  uint8_t                         _dlc;
  uint8_t                         _data[MAX_CAN_PACKET_SIZE];

  uint64_t                        _unique_id;
  static std::atomic_uint64_t     _unique_counter;
};


class CanMessagePtr;
/**
 * \class CanMessage
 *
 */
class CanMessage : public shared_class<CanMessage>
{
friend CanMessagePtr;
public:
  typedef std::function<void(uint64_t,const std::string&,bool)>   ConfirmationCallback;

private:
  explicit CanMessage(const uint8_t* data, uint32_t length, uint32_t pgn
            , uint8_t priority = DEFAULT_CAN_PRIORITY,ConfirmationCallback cback = ConfirmationCallback())
  : _pgn(pgn)
  , _priority(priority)
  , _unique_id(_unique_counter++)
  , _cback(cback)
  , _size(length)
  {  
    if (data != nullptr)
      memcpy(_data, data, _size);
  }

public:
  ~CanMessage() {}

          uint32_t                pgn() const { return _pgn; }
          uint8_t                 priority() const { return _priority; }
          uint8_t                 dp() const { return static_cast<uint8_t>((_pgn >> 16) & 1); } 
          uint8_t                 edp() const { return static_cast<uint8_t>((_pgn >> 17) & 1); } 
          uint8_t                 pf() const { return static_cast<uint8_t>((_pgn >> 8) & 0xFF); }

          bool                    is_pdu1() const { return (pf() < 240); }
          bool                    is_pdu2() const { return (pf() >= 240); }
  
          const uint8_t*          data() const { return _data; }
          uint8_t*                data() { return _data; }

          uint32_t                length() const { return static_cast<uint32_t>(_size); }

          uint64_t                unique_id() const { return _unique_id; }
  
          ConfirmationCallback    cback() const { return _cback; }
          void                    callback(const std::string& bus_name, bool succsess)
          {
            if (_cback)
              _cback(_unique_id, bus_name, succsess);
          }

          void operator delete(void*);

private:
  uint32_t                        _pgn;
  uint8_t                         _priority;
  ConfirmationCallback            _cback;

  uint64_t                        _unique_id;
  static std::atomic_uint64_t     _unique_counter;

  uint32_t                        _size;
  uint8_t                         _data[0];
};

/**
 * \class CanMessagePtr
 *
 * Inherited from :
 *             std :: shared_ptr<CanMessage>
 */
class CanMessagePtr : public shared_pointer<CanMessage>
{
friend class CanMessage;
public:
  CanMessagePtr() {}

  explicit CanMessagePtr(const std::initializer_list<uint8_t>& data, uint32_t pgn, uint8_t priority = DEFAULT_CAN_PRIORITY,
                        CanMessage::ConfirmationCallback cback = CanMessage::ConfirmationCallback())
  { 
    CanMessage* msg = nullptr;
    if (data.size() <= 8)
      msg = reinterpret_cast<CanMessage*>(_small_packet_allocator.allocate());
    else if (data.size() <= (255*7))
      msg = reinterpret_cast<CanMessage*>(_big_packet_allocator.allocate());

    if (msg != nullptr)
    {
      ::new (msg) CanMessage(data.begin(), data.size(), pgn, priority, cback);
      reset(msg);
    }
  }

  explicit CanMessagePtr(const uint8_t* data,uint32_t length, uint32_t pgn, uint8_t priority = DEFAULT_CAN_PRIORITY,
                        CanMessage::ConfirmationCallback cback = CanMessage::ConfirmationCallback())
  { 
    CanMessage* msg = nullptr;
    if (length <= 8)
      msg = reinterpret_cast<CanMessage*>(_small_packet_allocator.allocate());
    else if (length <= (255*7))
      msg = reinterpret_cast<CanMessage*>(_big_packet_allocator.allocate());

    if (msg != nullptr)
    {
      ::new (msg) CanMessage(data, length, pgn, priority, cback);
      reset(msg);
    }
  }

  explicit CanMessagePtr(uint32_t length, uint32_t pgn, uint8_t priority = DEFAULT_CAN_PRIORITY,
                        CanMessage::ConfirmationCallback cback = CanMessage::ConfirmationCallback())
  { 
    CanMessage* msg = nullptr;
    if (length <= 8)
      msg = reinterpret_cast<CanMessage*>(_small_packet_allocator.allocate());
    else if (length <= (255*7))
      msg = reinterpret_cast<CanMessage*>(_big_packet_allocator.allocate());

    if (msg != nullptr)
    {
      ::new (msg) CanMessage(nullptr, length, pgn, priority, cback);
      reset(msg);
    }
  }



private: 
  typedef allocator<CanMessage,255*7> big_packet_allocator;
  typedef allocator<CanMessage,8>     small_packet_allocator;
  
  static  big_packet_allocator    _big_packet_allocator;
  static  small_packet_allocator  _small_packet_allocator;
};

} // can
} // brt

