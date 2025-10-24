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

enum PalReturnCode pal_api_init(struct PalHandler *hpal,
                                size_t tx_capacity,
                                pal_send_fn send,
                                pal_serialize_fn serialize,
                                void (*cs_enter)(void),
                                void (*cs_exit)(void),
                                ArenaAllocatorHandler_t *arena) {
    if (!hpal || !send || !serialize)
        return PAL_RC_NULL_PTR;

    hpal->send = send;
    hpal->serialize = serialize;

    RingBufferReturnCode res = ring_buffer_api_init(&hpal->tx_queue, sizeof(void *), tx_capacity, cs_enter, cs_exit, arena);
    if (res != RING_BUFFER_OK)
        return PAL_RC_NULL_PTR;

    return PAL_RC_OK;
}

enum PalReturnCode pal_api_add_to_tx_queue(struct PalHandler *hpal, void *data) {
    if (!hpal || !data)
        return PAL_RC_NULL_PTR;

    if (ring_buffer_api_is_full(&hpal->tx_queue))
        return PAL_RC_QUEUE_FULL;

    RingBufferReturnCode res = ring_buffer_api_push_back(&hpal->tx_queue, data);
    if (res != RING_BUFFER_OK)
        return PAL_RC_IO_ERR;

    return PAL_RC_OK;
}

enum PalReturnCode pal_api_exec_tx(struct PalHandler *hpal, uint8_t *buff, size_t size) {
    if (!hpal || !buff)
        return PAL_RC_NULL_PTR;

    if (ring_buffer_api_is_empty(&hpal->tx_queue))
        return PAL_RC_QUEUE_EMPTY;

    void *data = NULL;
    RingBufferReturnCode res = ring_buffer_api_pop_front(&hpal->tx_queue, data);
    if (res != RING_BUFFER_OK)
        return PAL_RC_IO_ERR;

    bzero(buff, size);
    int len = hpal->serialize(data, buff, size);
    if (len < 0)
        return PAL_RC_IO_ERR;

    len = hpal->send(buff, size);
    return len < 0 ? PAL_RC_IO_ERR : PAL_RC_OK;
}
