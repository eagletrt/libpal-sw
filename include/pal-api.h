/*!
 * \file            pal-api.h
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

#ifndef PAL_API_H
#define PAL_API_H

#include "pal.h"
#include "arena-allocator.h"

#include <stddef.h>
#include <stdint.h>

/*!
 * \brief           Initialize the P.A.L. handler.
 *
 * \param[out]      hpal: A pointer to the P.A.L. handler to initialize.
 * \param[in]       rx_capacity: Number of elements in the reception queue.
 * \param[in]       tx_capacity: The capacity of the transmission queue.
 * \param[in]       max_msg_size: Maximum size of the message.
 * \param[in]       deserialize: A pointer to a function that should deserialize messages.
 * \param[in]       send: A pointer to a function that should send messages to the desired peripheral.
 * \param[in]       cs_enter: A pointer to a function that should manage a critical section (can be NULL).
 * \param[in]       cs_exit: A pointer to a function that should exit a critical section (can be NULL).
 * \param[out]      arena: A pointer to the arena allocator handler.
 * \retval          PAL_RC_OK on success, an error otherwise:
 * \retval          PAL_RC_NULL_POINTER  if `hpal`, `send` or `arena` is NULL.
 * \retval          PAL_RC_INVALID_ARGUMENT if `rx_capacity`, `tx_capacity` or `max_msg_size` are invalid.
 */
enum PalReturnCode pal_api_init(struct PalHandler *hpal,
                                uint32_t rx_capacity,
                                uint32_t tx_capacity,
                                uint32_t max_msg_size,
                                pal_deserialize_callback deserialize,
                                pal_send_callback send,
                                void (*cs_enter)(void),
                                void (*cs_exit)(void),
                                struct ArenaAllocatorHandler *arena);

/*!
 * \brief           Add to the reception queue.
 *
 * \param[out]      hpal: A pointer to the P.A.L. handler.
 * \param[in]       buff: A pointer to the buffer provided by the peripheral.
 * \param[in]       size: The size of `buff` buffer.
 * \retval          PAL_RC_OK on success, an error otherwise:
 * \retval          PAL_RC_NULL_POINTER if `hpal` or `buff` is NULL;
 * \retval          PAL_RC_MESSAGE_TOO_BIG if `size` is greater than `max_msg_size`;
 * \retval          PAL_RC_INVALID_ARGUMENT if `size` is invalid;
 * \retval          PAL_RC_QUEUE_FULL if the reception queue is full;
 * \retval          PAL_RC_IO_ERROR if the "push-back" operation fails.
 */
enum PalReturnCode pal_api_add_to_rx_queue(struct PalHandler *hpal, uint8_t *buff, uint32_t size);

/*!
 * \brief           Pop the first message and exec the deserialization.
 *
 * \param[out]      hpal: A pointer to the P.A.L. handler.
 * \param[out]      out: A pointer to the buffer used to transmit.
 * \retval          PAL_RC_OK on success, an error otherwise:
 * \retval          PAL_RC_NULL_POINTER if `hpal` or `out` is NULL;
 * \retval          PAL_RC_QUEUE_EMPTY if the reception queue is empty;
 * \retval          PAL_RC_IO_ERROR if the "pop-front" operation fails;
 * \retval          PAL_RC_DESERIALIZATION_ERROR if the deserialization fails.
 */
enum PalReturnCode pal_api_exec_rx(struct PalHandler *hpal, void *out);

/*!
 * \brief           Add data to the transmission buffer.
 *
 * \param[out]      hpal: A pointer to the P.A.L. handler.
 * \param[in]       data: A pointer to the structured data.
 * \param[in]       size: Number of bytes of the data.
 * \retval          PAL_RC_OK on success, an error otherwise:
 * \retval          PAL_RC_NULL_POINTER if `hpal` or `data` is NULL;
 * \retval          PAL_RC_INVALID_ARGUMENT if `size` is invalid;
 * \retval          PAL_RC_MESSAGE_TOO_BIG if `size` is greater than `max_msg_size`;
 * \retval          PAL_RC_QUEUE_FULL if the transmission queue is full;
 * \retval          PAL_RC_IO_ERROR if the "push-back" operation fails.
 */
enum PalReturnCode pal_api_add_to_tx_queue(struct PalHandler *hpal, void *data, uint32_t size);

/*!
 * \brief           Execute data transmission for the desired peripheral.
 *
 * \param[out]      hpal: A pointer to the P.A.L. handler.
 * \retval          PAL_RC_OK on success, an error otherwise:
 * \retval          PAL_RC_NULL_POINTER if `hpal` is NULL;
 * \retval          PAL_RC_QUEUE_EMPTY if the transmission queue is empty;
 * \retval          PAL_RC_IO_ERROR if the "pop-front" operation fails.
 * \retval          PAL_RC_SERIALIZATION_ERROR if the serialization fails.
 */
enum PalReturnCode pal_api_exec_tx(struct PalHandler *hpal);

#endif /*! PAL_API_H */
