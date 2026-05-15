#ifndef PAL_HANDLER_TEST_UTILS_H
#define PAL_HANDLER_TEST_UTILS_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

/**
 * \brief Performs a deep copy of a PalHandler.
 * Allocates a new PalHandler and duplicates all internal ring buffer data and PalMessage buffers based on the max_message_size.
 *
 * \param[in] src       Pointer to the source handler to copy.
 * \param[out] harena   Pointer to an arena allocator handler.
 * \retval              Pointer to the new PalHandler
 * \retval              NULL if allocation fails.
 */
struct PalHandler *pal_handler_deep_copy(const struct PalHandler *src, struct ArenaAllocatorHandler *harena);

/**
 * \brief Deeply compares two PalHandler structures.
 *
 * \param[in] a     Pointer to first pal handler.
 * \param[in] b     Pointer to first pal handler.
 * \retval          true if logically equivalent. 
 * \retval          false otherwise.
 */
bool pal_handler_deep_compare(const struct PalHandler *a, const struct PalHandler *b);
bool compare_ring_buffer(const struct RingBufferHandler *a, const struct RingBufferHandler *b);
bool compare_pal_message(const struct PalMessage *a, const struct PalMessage *b);
struct PalMessage *copy_pal_message_buffer(const struct PalMessage *src, uint32_t max_payload_size, struct ArenaAllocatorHandler *harena);
bool copy_ring_buffer_data(struct RingBufferHandler *dest, const struct RingBufferHandler *src, struct ArenaAllocatorHandler *harena);

/* ... existing declarations ... */
#endif /* PAL_HANDLER_TEST_UTILS_H */
