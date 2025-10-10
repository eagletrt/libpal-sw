/*!
 * \file            pal-api.c
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

#include "pal.h"
#include "pal-api.h"
#include "arena-allocator.h"
#include "ring-buffer.h"
#include "ring-buffer-api.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static int prv_pal_api_dummy_send(const uint8_t *raw_data, size_t size) {
    /*! Avoid "unused variable" warning messages */
    (void)raw_data;
    (void)size;
    return 0;
}

static int prv_pal_api_dummy_recv(uint8_t *raw_data, size_t size) {
    /*! Avoid "unused variable" warning messages */
    (void)raw_data;
    (void)size;
    return 0;
}

static void prv_pal_api_dummy_fn(void) {
}

enum PalReturnCode pal_api_init(
    struct PalHandler *hpal,
    enum PalProtocol protocol,
    pal_recv_raw_fn recv,
    pal_send_raw_fn send,
    void *driver_ctx,
    void (*enter_cs)(void),
    void (*exit_cs)(void),
    void (*app_rx_cb)(void),
    pal_serialize_fn serialize,
    pal_deserialize_fn deserialize,
    ArenaAllocatorHandler_t *arena) {
    if (hpal == NULL)
        return PAL_RC_NULL_PTR;

    hpal->protocol = protocol;

    hpal->send = send == NULL ? prv_pal_api_dummy_send : send;
    hpal->recv = recv == NULL ? prv_pal_api_dummy_recv : recv;
    hpal->enter_cs = enter_cs == NULL ? prv_pal_api_dummy_fn : enter_cs;
    hpal->exit_cs = exit_cs == NULL ? prv_pal_api_dummy_fn : exit_cs;

    if (hpal->deserialize == NULL || hpal->serialize == NULL) {
        return PAL_RC_NULL_PTR;
    }
    hpal->deserialize = deserialize;
    hpal->serialize = serialize;

    hpal->driver_ctx = driver_ctx;

    if (app_rx_cb == NULL)
        return PAL_RC_NULL_PTR;

    RingBufferReturnCode res = ring_buffer_api_init(&hpal->tx_buffer, 1U, PAL_TX_BUFFER_SIZE, NULL, NULL, arena); //enter_cs, exit_cs, arena);
    if (res != RING_BUFFER_OK)
        return PAL_RC_NULL_PTR;

    res = ring_buffer_api_init(&hpal->rx_buffer, 1U, PAL_RX_BUFFER_SIZE, NULL, NULL, arena); // enter_cs, exit_cs, arena);
    if (res != RING_BUFFER_OK)
        return PAL_RC_NULL_PTR;

    return PAL_RC_OK;
}

enum PalReturnCode pal_api_drv_recv_cb(struct PalHandler *hpal, uint8_t *raw_data, size_t size, pal_deserialize_fn deserialize) {
    if (hpal == NULL)
        return PAL_RC_NULL_PTR;

    if (ring_buffer_api_is_full(&hpal->rx_buffer))
        return PAL_RC_BUFF_FULL;

    if (size > PAL_RX_BUFFER_SIZE)
        return PAL_RC_TOO_BIG;

    hpal->enter_cs();

    struct PalMessage msg = { 0 };
    msg.size = size;
    memcpy(msg.raw_data, raw_data, size);

    RingBufferReturnCode res = ring_buffer_api_push_back(&hpal->rx_buffer, &msg);
    if (res != RING_BUFFER_OK) { // TODO: change to return code to PAL_RC_BUFF_FULL? this can only happen if the buffer became full between checks, consder moving enter_cs to top?
        hpal->exit_cs();
        return PAL_RC_IO_ERR;
    }
    hpal->exit_cs();

    hpal->app_event_notify(hpal->protocol);
    return PAL_RC_OK;
}

enum PalReturnCode pal_api_process_rx(struct PalHandler *hpal) {
    while (true) { // Done to avoid checking buffer status outside cs, cs inside loop to allow for messages to be inserted while processing
        hpal->enter_cs();
        if (ring_buffer_api_is_empty(&hpal->rx_buffer)) {
            hpal->exit_cs();
            break;
        }
        if (ring_buffer_api_is_full(&hpal->rx_buffer)) {
            hpal->exit_cs();
            return PAL_RC_BUFF_FULL;
        }
        struct PalMessage msg = { 0 };
        RingBufferReturnCode res_buff = ring_buffer_api_pop_front(&hpal->rx_buffer, &msg);
        if (res_buff != RING_BUFFER_OK) {
            hpal->exit_cs();
            return PAL_RC_IO_ERR;
        }
        void *desr_msg = NULL;
        size_t size;
        enum PalReturnCode res = hpal->deserialize(&msg, desr_msg, &size);
        if (res != PAL_RC_OK) {
            hpal->exit_cs();
            return PAL_RC_DESR_ERR;
        }
        hpal->exit_cs(); // Possible racecondition between line 137 and line 138?
        hpal->app_rx_cb(desr_msg, size);
    }
    return PAL_RC_OK;
}
