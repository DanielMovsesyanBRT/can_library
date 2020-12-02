/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 12/1/2020 10:56:6
 * File : can_transcoder.hpp
 *
 */

#pragma once

#include "../can_message.hpp"
#include "../can_utils.hpp"

namespace brt {
namespace can {

/**
 * \class CanTranscoder
 *
 * \brief <description goes here>
 */
class CanTranscoder : public shared_class<CanTranscoder>
{
public:
  CanTranscoder();
  virtual ~CanTranscoder();

  /**
   * \class Decoder
   *
   */
  class Decoder
  {
  public:
    Decoder(const CanMessagePtr& msg) : _msg(msg) {}
    virtual ~Decoder() {}

    virtual shared_pointer<CanTranscoder> decode() = 0;
    const CanMessagePtr&            msg() const { return _msg; }
  private:
    CanMessagePtr                   _msg;
  };
};


} // can
} // brt


