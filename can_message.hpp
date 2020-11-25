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
  explicit CanPacket(const std::vector<uint8_t>& data, uint32_t pgn, uint8_t da, uint8_t sa, uint8_t priority = DEFAULT_CAN_PRIORITY)
  : _dlc(std::min(static_cast<uint8_t>(data.size()),static_cast<uint8_t>(MAX_CAN_PACKET_SIZE)))
  {
    memcpy(_data, data.data(), _dlc);
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


/**
 * \class CanMessage
 *
 */
class CanMessage  
{
public:
  typedef std::function<void(uint64_t,const std::string&,bool)>   ConfirmationCallback;
  // CanMessage() : _pgn(0), _priority(0), _unique_id(_unique_counter++) {}
  explicit CanMessage(const std::vector<uint8_t> & data, uint32_t pgn
            , uint8_t priority = DEFAULT_CAN_PRIORITY,ConfirmationCallback cback = ConfirmationCallback())
  : _pgn(pgn)
  , _priority(priority)
  , _data(data)
  , _unique_id(_unique_counter++)
  , _cback(cback)
  {  }

  virtual ~CanMessage() {}

  uint32_t                        pgn() const { return _pgn; }
  uint8_t                         priority() const { return _priority; }
  uint8_t                         dp() const { return static_cast<uint8_t>((_pgn >> 16) & 1); } 
  uint8_t                         edp() const { return static_cast<uint8_t>((_pgn >> 17) & 1); } 
  uint8_t                         pf() const { return static_cast<uint8_t>((_pgn >> 8) & 0xFF); }

  bool                            is_pdu1() const { return (pf() < 240); }
  bool                            is_pdu2() const { return (pf() >= 240); }
  
  const uint8_t*                  data() const { return _data.data(); }
  uint8_t*                        data() { return _data.data(); }

  uint32_t                        length() const { return static_cast<uint32_t>(_data.size()); }

  uint64_t                        unique_id() const { return _unique_id; }
  
  ConfirmationCallback            cback() const { return _cback; }
  void                            callback(const std::string& bus_name, bool succsess)
  {
    if (_cback)
      _cback(_unique_id, bus_name, succsess);
  }

private:
  uint32_t                        _pgn;
  uint8_t                         _priority;
  std::vector<uint8_t>            _data;
  ConfirmationCallback            _cback;

  uint64_t                        _unique_id;
  static std::atomic_uint64_t     _unique_counter;
};

/**
 * \class CanMessagePtr
 *
 * Inherited from :
 *             std :: shared_ptr<CanMessage>
 */
class CanMessagePtr : public std::shared_ptr<CanMessage>
{
public:
  CanMessagePtr() {}

  explicit CanMessagePtr(const std::vector<uint8_t>& data, uint32_t pgn, uint8_t priority = DEFAULT_CAN_PRIORITY,
                        CanMessage::ConfirmationCallback cback = CanMessage::ConfirmationCallback())
   : std::shared_ptr<CanMessage>(new CanMessage(data, pgn, priority, cback))
  { }

  explicit CanMessagePtr(const uint8_t *data,uint32_t length, uint32_t pgn, uint8_t priority = DEFAULT_CAN_PRIORITY,
                        CanMessage::ConfirmationCallback cback = CanMessage::ConfirmationCallback())
  { 
    std::vector<uint8_t> dt(data, data + length);
    reset(new CanMessage(dt, pgn, priority, cback));
  }

};

} // can
} // brt

