/*!
 * \file            pal-api.h
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

#ifndef PAL_API_H
#define PAL_API_H

#include "pal.h"
#include "arena-allocator.h"

/*!
 * \brief           Initialize the P.A.L. handler.
 *
 * \param[out]      hpal: A pointer to the P.A.L. handler to initialize.
 * \param[in]       tx_capacity: The capacity of the transmission queue.
 * \param[in]       max_msg_size: The maximum size of a message for this handler.
 * \param[in]       send: A pointer to a function that should send messages to the desired peripheral.
 * \param[in]       cs_enter: A pointer to a function that should manage a critical section (can be NULL).
 * \param[in]       cs_exit: A pointer to a function that should exit a critical section (can be NULL).
 * \param[out]      arena: A pointer to the arena allocator handler.
 * \retval          PAL_RC_OK on success, an error otherwise:
 * \retval          PAL_RC_NULL_PTR  if `hpal`, `send`, or `arena` are NULL.
 * \retval          PAL_RC_INVALID_PARAM if `tx_capacity` or `max_msg_size` are invalid.
 */
enum PalReturnCode pal_api_init(struct PalHandler *hpal,
                                uint32_t tx_capacity,
                                uint32_t max_msg_size,
                                pal_send_fn send,
                                void (*cs_enter)(void),
                                void (*cs_exit)(void),
                                ArenaAllocatorHandler_t *arena);

/*!
 * \brief           Add data to the transmission buffer.
 *
 * \param[out]      hpal: A pointer to the P.A.L. handler.
 * \param[in]       data: A pointer to the structured data.
 * \param[in]       size: Number of bytes of the data.
 * \retval          PAL_RC_OK on success, an error otherwise:
 * \retval          PAL_RC_NULL_PTR if `hpal` or `data` is NULL;
 * \retval          PAL_RC_INVALID_PARAM if `size` is invalid;
 * \retval          PAL_RC_MSG_TOO_BIG if `size` is greater than `max_msg_size`;
 * \retval          PAL_RC_QUEUE_FULL if the transmission queue is full;
 * \retval          PAL_RC_IO_ERR if the "push-back" operation fails.
 */
enum PalReturnCode pal_api_add_to_tx_queue(struct PalHandler *hpal, void *data,uint32_t size);

/*!
 * \brief           Execute data transmission for the desired peripheral.
 *
 * \param[out]      hpal: A pointer to the P.A.L. handler.
 * \retval          PAL_RC_OK on success, an error otherwise:
 * \retval          PAL_RC_NULL_PTR if `hpal` is NULL;
 * \retval          PAL_RC_QUEUE_EMPTY if the transmission queue is empty;
 * \retval          PAL_RC_IO_ERR if the "pop-front" operation fails.
 * \retval          PAL_RC_SERIALIZATION_ERR if the serialization fails.
 */
enum PalReturnCode pal_api_exec_tx(struct PalHandler *hpal);

#endif /*! PAL_API_H */
