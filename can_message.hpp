/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 12:25:5
 * File : can_message.hpp
 *
 */

#pragma once

#include <stdint.h>

#include <vector>
#include <memory>
#include <atomic>

#include "can_constants.hpp"

namespace brt {
namespace can {


/**
 * \class CanMessage
 *
 */
class CanMessage  
{
public:
  CanMessage();
  explicit CanMessage(uint32_t id, const uint8_t* data, uint32_t length);
  explicit CanMessage(const std::vector<uint8_t> & data, uint32_t pgn, uint8_t da, uint8_t sa, uint8_t priority = DEFAULT_CAN_PRIORITY);

  virtual ~CanMessage();

  uint32_t                        pgn() const { return _pgn; }
  uint8_t                         sa() const { return _sa; }
  uint8_t                         da() const { return (is_pdu2() && (length() > 8))? BROADCATS_CAN_ADDRESS : _da; } 
  uint8_t                         priority() const { return _priority; }
  uint8_t                         dp() const { return static_cast<uint8_t>((_pgn >> 16) & 1); } 
  uint8_t                         edp() const { return static_cast<uint8_t>((_pgn >> 17) & 1); } 
  uint8_t                         pf() const { return static_cast<uint8_t>((_pgn >> 8) & 0xFF); }
  uint8_t                         ps() const { return static_cast<uint8_t>(_pgn & 0xFF); }

  bool                            is_pdu1() const { return (pf() < 240); }
  bool                            is_pdu2() const { return (pf() >= 240); }
  bool                            is_broadcast() const { return (da() == BROADCATS_CAN_ADDRESS); }
  
  const uint8_t*                  data() const { return _data.data(); }
  uint32_t                        length() const { return static_cast<uint32_t>(_data.size()); }
  uint64_t                        id() const { return _packet_id; }

private:
  uint32_t                        _pgn;
  uint8_t                         _sa;
  uint8_t                         _da;
  uint8_t                         _priority;

  std::vector<uint8_t>            _data;
  
  uint64_t                        _packet_id;
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
  explicit CanMessagePtr(uint32_t id, const uint8_t* data, uint32_t length)
  : std::shared_ptr<CanMessage>(new CanMessage(id, data, length))
  { }

  explicit CanMessagePtr(const std::vector<uint8_t>& data, uint32_t pgn, uint8_t da, uint8_t sa, uint8_t priority = DEFAULT_CAN_PRIORITY)
  : std::shared_ptr<CanMessage>(new CanMessage(std::move(data), pgn, da, sa, priority))
  { }

  explicit CanMessagePtr(const uint8_t *data,uint32_t length, uint32_t pgn, uint8_t da, uint8_t sa, uint8_t priority = DEFAULT_CAN_PRIORITY)
  { 
    std::vector<uint8_t> dt(data, data + length);
    reset(new CanMessage(dt, pgn, da, sa, priority));
  }

};

} // can
} // brt

