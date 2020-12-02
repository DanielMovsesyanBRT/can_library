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
 */
class CanTranscoderEcuId : public CanTranscoder
{
friend bool can_library_init(const LibraryConfig&);
friend bool can_library_release();

  CanTranscoderEcuId();
public:
  virtual ~CanTranscoderEcuId();

  virtual CanMessagePtr           encode() const;
  virtual uint32_t                pgn() const { return PGN_ECUID; }

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


  /**
   * \class Decoder
   *
   * Inherited from :
   *             CanTranscoder :: Decoder 
   */
  class Decoder : public CanTranscoder::Decoder
  {
  public:
    Decoder(const CanMessagePtr& msg) : CanTranscoder::Decoder(msg) {}

  protected:
    virtual CanTranscoder*          create() { return new CanTranscoderEcuId(); }
    virtual void                    on_decode(CanTranscoder*);
  };

  /**
   * \class Builder
   *
   */
  class Builder 
  {
  public:
    Builder();
    ~Builder() {}

            Builder&                set_part_number(const std::string&);
            Builder&                set_serial_number(const std::string&);
            Builder&                set_ecu_location(const std::string&);
            Builder&                set_ecu_type(const std::string&);
            Builder&                set_ecu_mf_name(const std::string&);
            Builder&                set_ecu_hardware_id(const std::string&);

            shared_pointer<CanTranscoderEcuId> build() { return _eid; }        

  private:
    shared_pointer<CanTranscoderEcuId> _eid;
  };

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


