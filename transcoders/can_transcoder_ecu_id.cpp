/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 12/1/2020 16:28:20
 * File : can_transcoder_ecu_id.cpp
 *
 */
    
#include "can_transcoder_ecu_id.hpp"  
#include "can_transport_defines.hpp"

namespace brt {
namespace can {

allocator<CanTranscoderEcuId>* CanTranscoderEcuId::_allocator = nullptr;
/**
 * \fn  constructor CanTranscoderEcuId::CanTranscoderEcuId
 *
 * \brief <description goes here>
 */
CanTranscoderEcuId::CanTranscoderEcuId()
{
  _ecu_part_number[0] = '\0';
  _ecu_serial_number[0] = '\0';
  _ecu_location[0] = '\0';
  _ecu_type[0] = '\0';
  _ecu_mf_name[0] = '\0';
  _ecu_hardware_id[0] = '\0';
}

/**
 * \fn  destructor CanTranscoderEcuId::~CanTranscoderEcuId
 *
 * \brief <description goes here>
 */
CanTranscoderEcuId::~CanTranscoderEcuId()
{

}

/**
 * \fn  CanTranscoderEcuId::encode
 *
 * @return  CanMessagePtr
 */
CanMessagePtr CanTranscoderEcuId::encode() const
{
  uint8_t  data_bytes[MAX_TP_DATA_SIZE];
  uint8_t *ptr = data_bytes, *end = ptr + MAX_TP_DATA_SIZE;

  auto fn=[&ptr,end](const char* value)
  {
    while (*value != '\0' && (ptr < end))
      *ptr++ = static_cast<uint8_t>(*value++);
    
    if (ptr < end)
      *ptr++ = '*';
  };

  fn(_ecu_part_number);
  fn(_ecu_serial_number);
  fn(_ecu_location);
  fn(_ecu_type);
  fn(_ecu_mf_name);
  fn(_ecu_hardware_id);

  return CanMessagePtr(data_bytes, ptr - data_bytes, PGN_ECUID);
}

/**
 * \fn  CanTranscoderEcuId::Decoder::on_decode
 *
 * @param  tcoder : CanTranscoder* 
 */
void CanTranscoderEcuId::Decoder::on_decode(CanTranscoder* tcoder)
{
  CanTranscoderEcuId* eid = dynamic_cast<CanTranscoderEcuId*>(tcoder);
  if (eid == nullptr)
    throw std::runtime_error("Invaliid transcoder casting");

  const char* ptr = reinterpret_cast<const char*>(msg()->data());
  const char* end = reinterpret_cast<const char*>(msg()->data() + msg()->length());

  size_t str_index = 0;
  size_t field_index = 0;

  char* field = eid->_ecu_part_number;

  while ((*ptr != '\0') && (ptr < end))
  {
    if (*ptr == '*')
    {
      if (field != nullptr)
        field[str_index] = '\0';
      str_index = 0;

      switch(++field_index)
      {
      case 0:
        field = eid->_ecu_part_number;
        break;

      case 1:
        field = eid->_ecu_serial_number;
        break;

      case 2:
        field = eid->_ecu_location;
        break;

      case 3:
        field = eid->_ecu_type;
        break;

      case 4:
        field = eid->_ecu_mf_name;
        break;

      case 5:
        field = eid->_ecu_hardware_id;
        break;

      default:
        field = nullptr;      
        break;
      }
    }
    else if ((str_index < MAX_ECUID_CHARACTERS) && (field != nullptr))
      field[str_index++] = *ptr;
    
    ptr++;
  }
}


/**
 * \fn  constructor CanTranscoderEcuId::Builder::Builder
 *
 */
CanTranscoderEcuId::Builder::Builder()
{
  _eid.reset(new CanTranscoderEcuId);
}

/**
 * \fn  CanTranscoderEcuId::Builder::set_part_number
 *
 * @param  & pn : const std::string
 * @return  CanTranscoderEcuId::Builder
 */
CanTranscoderEcuId::Builder& CanTranscoderEcuId::Builder::set_part_number(const std::string& pn)
{
  strncpy(_eid->_ecu_part_number, pn.c_str(), sizeof(_eid->_ecu_part_number) - 1);
  return *this;
}

/**
 * \fn  CanTranscoderEcuId::Builder::set_serial_number
 *
 * @param  & sn : const std::string
 * @return  CanTranscoderEcuId::Builder
 */
CanTranscoderEcuId::Builder& CanTranscoderEcuId::Builder::set_serial_number(const std::string& sn)
{
  strncpy(_eid->_ecu_serial_number, sn.c_str(), sizeof(_eid->_ecu_serial_number) - 1);
  return *this;
}

/**
 * \fn  CanTranscoderEcuId::Builder::set_ecu_location
 *
 * @param  & ecu_location : const std::string
 * @return  CanTranscoderEcuId::Builder
 */
CanTranscoderEcuId::Builder& CanTranscoderEcuId::Builder::set_ecu_location(const std::string& ecu_location)
{
  strncpy(_eid->_ecu_location, ecu_location.c_str(), sizeof(_eid->_ecu_location) - 1);
  return *this;
}

/**
 * \fn  CanTranscoderEcuId::Builder::set_ecu_type
 *
 * @param  & ecu_type : const std::string
 * @return  CanTranscoderEcuId::Builder
 */
CanTranscoderEcuId::Builder& CanTranscoderEcuId::Builder::set_ecu_type(const std::string& ecu_type)
{
  strncpy(_eid->_ecu_type, ecu_type.c_str(), sizeof(_eid->_ecu_type) - 1);
  return *this;
}

/**
 * \fn  CanTranscoderEcuId::Builder::set_ecu_mf_name
 *
 * @param  & mf_name : const std::string
 * @return  CanTranscoderEcuId::Builder
 */
CanTranscoderEcuId::Builder& CanTranscoderEcuId::Builder::set_ecu_mf_name(const std::string& mf_name)
{
  strncpy(_eid->_ecu_mf_name, mf_name.c_str(), sizeof(_eid->_ecu_mf_name) - 1);
  return *this;
}

/**
 * \fn  CanTranscoderEcuId::Builder::set_ecu_hardware_id
 *
 * @param  & hw_id : const std::string
 * @return  CanTranscoderEcuId::Builder
 */
CanTranscoderEcuId::Builder& CanTranscoderEcuId::Builder::set_ecu_hardware_id(const std::string& hw_id)
{
  strncpy(_eid->_ecu_hardware_id, hw_id.c_str(), sizeof(_eid->_ecu_hardware_id) - 1);
  return *this;
}


} // can
} // brt


