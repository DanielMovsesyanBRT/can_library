/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 12/2/2020 13:49:0
 * File : can_transcoder_diag_prot.cpp
 *
 */
    
#include "can_transcoder_diag_prot.hpp"  

namespace brt {
namespace can {

allocator<CanTranscoderDiagProt>* CanTranscoderDiagProt::_allocator = nullptr;
/**
 * \fn  constructor CanTranscoderDiagProt::CanTranscoderDiagProt
 *
 */
CanTranscoderDiagProt::CanTranscoderDiagProt()
: _supported_diagnostics(0)
{

}

/**
 * \fn  destructor CanTranscoderDiagProt::~CanTranscoderDiagProt
 *
 * \brief <description goes here>
 */
CanTranscoderDiagProt::~CanTranscoderDiagProt()
{

}

} // can
} // brt


