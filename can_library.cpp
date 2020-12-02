/**
 * 
 * can_library.cpp
 * 
 */

#include "can_library.hpp"
#include "can_processor.hpp"  

#include "can_message.hpp"
#include "local_ecu.hpp"
#include "remote_ecu.hpp"
#include "transport_protocol/can_transport_rxsession.hpp"
#include "transport_protocol/can_transport_txsession.hpp"
#include "transcoders/can_transcoder_software_id.hpp"

namespace brt {
namespace can {


/**
 * \fn  constructor CanInterface::CanInterface
 *
 * @param  callback : Callback* 
 */
CanInterface::CanInterface(Callback* callback)
: _cback(callback)
{
  if (_cback == nullptr)
    std::runtime_error("Callback interface is not set");
}


static bool _library_initialized = false;

/**
 * \fn  can_library_init
 *
 * @param  cfg : const LibraryConfig& 
 * @return  bool
 */
bool can_library_init(const LibraryConfig& cfg /*= LibraryConfig()*/)
{
  if (_library_initialized)
    return false;

  if ((CanMessage::_small_msg_allocator != nullptr) ||
      (CanMessage::_big_msg_allocator != nullptr)   ||
      (LocalECU::_allocator != nullptr)   ||
      (RemoteECU::_allocator != nullptr)  ||
      (TxSession::_allocator != nullptr)  ||
      (RxSession::_allocator != nullptr)  || 
      (CanTranscoderSoftwareId::_allocator != nullptr))
    return false;
  
  CanMessage::_small_msg_allocator    = new allocator<CanMessage,8>(cfg._small_messages_pool_size);
  CanMessage::_big_msg_allocator      = new allocator<CanMessage,255*7>(cfg._big_messages_pool_size);
  LocalECU::_allocator                = new allocator<LocalECU>(cfg._local_ecu_pool_size);
  RemoteECU::_allocator               = new allocator<RemoteECU>(cfg._remote_ecu_pool_size);
  TxSession::_allocator               = new allocator<TxSession>(cfg._tx_tpsessions_pool_size);
  RxSession::_allocator               = new allocator<RxSession>(cfg._rx_tpsessions_pool_size);
  CanTranscoderSoftwareId::_allocator = new allocator<CanTranscoderSoftwareId>(cfg._sid_transcoder_pool_size); 
  
  _library_initialized = true;
  return true;
}

/**
 * \fn  can_library_release
 *
 * @return  bool
 */
bool can_library_release()
{
  if (!_library_initialized)
    return false;

  if (CanMessage::_small_msg_allocator != nullptr)
    delete CanMessage::_small_msg_allocator;

  if (CanMessage::_big_msg_allocator != nullptr)
    delete CanMessage::_big_msg_allocator;
    
  if (LocalECU::_allocator != nullptr)
    delete LocalECU::_allocator;

  if (RemoteECU::_allocator != nullptr)
    delete RemoteECU::_allocator;

  if (TxSession::_allocator != nullptr)
    delete TxSession::_allocator;

  if (RxSession::_allocator != nullptr)
    delete RxSession::_allocator;

  if (CanTranscoderSoftwareId::_allocator != nullptr)
    delete CanTranscoderSoftwareId::_allocator;

  CanMessage::_small_msg_allocator    = nullptr;
  CanMessage::_big_msg_allocator      = nullptr;
  LocalECU::_allocator                = nullptr;
  RemoteECU::_allocator               = nullptr;
  TxSession::_allocator               = nullptr;
  RxSession::_allocator               = nullptr;
  CanTranscoderSoftwareId::_allocator = nullptr;

  _library_initialized = false;
  return true;
}

/**
 * \fn  create_can_interface
 *
 * @param   callback : CanInterface::Callback*
 * @param  cfg : const LibraryConfig& 
 * @return  CanInterface*
 */
CanInterface* create_can_interface(CanInterface::Callback* callback)
{
  return new CanProcessor(callback);
}

/**
 * \fn  delete_can_interface
 *
 * @param  interface : CanInterface* 
 */
void delete_can_interface(CanInterface* interface)
{
  if (interface != nullptr)
    delete interface;
}


} // can
} // brt


