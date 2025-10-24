/*!
 * \file            pal.h
 * \date            2025-09-26
 * \authors         Mario Mazzara [mario.mazzara@eagletrt.it]
 *                  Mirko Lana [mirko.lana@eagletrt.it]
 *
 * \brief           Peripheral Abstraction Layer (PAL) library, suited for all
 *                  embedded devices.
 *
 * \details         This library provides a set of functions and data structures
 *                  to abstract the communication with peripherals in a device.
 *                  It's designed to be platform-independent so a driver must be implemented
 *                  by the user to provide the necessary functionality.
 *
 * \warning         The dynamically allocated memory will not be deallocated automatically but has
 *                  to be freed by using the arena allocator.
 */

#ifndef PAL_H
#define PAL_H

#include "ring-buffer.h"

#include <stddef.h>
#include <stdint.h>

/*!
 * \brief           Enumeration with all possible return codes of the library.
 */
enum PalReturnCode {
    PAL_RC_OK,         /*!< Everything is fine. */
    PAL_RC_NULL_PTR,   /*!< Unexpected NULL pointer. */
    PAL_RC_IO_ERR,     /*!< Generic I/O error. */
    PAL_RC_QUEUE_FULL, /*!< The queue is full. */
    PAL_RC_QUEUE_EMPTY /*!< The queue is empty. */
};

/*!
 * \brief           Type definition for a function pointer used to send data.
 *
 * \param[in]       buff: Pointer to the data to be sent.
 * \param[in]       size: Size of the data buffer in bytes.
 * \return          0 on success, -1 otherwise.
 */
typedef int (*pal_send_fn)(const uint8_t *buff, size_t size);

/*!
 * \brief           Type definition for a function pointer used to serialize data.
 *
 * \param[in]       in: A pointer to the structure to serialize.
 * \param[out]      out: A pointer to the serialized data.
 * \param[in]       size: Size of the buffer in bytes.
 * \return          The number of bytes received on success, -1 otherwise.
 */
typedef int (*pal_serialize_fn)(const void *in, uint8_t *out, size_t size);

/*!
 * \brief           A structure that encapsulate data and functions required to
 *                  handle the communication with the desired peripheral.
 *
 * \attention       This structure should not be used directly.
 */
struct PalHandler {
    pal_send_fn send;             /*!< Function pointer for sending data to the peripheral. */
    RingBufferHandler_t tx_queue; /*!< Ring buffer handler for managing the outgoing data queue. */
    pal_serialize_fn serialize;   /*!< Function pointer for serializing data before sending. */
};

#endif /*! PAL_H */
