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
#include "arena-allocator-api.h"
#include "ring-buffer.h"
#include "ring-buffer-api.h"
#include "eagletrt.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

EAGLETRT_STATIC_INLINE enum PalReturnCode prv_pal_deserialize_dummy(const struct PalMessage *in, void *out) {
    return memcpy(out, in->data, in->size) == NULL ? PAL_RC_DESERIALIZATION_ERR : PAL_RC_OK;
}

enum PalReturnCode pal_api_init(struct PalHandler *hpal,
                                uint32_t rx_capacity,
                                uint32_t tx_capacity,
                                uint32_t max_msg_size,
                                pal_deserialize_fn deserialize,
                                pal_send_fn send,
                                void (*cs_enter)(void),
                                void (*cs_exit)(void),
                                struct ArenaAllocatorHandler *arena) {
    if (hpal == NULL || send == NULL || arena == NULL) {
        return PAL_RC_NULL_PTR;
    }
    if (max_msg_size <= 0) {
        return PAL_RC_INVALID_PARAM;
    }
    hpal->deserialize = deserialize == NULL ? prv_pal_deserialize_dummy : deserialize;
    hpal->send = send;
    ring_buffer_api_init(&hpal->rx_queue, max_msg_size + sizeof(uint32_t), rx_capacity, cs_enter, cs_exit, arena);
    ring_buffer_api_init(&hpal->tx_queue, sizeof(uint32_t) + max_msg_size, tx_capacity, cs_enter, cs_exit, arena);
    hpal->max_msg_size = max_msg_size;
    hpal->add_to_rx_msg = (uint8_t *)arena_allocator_api_alloc(arena, max_msg_size + sizeof(uint32_t));
    hpal->exec_rx_msg = (uint8_t *)arena_allocator_api_alloc(arena, max_msg_size + sizeof(uint32_t));
    hpal->add_to_tx_msg = (uint8_t *)arena_allocator_api_alloc(arena, sizeof(uint32_t) + max_msg_size);
    hpal->exec_tx_msg = (uint8_t *)arena_allocator_api_alloc(arena, sizeof(uint32_t) + max_msg_size);
    return PAL_RC_OK;
}

enum PalReturnCode pal_api_add_to_rx_queue(struct PalHandler *hpal, uint8_t *buff, uint32_t size) {
    if (hpal == NULL || buff == NULL)
        return PAL_RC_NULL_PTR;

    if (ring_buffer_api_is_full(&hpal->rx_queue))
        return PAL_RC_QUEUE_FULL;

    if (size <= 0)
        return PAL_RC_INVALID_PARAM;

    if (size > hpal->max_msg_size)
        return PAL_RC_MSG_TOO_BIG;

    if (ring_buffer_api_is_full(&hpal->rx_queue))
        return PAL_RC_QUEUE_FULL;

    memcpy(hpal->add_to_rx_msg, &size, sizeof(uint32_t));
    memcpy((uint8_t *)hpal->add_to_rx_msg + sizeof(uint32_t), buff, size);
    enum RingBufferReturnCode res = ring_buffer_api_push_back(&hpal->rx_queue, hpal->add_to_rx_msg);
    if (res == RING_BUFFER_RC_FULL) {
        return PAL_RC_QUEUE_FULL;
    } else if (res != RING_BUFFER_RC_OK) {
        return PAL_RC_IO_ERR;
    }
    return PAL_RC_OK;
}

enum PalReturnCode pal_api_add_to_tx_queue(struct PalHandler *hpal, void *buff, uint32_t size) {
    if (hpal == NULL || buff == NULL)
        return PAL_RC_NULL_PTR;

    if (size <= 0)
        return PAL_RC_INVALID_PARAM;

    if (size > hpal->max_msg_size)
        return PAL_RC_MSG_TOO_BIG;

    if (ring_buffer_api_is_full(&hpal->tx_queue))
        return PAL_RC_QUEUE_FULL;

    memcpy(hpal->add_to_tx_msg, &size, sizeof(uint32_t));
    memcpy(hpal->add_to_tx_msg + sizeof(uint32_t), buff, size);
    enum RingBufferReturnCode res = ring_buffer_api_push_back(&hpal->tx_queue, hpal->add_to_tx_msg);
    if (res == RING_BUFFER_RC_FULL) {
        return PAL_RC_QUEUE_FULL;
    } else if (res != RING_BUFFER_RC_OK) {
        return PAL_RC_IO_ERR;
    }
    return PAL_RC_OK;
}

enum PalReturnCode pal_api_exec_rx(struct PalHandler *hpal, void *out) {
    if (hpal == NULL || out == NULL)
        return PAL_RC_NULL_PTR;

    if (ring_buffer_api_is_empty(&hpal->rx_queue))
        return PAL_RC_QUEUE_EMPTY;

    enum RingBufferReturnCode res = ring_buffer_api_pop_front(&hpal->rx_queue, hpal->exec_rx_msg);
    if (res == RING_BUFFER_RC_EMPTY) {
        return PAL_RC_QUEUE_EMPTY;
    } else if (res != RING_BUFFER_RC_OK) {
        return PAL_RC_IO_ERR;
    }

    return hpal->deserialize((struct PalMessage *)hpal->exec_rx_msg, out);
}

enum PalReturnCode pal_api_exec_tx(struct PalHandler *hpal) {
    if (hpal == NULL)
        return PAL_RC_NULL_PTR;

    if (ring_buffer_api_is_empty(&hpal->tx_queue))
        return PAL_RC_QUEUE_EMPTY;

    enum RingBufferReturnCode res = ring_buffer_api_pop_front(&hpal->tx_queue, hpal->exec_tx_msg);

    if (res == RING_BUFFER_RC_EMPTY) {
        return PAL_RC_QUEUE_EMPTY;
    } else if (res != RING_BUFFER_RC_OK) {
        return PAL_RC_IO_ERR;
    }

    return hpal->send((struct PalMessage *)hpal->exec_tx_msg);
}
