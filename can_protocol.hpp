/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/23/2020 11:9:41
 * File : can_transport.hpp
 *
 */

#pragma once

#include "can_message.hpp"
#include "local_ecu.hpp"
#include "remote_ecu.hpp"

#include <memory>

namespace brt {
namespace can {

class CanProcessor;
/**
 * \class CanProtocol
 *
 * Inherited from :
 *             shared_class<CanProtocol>
 */
class CanProtocol : public shared_class<CanProtocol>
{
public:
  CanProtocol(CanProcessor* processor);
  virtual ~CanProtocol();

  virtual bool                    send_message(const CanMessagePtr& message,const LocalECUPtr& local,
                                                          const RemoteECUPtr& remote, const ConstantString& buses) = 0;

          CanProcessor*           processor() { return _processor; }

private:
  CanProcessor*                   _processor;
};

typedef shared_pointer<CanProtocol>   CanProtocolPtr;

} // can
} // brt

