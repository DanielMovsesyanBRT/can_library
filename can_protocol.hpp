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
 *             std :: enable_shared_from_this <CanProtocol>
 */
class CanProtocol : std::enable_shared_from_this<CanProtocol>
{
public:
  CanProtocol(CanProcessor* processor);
  virtual ~CanProtocol();

  virtual bool                    send_message(const CanMessagePtr& message,const LocalECUPtr& local,
                                                          const RemoteECUPtr& remote, const std::string& buses) = 0;        

          std::shared_ptr<CanProtocol> getptr() { return shared_from_this(); }
          CanProcessor*           processor() { return _processor; }

private:
  CanProcessor*                   _processor;
};

typedef std::shared_ptr<CanProtocol>   CanProtocolPtr;

} // can
} // brt

