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
    PAL_RC_OK,            /*!< Everything is fine. */
    PAL_RC_INVALID_PARAM, /*!< Invalid parameter data. */
    PAL_RC_NULL_PTR,      /*!< Unexpected NULL pointer. */
    PAL_RC_IO_ERR,        /*!< Generic I/O error. */
    PAL_RC_QUEUE_FULL,    /*!< The queue is full. */
    PAL_RC_QUEUE_EMPTY,   /*!< The queue is empty. */
    PAL_RC_MSG_TOO_BIG,
    PAL_RC_SERIALIZATION_ERR /*!< Serialization error. */
};

/*!
 * \brief           A structure representing a message.
 */
struct PalMessage {
    uint32_t size;  /*!< The size of the message. */
    uint8_t data[]; /*!< Flexible array member holding the payload (valid bytes = size). */
};

/*!
 * \brief           Type definition for a function pointer used to send data.
 *
 * \param[in]       msg: Pointer to the message to be sent.
 * \retval          PAL_RC_OK on success, an error code otherwise:
 * \retval          PAL_RC_NULL_PTR if `msg` is NULL;
 * \retval          PAL_RC_SERIALIZATION_ERR if serialization fails;
 * \retval          PAL_RC_IO_ERR if fails.
 */
typedef enum PalReturnCode (*pal_send_fn)(const struct PalMessage *msg);

/*!
 * \brief           A structure that encapsulate data and functions required to
 *                  handle the communication with the desired peripheral.
 *
 * \attention       This structure should not be used directly.
 */
struct PalHandler {
    uint32_t max_msg_size;        /*!< Maximum size a message can have */
    pal_send_fn send;             /*!< Function pointer for sending data to the peripheral. */
    RingBufferHandler_t tx_queue; /*!< Ring buffer handler for managing the outgoing data queue. */
    uint8_t *add_to_tx_msg;       /*!< Pointer to the buffer used in add_to_tx_msg */
    uint8_t *exec_tx_msg;         /*!< Pointer to the buffer used in exec_tx_msg */
};

#endif /*! PAL_H */
