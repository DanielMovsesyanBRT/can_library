/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 12/2/2020 13:49:0
 * File : can_transcoder_diag_prot.hpp
 *
 */

#pragma once

#include "can_transcoder.hpp"

namespace brt {
namespace can {

/**
 * \class CanTranscoderDiagProt
 *
 */
class CanTranscoderDiagProt : public CanTranscoder
{
friend bool can_library_init(const LibraryConfig&);
friend bool can_library_release();

  CanTranscoderDiagProt();
public:
  
  /**
   * \enum SupportedProtocolBits
   *
   */
  enum SupportedProtocolBits
  {
    spJ1939_73    = 0x01,    // J1939–73
    spISO_14230   = 0x02,   // ISO 14230 (KWP 2000 using ISO 15765-3 transport protocol)
    spISO_14229_3 = 0x04, // ISO 14229-3 (UDS on CAN)
  };

  virtual ~CanTranscoderDiagProt();

  virtual CanMessagePtr           encode() const
  {
    uint8_t data[8] = { _supported_diagnostics, 0 };
    return CanMessagePtr(data,sizeof(data),PGN_DiagnosticProtocol);
  }

  virtual uint32_t                pgn() const { return PGN_DiagnosticProtocol; }

          bool                    is_protocol_supported(uint8_t prot) const 
          { return ((_supported_diagnostics & prot) != 0); }

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
    virtual CanTranscoder*          create() { return new CanTranscoderDiagProt(); }
    virtual void                    on_decode(CanTranscoder *tc)
    {
      CanTranscoderDiagProt* diag = dynamic_cast<CanTranscoderDiagProt*>(tc);
      if (diag == nullptr)
        throw std::runtime_error("Invaliid transcoder casting");

      if (msg()->length() > 0)
        diag->_supported_diagnostics = msg()->data()[0];
    }
  };


  /**
   * \class Builder
   *
   */
  class Builder 
  {
  public:
    Builder() { _diag.reset(new CanTranscoderDiagProt()); }
    ~Builder() {}

              Builder& set_supported_protocol(uint8_t prot) 
              { 
                _diag->_supported_diagnostics |= prot; 
                return *this;
              }
  
              shared_pointer<CanTranscoderDiagProt> build() { return _diag; }

  private:
    shared_pointer<CanTranscoderDiagProt> _diag;
  };

private:
  static  allocator<CanTranscoderDiagProt>*  _allocator;

  uint8_t                         _supported_diagnostics;
};

typedef shared_pointer<CanTranscoderDiagProt> CanTranscoderDiagProtPtr;

} // can
} // brt


