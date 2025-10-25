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
 * \param[in]       send: A pointer to a function that should send messages to the desired peripheral.
 * \param[in]       cs_enter: A pointer to a function that should manage a critical section (can be NULL).
 * \param[in]       cs_exit: A pointer to a function that should exit a critical section (can be NULL).
 * \param[out]      arena: A pointer to the arena allocator handler.
 * \return          PAL_RC_OK on success, an error otherwise:
 *                   - PAL_RC_NULL_PTR  if `hpal`, `send`, or `serialize` is NULL.
 */
enum PalReturnCode pal_api_init(struct PalHandler *hpal,
                                size_t tx_capacity,
                                pal_send_fn send,
                                pal_serialize_fn serialize,
                                void (*cs_enter)(void),
                                void (*cs_exit)(void),
                                ArenaAllocatorHandler_t *arena);

/*! 
 * \brief           Add data to the transmission buffer.
 *
 * \param[out]      hpal: A pointer to the P.A.L. handler.
 * \param[in]       data: A pointer to the structured data.
 * \return          PAL_RC_OK on success, an error otherwise:
 *                   - PAL_RC_NULL_PTR if `hpal` or `data` is NULL;
 *                   - PAL_RC_QUEUE_FULL if the transmission queue is full;
 *                   - PAL_RC_IO_ERR if the "push-back" operation fails.
 */
enum PalReturnCode pal_api_add_to_tx_queue(struct PalHandler *hpal, void *data);

/*!
 * \brief           Exec data serialization and transmission to the desired peripheral.
 *
 * \param[out]      hpal: A pointer to the P.A.L. handler.
 * \param[out]      buff: A pointer to the buffer used to transmit.
 * \param[in]       size: The size of `buff` buffer.
 * \return          PAL_RC_OK on success, an error otherwise:
 *                   - PAL_RC_NULL_PTR if `hpal` or `buff` is NULL;
 *                   - PAL_RC_QUEUE_EMPTY if the transmission queue is full;
 *                   - PAL_RC_IO_ERR if the "pop-front" operation.
 *                   - PAL_RC_SER_ERRR if the serialization fails.
 */
enum PalReturnCode pal_api_exec_tx(struct PalHandler *hpal, uint8_t *buff, size_t size);

#endif /*! PAL_API_H */
