/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 12/1/2020 10:56:6
 * File : can_transcoder.cpp
 *
 */
    
#include "can_transcoder.hpp"  
#include "can_transcoder_ack.hpp"  

namespace brt {
namespace can {

/**
 * \fn  constructor CanTranscoder::CanTranscoder
 *
 * \brief <description goes here>
 */
CanTranscoder::CanTranscoder()
: _not_supported(false)
{

}

/**
 * \fn  destructor CanTranscoder::~CanTranscoder
 *
 * \brief <description goes here>
 */
CanTranscoder::~CanTranscoder()
{

}

/**
 * \fn  CanTranscoder::Decoder::decode
 *
 * @return  shared_pointer<CanTranscoder
 */
shared_pointer<CanTranscoder> CanTranscoder::Decoder::decode()
{
  CanTranscoder* transcoder = create();
  if (transcoder == nullptr)
    return shared_pointer<CanTranscoder>();

  if (_msg->pgn() == PGN_AckNack)
  {
    CanTranscoderAck ack(_msg);
    if (ack.pgn() != transcoder->pgn())
    {
      delete transcoder;
      return shared_pointer<CanTranscoder>();
    }

    if (ack.value() == CanTranscoderAck::Nack)
      transcoder->_not_supported = true;

    // todo: other ack responses....
  }
  else if (_msg->pgn() == transcoder->pgn())
  {
    on_decode(transcoder);
  }
  else
  {
    delete transcoder;
    return shared_pointer<CanTranscoder>();
  }
    
  return shared_pointer<CanTranscoder>(transcoder);
}


} // can
} // brt


