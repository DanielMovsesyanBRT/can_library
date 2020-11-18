/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 
 * File : can_constants.hpp
 *
 */

#pragma once

namespace brt {
namespace can {

#define DEFAULT_CAN_PRIORITY                (6)
#define BROADCATS_CAN_ADDRESS               (255)
#define NULL_CAN_ADDRESS                    (254)

/**
 * \enum PGNs
 *
 */
enum PGNs
{
  PGN_AckNack             = 0xE800, // 59392
  PGN_Request             = 0xEA00, // 59904

  PGN_TP_DT               = 0xEB00, // 60160  - Transport Protocol Data Transfer
  PGN_TP_CM               = 0xEC00, // 60416  - Transport Protocol Connection Management

  PGN_AddressClaimed      = 0xEE00, // 60928
  PGN_ProprietaryA        = 0xEF00, // 61184
  PGN_Timedate            = 0xFEE6, // 65254

  PGN_ProprietaryB_start  = 0xFF00,
  PGN_ProprietaryB_end    = 0xFFFF,

  PGN_ProprietaryA2       = 0x1EF00, // 126720
};

#define PGN_ProprietaryB(x)                 PGN_ProprietaryB_start + ((x) & 0xFF)


// Timeouts
// Maximum time for a message to be in "Wait For COnfirmation" state
#define CAN_MESSAGE_MAX_CONFIRMATION_TIME   (1000) // 1s

// Time to wait after request for address claimed sent
#define CAN_ADDRESS_CLAIMED_WAITING_TIME    (250) 


} // can
} // brt

