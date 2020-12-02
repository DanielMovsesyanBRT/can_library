/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 12/1/2020 11:3:42
 * File : can_transcoder_software_id.cpp
 *
 */
    
#include "can_transcoder_software_id.hpp"  
#include "can_transport_defines.hpp"

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
 * \fn  CanTranscoderSoftwareId::encode
 *
 * @return  CanMessagePtr
 */
CanMessagePtr CanTranscoderSoftwareId::encode() const
{
  if (_num_cfs == 0)
    return CanMessagePtr();

  uint8_t  data_bytes[MAX_TP_DATA_SIZE];
  uint8_t *ptr = data_bytes, *end = ptr + MAX_TP_DATA_SIZE;
  
  *ptr++ = static_cast<uint8_t>(_num_cfs);
  for (size_t index = 0; ((index < _num_cfs) && (ptr < end)); index++)
  {
    for (size_t module_id = 0; ((module_id < _sid[index]._num_modules) && (ptr < end)); module_id++)
    {
      const char *value = _sid[index]._modules[module_id]._version;
      while (*value != '\0' && (ptr < end))
        *ptr++ = static_cast<uint8_t>(*value++);

      if (ptr < end)
        *ptr++ = '#';
    }

    if (ptr < end)
      *ptr++ = '*';
  }

  return CanMessagePtr(data_bytes,ptr - data_bytes,PGN_SoftwareID);
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

/**
 * \fn  constructor CanTranscoderSoftwareId::Builder::Builder
 *
 */
CanTranscoderSoftwareId::Builder::Builder() 
{
  _sid.reset(new CanTranscoderSoftwareId());
}

/**
 * \fn  CanTranscoderSoftwareId::Builder::add_module_id
 *
 * @param  & module_id : const std::string
 * @return  Builder
 */
CanTranscoderSoftwareId::Builder& CanTranscoderSoftwareId::Builder::add_module_id(const std::string& module_id)
{
  if (_sid->_num_cfs == 0)
    _sid->_num_cfs++;

  if (_sid->_sid[_sid->_num_cfs]._num_modules < MAX_SID_MODULES)
    strncpy(_sid->_sid[_sid->_num_cfs - 1]._modules[_sid->_sid[_sid->_num_cfs - 1]._num_modules++]._version,
                         module_id.c_str(), 255);

  return *this;
}

/**
 * \fn  CanTranscoderSoftwareId::Builder::add_software_id
 *
 * @return  Builder
 */
CanTranscoderSoftwareId::Builder& CanTranscoderSoftwareId::Builder::add_software_id()
{
  if (_sid->_num_cfs < MAX_SID_DESIGNATORS)
    _sid->_num_cfs++;

  return *this;
}

} // can
} // brt

