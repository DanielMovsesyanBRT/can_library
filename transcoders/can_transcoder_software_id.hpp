/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 12/1/2020 11:3:42
 * File : can_transcoder_software_id.hpp
 *
 */

#pragma once

#include "can_transcoder.hpp"

#define MAX_SID_DESIGNATORS                 (250)
#define MAX_SID_MODULES                     (32)

namespace brt {
namespace can {

/**
 * \class CanTranscoderSoftwareId
 *
 * \brief <description goes here>
 */
class CanTranscoderSoftwareId : public CanTranscoder
{
friend bool can_library_init(const LibraryConfig&);
friend bool can_library_release();

public:
  CanTranscoderSoftwareId();
  virtual ~CanTranscoderSoftwareId();

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
    virtual shared_pointer<CanTranscoder> decode();
  };

  struct ControlFunction
  {
    ControlFunction() : _num_modules(0) {}
    struct Module{ char _version[255]; } _modules[MAX_SID_MODULES];
    size_t                          _num_modules;
  };

          size_t                  number_of_cfs() const { return _num_cfs; }
          const ControlFunction*  cf(size_t index) const { return (index < _num_cfs) ? &_sid[index] : nullptr; }

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
  static  allocator<CanTranscoderSoftwareId>*  _allocator;

  ControlFunction                 _sid[MAX_SID_DESIGNATORS];
  size_t                          _num_cfs;
};

typedef shared_pointer<CanTranscoderSoftwareId> CanTranscoderSoftwareIdPtr;

} // can
} // brt


