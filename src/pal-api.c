/*!
 * \file            pal-api.c
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

EAGLETRT_STATIC_INLINE enum PalReturnCode prv_pal_deserialize_dummy(const struct PalMessage *message, void *out) {
    return memcpy(out, message->payload, message->size) == NULL ? PAL_RC_DESERIALIZATION_ERROR : PAL_RC_OK;
}

enum PalReturnCode pal_api_init(struct PalHandler *pal_handler,
                                uint32_t rx_capacity,
                                uint32_t tx_capacity,
                                uint32_t max_message_size,
                                pal_deserialize_callback deserialize,
                                pal_send_callback send,
                                void (*cs_enter)(void),
                                void (*cs_exit)(void),
                                struct ArenaAllocatorHandler *arena) {
    if (pal_handler == NULL || send == NULL || arena == NULL) {
        return PAL_RC_NULL_POINTER;
    }
    if (max_message_size == 0) {
        return PAL_RC_INVALID_ARGUMENT;
    }
    pal_handler->deserialize = deserialize == NULL ? prv_pal_deserialize_dummy : deserialize;
    pal_handler->send = send;
    pal_handler->max_message_size = max_message_size;

    /* Initialize ring buffers */
    const size_t item_size = sizeof(struct PalMessage) + max_message_size;
    enum RingBufferReturnCode ring_res = RING_BUFFER_RC_OK;
    ring_res = ring_buffer_api_init(&pal_handler->rx_queue, item_size, rx_capacity, cs_enter, cs_exit, arena);
    if (ring_res == RING_BUFFER_RC_NULL_POINTER) {
        return PAL_RC_IO_ERROR;
    }
    ring_res = ring_buffer_api_init(&pal_handler->tx_queue, item_size, tx_capacity, cs_enter, cs_exit, arena);
    if (ring_res == RING_BUFFER_RC_NULL_POINTER) {
        return PAL_RC_IO_ERROR;
    }

    /* Allocate message buffers */
    struct PalMessage *tmp_add_to_rx = (struct PalMessage *)arena_allocator_api_alloc(arena, item_size);
    if (tmp_add_to_rx == NULL) {
        return PAL_RC_IO_ERROR;
    }
    struct PalMessage *tmp_process_rx = (struct PalMessage *)arena_allocator_api_alloc(arena, item_size);
    if (tmp_process_rx == NULL) {
        return PAL_RC_IO_ERROR;
    }

    struct PalMessage *tmp_add_to_tx = (struct PalMessage *)arena_allocator_api_alloc(arena, item_size);
    if (tmp_add_to_tx == NULL) {
        return PAL_RC_IO_ERROR;
    }

    struct PalMessage *tmp_process_tx = (struct PalMessage *)arena_allocator_api_alloc(arena, item_size);
    if (tmp_process_tx == NULL) {
        return PAL_RC_IO_ERROR;
    }

    /* All allocations succeeded —> assign to pal_handler */
    pal_handler->add_to_rx_message = tmp_add_to_rx;
    pal_handler->process_rx_message = tmp_process_rx;
    pal_handler->add_to_tx_message = tmp_add_to_tx;
    pal_handler->process_tx_message = tmp_process_tx;

    pal_handler->add_to_rx_message->size = max_message_size;
    memset(pal_handler->add_to_rx_message->payload, 0, max_message_size);
    pal_handler->process_rx_message->size = max_message_size;
    memset(pal_handler->process_rx_message->payload, 0, max_message_size);
    pal_handler->add_to_tx_message->size = max_message_size;
    memset(pal_handler->add_to_tx_message->payload, 0, max_message_size);
    pal_handler->process_tx_message->size = max_message_size;
    memset(pal_handler->process_tx_message->payload, 0, max_message_size);
    return PAL_RC_OK;
}

enum PalReturnCode pal_api_add_to_rx_queue(struct PalHandler *pal_handler, uint8_t *payload, uint32_t size) {
    if (pal_handler == NULL || payload == NULL) {
        return PAL_RC_NULL_POINTER;
    }
    if (size == 0) {
        return PAL_RC_INVALID_ARGUMENT;
    }
    if (size > pal_handler->max_message_size) {
        return PAL_RC_MESSAGE_TOO_BIG;
    }

    if (ring_buffer_api_is_full(&pal_handler->rx_queue)) {
        return PAL_RC_QUEUE_FULL;
    }

    pal_handler->add_to_rx_message->size = size;
    memcpy(pal_handler->add_to_rx_message->payload, payload, size);
    enum RingBufferReturnCode res = ring_buffer_api_push_back(&pal_handler->rx_queue, pal_handler->add_to_rx_message);
    if (res == RING_BUFFER_RC_FULL) {
        return PAL_RC_QUEUE_FULL;
    }
    return (res != RING_BUFFER_RC_OK) ? PAL_RC_IO_ERROR : PAL_RC_OK;
}

enum PalReturnCode pal_api_add_to_tx_queue(struct PalHandler *pal_handler, void *payload, uint32_t size) {
    if (pal_handler == NULL || payload == NULL) {
        return PAL_RC_NULL_POINTER;
    }
    if (size == 0) {
        return PAL_RC_INVALID_ARGUMENT;
    }
    if (size > pal_handler->max_message_size) {
        return PAL_RC_MESSAGE_TOO_BIG;
    }

    if (ring_buffer_api_is_full(&pal_handler->tx_queue)) {
        return PAL_RC_QUEUE_FULL;
    }

    pal_handler->add_to_tx_message->size = size;
    memcpy(pal_handler->add_to_tx_message->payload, payload, size);
    enum RingBufferReturnCode res = ring_buffer_api_push_back(&pal_handler->tx_queue, pal_handler->add_to_tx_message);
    if (res == RING_BUFFER_RC_FULL) {
        return PAL_RC_QUEUE_FULL;
    }
    return (res != RING_BUFFER_RC_OK) ? PAL_RC_IO_ERROR : PAL_RC_OK;
}

enum PalReturnCode pal_api_process_rx(struct PalHandler *pal_handler, void *out) {
    if (pal_handler == NULL || out == NULL) {
        return PAL_RC_NULL_POINTER;
    }
    if (ring_buffer_api_is_empty(&pal_handler->rx_queue)) {
        return PAL_RC_QUEUE_EMPTY;
    }

    enum RingBufferReturnCode res = ring_buffer_api_pop_front(&pal_handler->rx_queue, pal_handler->process_rx_message);
    if (res == RING_BUFFER_RC_EMPTY) {
        return PAL_RC_QUEUE_EMPTY;
    }
    if (res != RING_BUFFER_RC_OK) {
        return PAL_RC_IO_ERROR;
    }

    return pal_handler->deserialize((struct PalMessage *)pal_handler->process_rx_message, out);
}

enum PalReturnCode pal_api_process_tx(struct PalHandler *pal_handler) {
    if (pal_handler == NULL) {
        return PAL_RC_NULL_POINTER;
    }
    if (ring_buffer_api_is_empty(&pal_handler->tx_queue)) {
        return PAL_RC_QUEUE_EMPTY;
    }

    enum RingBufferReturnCode res = ring_buffer_api_pop_front(&pal_handler->tx_queue, pal_handler->process_tx_message);

    if (res == RING_BUFFER_RC_EMPTY) {
        return PAL_RC_QUEUE_EMPTY;
    }
    if (res != RING_BUFFER_RC_OK) {
        return PAL_RC_IO_ERROR;
    }

    return pal_handler->send((struct PalMessage *)pal_handler->process_tx_message);
}
