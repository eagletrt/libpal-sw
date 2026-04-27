/*!
 * \file            pal.h
 * \date            2026-04-24
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
    PAL_RC_OK,                  /*!< Everything is fine. */
    PAL_RC_INVALID_PARAM,       /*!< Invalid parameter data. */
    PAL_RC_NULL_PTR,            /*!< Unexpected NULL pointer. */
    PAL_RC_IO_ERR,              /*!< Generic I/O error. */
    PAL_RC_QUEUE_FULL,          /*!< The queue is full. */
    PAL_RC_QUEUE_EMPTY,         /*!< The queue is empty. */
    PAL_RC_MSG_TOO_BIG,         /*!< Message size is too big. */
    PAL_RC_DESERIALIZATION_ERR, /*!< Deserialization error. */
    PAL_RC_SERIALIZATION_ERR    /*!< Serialization error. */
};

/*!
 * \brief           A structure representing a message.
 */
struct PalMessage {
    uint32_t size;     /*!< The size of the message. */
    uint8_t payload[]; /*!< Flexible array member holding the payload (valid bytes = size). */
};

/*!
 * \brief           Type definition for a function pointer used to deserialize data.
 *
 * \param[in]       in: Pointer to the data to be deserialized.
 * \param[out]      out: Pointer to the deserialized data.
 * \retval          PAL_RC_OK on success,
 * \retval          PAL_RC_DESERIALIZATION_ERR if deserialization fails.
 * \retval          PAL_RC_NULL_PTR if `in` or `out` is NULL
 * \retval          PAL_RC_IO_ERR otherwise.
 */
typedef enum PalReturnCode (*pal_deserialize_callback)(const struct PalMessage *in, void *out);

/*!
 * \brief           Type definition for a function pointer used to send data.
 *
 * \param[in]       msg: Pointer to the message to be sent.
 * \retval          PAL_RC_OK on success, an error code otherwise:
 * \retval          PAL_RC_NULL_PTR if `msg` is NULL;
 * \retval          PAL_RC_SERIALIZATION_ERR if serialization fails;
 * \retval          PAL_RC_IO_ERR if fails.
 */
typedef enum PalReturnCode (*pal_send_callback)(const struct PalMessage *msg);

/*!
 * \brief           A structure that encapsulates data and functions required to
 *                  handle the communication with the desired peripheral.
 *
 * \attention       This structure should not be used directly.
 */
struct PalHandler {
    struct RingBufferHandler rx_queue;    /*!< Ring buffer handler for managing received messages queue. */
    struct RingBufferHandler tx_queue;    /*!< Ring buffer handler for managing the outgoing data queue. */
    pal_deserialize_callback deserialize; /*!< Function pointer for deserializing messages. */
    pal_send_callback send;               /*!< Function pointer for sending data to the peripheral. */
    uint32_t max_msg_size;                /*!< Maximum size of the message */
    uint8_t *add_to_rx_msg;               /*!< Pointer to the buffer used in add_to_rx_msg */
    uint8_t *exec_rx_msg;                 /*!< Pointer to the buffer used in exec_rx_msg */
    uint8_t *add_to_tx_msg;               /*!< Pointer to the buffer used in add_to_tx_msg */
    uint8_t *exec_tx_msg;                 /*!< Pointer to the buffer used in exec_tx_msg */
};

#endif /*! PAL_H */
