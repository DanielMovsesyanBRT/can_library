/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 12/1/2020 16:28:20
 * File : can_transcoder_ecu_id.cpp
 *
 */
    
#include "can_transcoder_ecu_id.hpp"  

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
 * \fn  CanTranscoderEcuId::Decoder::decode
 *
 * @return  shared_pointer<CanTranscoder>
 */
shared_pointer<CanTranscoder> CanTranscoderEcuId::Decoder::decode()
{
  const char* ptr = reinterpret_cast<const char*>(msg()->data());
  const char* end = reinterpret_cast<const char*>(msg()->data() + msg()->length());

  CanTranscoderEcuId* eid = new CanTranscoderEcuId();
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

  return shared_pointer<CanTranscoder>(eid);
}


} // can
} // brt


