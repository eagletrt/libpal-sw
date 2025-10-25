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
                                size_t rx_capacity,
                                pal_deserialize_fn deserialize,
                                void (*cs_enter)(void),
                                void (*cs_exit)(void),
                                ArenaAllocatorHandler_t *arena) {
    if (!hpal || !deserialize || !arena)
        return PAL_RC_NULL_PTR;

    hpal->deserialize = deserialize;
    ring_buffer_api_init(&hpal->rx_queue, sizeof(void *), rx_capacity, cs_enter, cs_exit, arena);

    return PAL_RC_OK;
}

enum PalReturnCode pal_api_add_to_rx_queue(struct PalHandler *hpal, uint8_t *buff, size_t size) {
    if (!hpal || !buff)
        return PAL_RC_NULL_PTR;

    if (ring_buffer_api_is_full(&hpal->rx_queue))
        return PAL_RC_QUEUE_FULL;

    struct PalMessage msg = { buff, size };
    RingBufferReturnCode res = ring_buffer_api_push_back(&hpal->rx_queue, &msg);
    if (res != RING_BUFFER_OK)
        return PAL_RC_IO_ERR;

    return PAL_RC_OK;
}

enum PalReturnCode pal_api_exec_rx(struct PalHandler *hpal, void *out) {
    if (!hpal || !out)
        return PAL_RC_NULL_PTR;

    if (ring_buffer_api_is_empty(&hpal->rx_queue))
        return PAL_RC_QUEUE_EMPTY;

    struct PalMessage msg = { 0 };
    RingBufferReturnCode res_buff = ring_buffer_api_pop_front(&hpal->rx_queue, &msg);
    if (res_buff != RING_BUFFER_OK)
        return PAL_RC_IO_ERR;

    return hpal->deserialize(&msg, out);
}
