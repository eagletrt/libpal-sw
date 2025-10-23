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
 * \param[out]      hpal: A pointer to the handler to initialize.
 * \param[in]       protocol: Protocol used
 * \param[in]       send: Send function
 * \param[in]       recv: Receive function
 * \param[in]       driver_ctx: Internal driver context
 * \param[in]       enter_cs: Enter critical section function
 * \param[in]       exit_cs: Exit critical section function
 * \param[in]       app_rx_cb: Application callback function
 * \param[in]       serialize: Serialize function
 * \param[in]       deserialize: Deserialize function
 * \param[in]       rx_buffer_size: Size of the receive buffer
 * \param[in]       tx_buffer_size: Size of the transmit buffer
 * \param[in]       arena: Arena allocator handler (already initialized)
 * \return          PAL_RC_OK on success, and error code otherwise:
 *                      - PAL_RC_NULL_PTR
 */
enum PalReturnCode pal_api_init(
    struct PalHandler *hpal,
    pal_send_raw_fn send,
    void (*enter_cs)(void),
    void (*exit_cs)(void),
    pal_serialize_fn serialize,
    pal_deserialize_fn deserialize,
    size_t rx_buffer_size,
    size_t tx_buffer_size,
    ArenaAllocatorHandler_t *arena);

/*!
 * \brief           Function called by the driver inside the ISR routine to give the message data to PAL
 *
 * \param[out]      hpal: PAL handler
 * \param[out]      raw_data: Raw data buffer provided by the driver
 * \param[in]       size: Size of the raw data buffer
 * \return          PAL_RC_OK on success, an error code otherwise.
 */
enum PalReturnCode pal_api_add_rx(struct PalHandler *hpal, uint8_t *raw_data, size_t size);

/*!
 * \brief           Function to send data using the provided handler and serialization function
 *
 * \param[out]      hpal:
 * \param[in]       data:
 * \param[in]       serialize:
 * \return          PAL_RC_OK on success, an error code otherwise.
 */
enum PalReturnCode pal_api_send(struct PalHandler *hpal, const void *data, pal_serialize_fn serialize);

/*!
 * \brief Function to process all the messages in the rx queue, to be called by the application
 *
 * \param[out]      hpal:
 * \return          PAL_RC_OK on success, an error code otherwise.
 */
enum PalReturnCode pal_api_get_rx(struct PalHandler *hpal, void *destination);

#endif /*! PAL_API_H */
