/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 12/1/2020 16:28:20
 * File : can_transcoder_ecu_id.hpp
 *
 */

#pragma once

#include "can_transcoder.hpp"

#define MAX_ECUID_CHARACTERS                (200)

namespace brt {
namespace can {

/**
 * \class CanTranscoderEcuId
 *
 * \brief <description goes here>
 */
class CanTranscoderEcuId : public CanTranscoder
{
friend bool can_library_init(const LibraryConfig&);
friend bool can_library_release();

public:
  CanTranscoderEcuId();
  virtual ~CanTranscoderEcuId();

  /**
   * \class Encoder
   *
   * Inherited from :
   *             CanTranscoder :: Encoder 
   */
  class Encoder : public CanTranscoder::Encoder
  {
  public:
    Encoder(const CanMessagePtr& msg) : CanTranscoder::Encoder(msg) {}
    virtual shared_pointer<CanTranscoder> encode();
  };

          const char*             ecu_part_number() const { return _ecu_part_number; }
          const char*             ecu_serial_number() const { return _ecu_serial_number; }
          const char*             ecu_location() const { return _ecu_location; }
          const char*             ecu_type() const { return _ecu_type; }
          const char*             ecu_mf_name() const { return _ecu_mf_name; }
          const char*             ecu_hardware_id() const { return _ecu_hardware_id; }
          
          void*                   operator new(size_t size)
          {
            if (_allocator == nullptr)
              throw std::runtime_error("Library is not properly initialized");

            void* ptr = _allocator->allocate();
            if (ptr == nullptr)
              ptr = ::malloc(size);

            return ptr;
          }

          void                    operator delete(void* ptr)
          {
            if (_allocator == nullptr)
              throw std::runtime_error("Library is not properly initialized");

            if (!_allocator->free(ptr))
              ::free(ptr);
          }


private:
  static  allocator<CanTranscoderEcuId>*  _allocator;

  char                            _ecu_part_number[MAX_ECUID_CHARACTERS];
  char                            _ecu_serial_number[MAX_ECUID_CHARACTERS];
  char                            _ecu_location[MAX_ECUID_CHARACTERS];
  char                            _ecu_type[MAX_ECUID_CHARACTERS];
  char                            _ecu_mf_name[MAX_ECUID_CHARACTERS];
  char                            _ecu_hardware_id[MAX_ECUID_CHARACTERS];
};

typedef shared_pointer<CanTranscoderEcuId> CanTranscoderEcuIdPtr;

} // can
} // brt


