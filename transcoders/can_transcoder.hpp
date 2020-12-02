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

          bool                    is_supported() const { return !_not_supported; }
  virtual uint32_t                pgn() const = 0;
  /**
   * \class Decoder
   *
   */
  class Decoder
  {
  public:
    Decoder(const CanMessagePtr& msg) : _msg(msg) {}
    virtual ~Decoder() {}

            shared_pointer<CanTranscoder> decode();

  protected:
    virtual CanTranscoder*          create() = 0;
    virtual void                    on_decode(CanTranscoder*) = 0;

    const CanMessagePtr&            msg() const { return _msg; }
  private:
    CanMessagePtr                   _msg;
  };

private:
  bool                            _not_supported;  
 
};


} // can
} // brt


