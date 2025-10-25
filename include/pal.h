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
    PAL_RC_OK,          /*!< Everything is fine. */
    PAL_RC_NULL_PTR,    /*!< Unexpected NULL pointer. */
    PAL_RC_IO_ERR,      /*!< Generic I/O error. */
    PAL_RC_QUEUE_FULL,  /*!< The queue is full. */
    PAL_RC_QUEUE_EMPTY, /*!< The queue is empty. */
    PAL_RC_DESER_ERR    /*!< Deserialization error. */
};

/*!
 * \brief           A structure representing a message.
 */
struct PalMessage {
    uint8_t *ptr; /*!< Pointer to message data. */
    size_t size;  /*!< The size of the message. */
};

/*!
 * \brief           Type definition for a function pointer used to deserialize data.
 *
 * \param[in]       in: Pointer to the data to be deserialized.
 * \param[out]      out: Pointer to the deserialized data.
 * \return          PAL_RC_OK on success, PAL_RC_DESER_ERR otherwise.
 */
typedef enum PalReturnCode (*pal_deserialize_fn)(const struct PalMessage *in, void *out);

/*!
 * \brief           A structure that encapsulate data and functions required to
 *                  handle the communication with the desired peripheral.
 *
 * \attention       This structure should not be used directly.
 */
struct PalHandler {
    RingBufferHandler_t rx_queue;   /*!< Ring buffer handler for managing received messages queue. */
    pal_deserialize_fn deserialize; /*!< Function pointer for deserialize messages. */
};

#endif /*! PAL_H */
