/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 12/1/2020 11:3:42
 * File : can_transcoder_software_id.cpp
 *
 */
    
#include "can_transcoder_software_id.hpp"  

namespace brt {
namespace can {

allocator<CanTranscoderSoftwareId>* CanTranscoderSoftwareId::_allocator;
/**
 * \fn  constructor CanTranscoderSoftwareId::CanTranscoderSoftwareId
 *
 * \brief <description goes here>
 */
CanTranscoderSoftwareId::CanTranscoderSoftwareId()
: _num_cfs(0)
{

}

/**
 * \fn  destructor CanTranscoderSoftwareId::~CanTranscoderSoftwareId
 *
 * \brief <description goes here>
 */
CanTranscoderSoftwareId::~CanTranscoderSoftwareId()
{

}

/**
 * \fn  CanTranscoderSoftwareId::Decoder::on_decode
 *
 * @param  tcoder : CanTranscoder* 
 */
void CanTranscoderSoftwareId::Decoder::on_decode(CanTranscoder* tcoder)
{
  if (msg()->length() <= 1)
    return;

  CanTranscoderSoftwareId* sid = dynamic_cast<CanTranscoderSoftwareId*>(tcoder);
  if (sid == nullptr)
    throw std::runtime_error("Invaliid transcoder casting");

  sid->_num_cfs = std::min(msg()->data()[0], static_cast<uint8_t>(MAX_SID_DESIGNATORS));
  const char* ptr = reinterpret_cast<const char*>(&msg()->data()[1]);
  const char* end = reinterpret_cast<const char*>(msg()->data() + msg()->length());

  for (size_t index = 0; index < sid->_num_cfs; index++, ptr++)
  {
    if (ptr >= end || *ptr == '\0')
      break;

    size_t str_index = 0;
    sid->_sid[index]._num_modules = 0;

    while ((*ptr != '\0') && (ptr < end) && (*ptr != '*'))
    {
      if (sid->_sid[index]._num_modules < MAX_SID_MODULES)
      {
        if (*ptr == '#')
        {
          sid->_sid[index]._modules[sid->_sid[index]._num_modules]._version[str_index] = '\0';
          sid->_sid[index]._num_modules++;
          str_index = 0;
        }
        else
          sid->_sid[index]._modules[sid->_sid[index]._num_modules]._version[str_index++] = *ptr;
      }

      ptr++;
    }

    if (str_index > 0)
    {
      sid->_sid[index]._modules[sid->_sid[index]._num_modules]._version[str_index] = '\0';
      sid->_sid[index]._num_modules++;
    }
  }
}



} // can
} // brt

