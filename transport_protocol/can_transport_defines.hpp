/**
 * can_transport_defines.hpp
 * 
 */

#pragma once

// ISO 11783-3:2018 5.13.3:
// The required time interval between packets of a multi-packet broadcast message is 10 ms to 200 ms
#define BAM_TP_MINIMUM_TIMEOUT              (10) // ms

// ISO 11783-3:2018 timeouts
#define TRANSPORT_TIMEOUT_Tr                (200) // ms
#define TRANSPORT_TIMEOUT_Th                (500) // ms
#define TRANSPORT_TIMEOUT_T1                (750) // ms
#define TRANSPORT_TIMEOUT_T2                (1250) // ms
#define TRANSPORT_TIMEOUT_T3                (1250) // ms
#define TRANSPORT_TIMEOUT_T4                (1050) // ms

#define MAX_TP_PACKETS                      (255)
#define MAX_TP_DATA_SIZE                    (MAX_TP_PACKETS * 7)
#define MAX_CTS_ATTEMPTS                    (2)

namespace brt {
namespace can {

/**
 * \enum TPControlByte
 *
 */
enum TPControlByte
{
  RTS = 16,
  CTS = 17,
  EOM = 19, // End of message ACK
  Abort = 255,
  BAM = 32
};

/**
 * \enum TPAbortReason
 *
 */
enum TPAbortReason
{
  AbortDuplicateConnection = 1, // Already in one or more connection-managed sessions and cannot support another
  AbortScarseResources = 2,     // System resources were needed for another task so this connection managed session was terminated
  AbortTimeout = 3,             // A timeout occurred and this is the connection abort to close the session
  AbortCTSWhileSending = 4,     // CTS messages received when data transfer is in progress
  AbortMaxTxRequestLimit = 5,   // Maximum retransmit request limit reached
  AbortUnexpectedDT = 6,        // Unexpected data transfer packet
  AbortBadSequenceNumber = 7,   // Bad sequence number (and software is not able to recover)
  AbortDupSequenceNumber = 8,   // Duplicate sequence number (and software is not able to recover)
  AbortSizeToBig = 9,           // “Total message size” is greater than 1785 byte
  AbortUnknown = 250,           // If a Connection Abort reason is identified that is not listed in the table use code 250

  AbortIgnoreMessage
};

} // can
} // brt


