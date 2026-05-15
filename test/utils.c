#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pal.h"
#include "pal-api.h"
#include "arena-allocator.h"
#include "arena-allocator-api.h"

/**
 * \brief Helper function to deep copy a PalMessage buffer.
 * Assumes the buffer is allocated to hold 'max_payload_size' bytes.
 *
 * \param src       Pointer to the source message to copy.
 * \param harena    Pointer to an arena allocator handler.
 * \retval          Pointer to the new PalMessage
 * \retval          NULL if allocation fails.
 */
struct PalMessage *copy_pal_message_buffer(const struct PalMessage *src, uint32_t max_payload_size, struct ArenaAllocatorHandler *harena) {
    if (src == NULL)
        return NULL;

    size_t allocation_size = sizeof(struct PalMessage) + max_payload_size;
    struct PalMessage *dest = (struct PalMessage *)arena_allocator_api_alloc(harena, allocation_size);
    if (dest != NULL) {
        dest->size = src->size;
        memcpy(dest->payload, src->payload, src->size);
    }
    return dest;
}

bool copy_ring_buffer_data(struct RingBufferHandler *dest, const struct RingBufferHandler *src, struct ArenaAllocatorHandler *harena) {
    if (src->data == NULL || src->capacity == 0) {
        dest->data = NULL;
        return true;
    }

    size_t bytes = src->capacity * src->data_size;
    dest->data = arena_allocator_api_alloc(harena, bytes);
    if (dest->data == NULL)
        return false;

    memcpy(dest->data, src->data, bytes);
    return true;
}

struct PalHandler *pal_handler_deep_copy(const struct PalHandler *src, struct ArenaAllocatorHandler *harena) {
    if (src == NULL || harena == NULL)
        return NULL;

    // 1. Allocate the main handler structure
    const size_t item_size = sizeof(struct PalHandler);
    struct PalHandler *dest = (struct PalHandler *)arena_allocator_api_alloc(harena, item_size);
    if (dest == NULL)
        return NULL;

    // 2. Shallow copy all values and function pointers natively
    *dest = *src;

    if (copy_ring_buffer_data(&dest->rx_queue, &src->rx_queue, harena) == false)
        return NULL;
    if (copy_ring_buffer_data(&dest->tx_queue, &src->tx_queue, harena) == false)
        return NULL;

    // 6. Deep copy the PalMessage buffers
    uint32_t max_size = src->max_message_size;

    if (src->add_to_rx_message != NULL) {
        dest->add_to_rx_message = copy_pal_message_buffer(src->add_to_rx_message, max_size, harena);
        if (dest->add_to_rx_message == NULL)
            return NULL;
    }

    if (src->process_rx_message != NULL) {
        dest->process_rx_message = copy_pal_message_buffer(src->process_rx_message, max_size, harena);
        if (dest->process_rx_message == NULL)
            return NULL;
    }

    if (src->add_to_tx_message != NULL) {
        dest->add_to_tx_message = copy_pal_message_buffer(src->add_to_tx_message, max_size, harena);
        if (dest->add_to_tx_message == NULL)
            return NULL;
    }

    if (src->process_tx_message != NULL) {
        dest->process_tx_message = copy_pal_message_buffer(src->process_tx_message, max_size, harena);
        if (dest->process_tx_message == NULL)
            return NULL;
    }

    return dest;
}

// Helper to compare PalMessage buffers based on their valid payload size
bool compare_pal_message(const struct PalMessage *a, const struct PalMessage *b) {
    if (a == b)
        return true; // Handles both being NULL or the exact same pointer
    if (a == NULL || b == NULL)
        return false;
    if (a->size != b->size)
        return false;

    // Compare only the valid payload bytes defined by 'size'
    if (a->size > 0) {
        if (memcmp(a->payload, b->payload, a->size) != 0)
            return false;
    }
    return true;
}

// Helper to compare RingBufferHandlers
bool compare_ring_buffer(const struct RingBufferHandler *a, const struct RingBufferHandler *b) {
    if (a == b)
        return true; // Handles both being NULL or the exact same pointer
    if (a == NULL || b == NULL)
        return false;

    if (a->start != b->start ||
        a->size != b->size ||
        a->data_size != b->data_size ||
        a->capacity != b->capacity ||
        a->cs_enter != b->cs_enter ||
        a->cs_exit != b->cs_exit) {
        return false;
    }

    if (a->data == b->data)
        return true; // Both NULL or same pointer
    if (a->data == NULL || b->data == NULL)
        return false;

    size_t total_bytes = a->capacity * a->data_size;
    if (total_bytes > 0) {
        if (memcmp(a->data, b->data, total_bytes) != 0)
            return false;
    }

    return true;
}

bool pal_handler_deep_compare(const struct PalHandler *a, const struct PalHandler *b) {
    if (a == b)
        return true;
    if (a == NULL || b == NULL)
        return false;

    // Compare simple values and function pointer fields
    if (a->max_message_size != b->max_message_size ||
        a->deserialize != b->deserialize ||
        a->send != b->send) {
        return false;
    }

    // Compare Ring Buffers
    if (compare_ring_buffer(&a->rx_queue, &b->rx_queue) == false)
        return false;
    if (compare_ring_buffer(&a->tx_queue, &b->tx_queue) == false)
        return false;

    // Compare PalMessage buffers
    if (compare_pal_message(a->add_to_rx_message, b->add_to_rx_message) == false)
        return false;
    if (compare_pal_message(a->process_rx_message, b->process_rx_message) == false)
        return false;
    if (compare_pal_message(a->add_to_tx_message, b->add_to_tx_message) == false)
        return false;
    if (compare_pal_message(a->process_tx_message, b->process_tx_message) == false)
        return false;

    return true;
}
