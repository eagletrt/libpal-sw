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

static void prv_pal_api_dummy_fn(void) {
}

enum PalReturnCode pal_api_init(
    struct PalHandler *hpal,
    pal_send_raw_fn send,
    void (*enter_cs)(void),
    void (*exit_cs)(void),
    pal_serialize_fn serialize,
    pal_deserialize_fn deserialize,
    ArenaAllocatorHandler_t *arena) {
    if (hpal == NULL)
        return PAL_RC_NULL_PTR;

    hpal->send = send == NULL ? prv_pal_api_dummy_send : send;
    hpal->enter_cs = enter_cs == NULL ? prv_pal_api_dummy_fn : enter_cs;
    hpal->exit_cs = exit_cs == NULL ? prv_pal_api_dummy_fn : exit_cs;

    if (hpal->deserialize == NULL || hpal->serialize == NULL) {
        return PAL_RC_NULL_PTR;
    }
    hpal->deserialize = deserialize;
    hpal->serialize = serialize;

    RingBufferReturnCode res = ring_buffer_api_init(&hpal->tx_buffer, sizeof(struct PalMessage), PAL_TX_BUFFER_SIZE, hpal->enter_cs, hpal->exit_cs, arena);
    if (res != RING_BUFFER_OK)
        return PAL_RC_NULL_PTR;

    res = ring_buffer_api_init(&hpal->rx_buffer, sizeof(struct PalMessage), PAL_RX_BUFFER_SIZE, hpal->enter_cs, hpal->exit_cs, arena);
    if (res != RING_BUFFER_OK)
        return PAL_RC_NULL_PTR;

    return PAL_RC_OK;
}

enum PalReturnCode pal_api_add_rx(struct PalHandler *hpal, uint8_t *raw_data, size_t size) {
    if (hpal == NULL)
        return PAL_RC_NULL_PTR;

    if (ring_buffer_api_is_full(&hpal->rx_buffer))
        return PAL_RC_BUFF_FULL;

    struct PalMessage msg = {
        .size = size,
        .ptr = raw_data,
    };

    RingBufferReturnCode res = ring_buffer_api_push_back(&hpal->rx_buffer, &msg);
    if (res != RING_BUFFER_OK) {
        return PAL_RC_IO_ERR;
    }

    hpal->app_event_notify(hpal->protocol);
    return PAL_RC_OK;
}

enum PalReturnCode pal_api_get_rx(struct PalHandler *hpal, void *destination) {
    if (hpal == NULL) {
        return PAL_RC_NULL_PTR;
    }
    if (ring_buffer_api_is_empty(&hpal->rx_buffer)) {
        return PAL_RC_BUFF_EMPTY;
    }
    struct PalMessage msg = { 0 };
    RingBufferReturnCode res_buff = ring_buffer_api_pop_front(&hpal->rx_buffer, &msg);
    if (res_buff != RING_BUFFER_OK) {
        return PAL_RC_IO_ERR;
    }
    enum PalReturnCode res = hpal->deserialize(&msg, destination);
    if (res != PAL_RC_OK) {
        return PAL_RC_DESR_ERR;
    }
    return PAL_RC_OK;
}
