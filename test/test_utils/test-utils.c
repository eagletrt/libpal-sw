#include "unity.h"
#include <stdlib.h>
#include <string.h>
#include "pal.h"
#include "pal-api.h"
#include "arena-allocator.h"
#include "arena-allocator-api.h"
#include "utils.h"
#include "fff.h"

DEFINE_FFF_GLOBALS

FAKE_VOID_FUNC(dummy_deserialize)
FAKE_VOID_FUNC(dummy_send)
FAKE_VOID_FUNC(dummy_cs_enter)
FAKE_VOID_FUNC(dummy_cs_exit)

struct PalHandler base_handler;
struct ArenaAllocatorHandler harena;
struct PalMessage *a;
struct PalMessage *b;
const uint32_t TEST_MAX_MSG_SIZE = 64;
const uint32_t data_size = 10;
const uint32_t total_size = data_size + sizeof(struct PalMessage);

void setUp(void) {
    arena_allocator_api_init(&harena);
    pal_api_init(&base_handler, 2, 1, TEST_MAX_MSG_SIZE, (pal_deserialize_callback)dummy_deserialize, (pal_send_callback)dummy_send, dummy_cs_enter, dummy_cs_exit, &harena);
    a = arena_allocator_api_alloc(&harena, total_size);
    b = arena_allocator_api_alloc(&harena, total_size);
    a->size = data_size;
    b->size = a->size;
}

void tearDown(void) {
    arena_allocator_api_free(&harena);
}

/*!
 * \defgroup            deep_compare Test deep compare helper function
 * @{
 */

void check_copy_pal_message_buffer_null_source(void) {
    TEST_ASSERT_NULL(copy_pal_message_buffer(NULL, TEST_MAX_MSG_SIZE, &harena));
}

void check_copy_pal_message_buffer_valid_copy(void) {
    // Setup source message with specific data
    a->size = 5;
    a->payload[0] = 0xA;
    a->payload[4] = 0xB;

    struct PalMessage *copy = copy_pal_message_buffer(a, TEST_MAX_MSG_SIZE, &harena);

    TEST_ASSERT_NOT_NULL(copy);
    TEST_ASSERT_NOT_EQUAL_MESSAGE(a, copy, "copy should have a different memory address than the source");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(a->size, copy->size, "copy size should match source size");
    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(a->payload, copy->payload, a->size, "copy payload should match source payload");
}

/*!
 * @}
 */

/*!
 * \defgroup            compare_pal_message Test deep copy helper function
 * @{
 */

void check_pal_message_compare_same_pointer(void) {
    TEST_ASSERT_TRUE_MESSAGE(compare_pal_message(a, a), "compare_pal_message should return true when comparing the same pointer");
}

void check_pal_message_compare_null_pointer(void) {
    TEST_ASSERT_FALSE_MESSAGE(compare_pal_message(a, NULL), "compare_pal_message should return false when comparing with a NULL pointer");
    TEST_ASSERT_FALSE_MESSAGE(compare_pal_message(NULL, a), "compare_pal_message should return false when comparing with a NULL pointer");
}

void check_pal_message_compare_different_size(void) {
    b->size = a->size + 1;

    TEST_ASSERT_FALSE_MESSAGE(compare_pal_message(a, b), "compare_pal_message should return false when comparing different size messages");
}

void check_pal_message_compare_same_message(void) {
    memset(a->payload, 0, a->size);
    memset(b->payload, 0, b->size);
    TEST_ASSERT_TRUE_MESSAGE(compare_pal_message(a, b), "compare_pal_message should return true when comparing same messages");
}

void check_pal_message_compare_different_message(void) {
    memset(a->payload, 0, a->size);
    memset(b->payload, 1, b->size);
    TEST_ASSERT_FALSE_MESSAGE(compare_pal_message(a, b), "compare_pal_message should return false when comparing different messages");
}

void check_pal_message_compare_zero_size(void) {
    memset(a->payload, 0, a->size);
    memset(b->payload, 1, b->size);
    a->size = 0;
    b->size = 0;
    TEST_ASSERT_TRUE_MESSAGE(compare_pal_message(a, b), "compare_pal_message should return true when comparing zero size messages");
}

/*!
 * @}
 */

/*!
 * \defgroup            deep_copy Test deep copy helper function
 * @{
 */

void check_pal_handler_deep_copy_null_pointer(void) {
    TEST_ASSERT_NULL_MESSAGE(pal_handler_deep_copy(NULL, &harena), "pal_handler_deep_copy should return NULL when given a NULL pointer");
}

void check_pal_handler_deep_copy_null_arena(void) {
    TEST_ASSERT_NULL_MESSAGE(pal_handler_deep_copy(&base_handler, NULL), "pal_handler_deep_copy should return NULL when given a NULL arena");
}

void check_pal_handler_deep_copy_creates_exact_copy(void) {
    struct PalHandler *copy = pal_handler_deep_copy(&base_handler, &harena);

    TEST_ASSERT_NOT_NULL_MESSAGE(copy, "pal_handler_deep_copy should return a non-NULL pointer when given a valid handler and arena");
    TEST_ASSERT_TRUE_MESSAGE(pal_handler_deep_compare(&base_handler, copy), "pal_handler_deep_compare should return true when comparing the original and copied handlers");
}

void check_pal_handler_deep_copy_pointers_distinct(void) {
    struct PalHandler *copy = pal_handler_deep_copy(&base_handler, &harena);

    TEST_ASSERT_NOT_NULL_MESSAGE(copy, "pal_handler_deep_copy should return a non-NULL pointer when given a valid handler and arena");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(&base_handler, copy, "pal_handler_deep_copy should return a distinct pointer to the copied handler");

    TEST_ASSERT_NOT_EQUAL_MESSAGE(base_handler.rx_queue.data, copy->rx_queue.data, "rx_queue data should be distinct in the copied handler");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(base_handler.tx_queue.data, copy->tx_queue.data, "tx_queue data should be distinct in the copied handler");

    TEST_ASSERT_NOT_EQUAL_MESSAGE(base_handler.add_to_rx_message, copy->add_to_rx_message, "add_to_rx_message should be distinct in the copied handler");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(base_handler.add_to_tx_message, copy->add_to_tx_message, "add_to_tx_message should be distinct in the copied handler");

    TEST_ASSERT_NOT_EQUAL_MESSAGE(base_handler.process_rx_message, copy->process_rx_message, "process_rx_message should be distinct in the copied handler");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(base_handler.process_tx_message, copy->process_tx_message, "process_tx_message should be distinct in the copied handler");
}

/*!
 * @}
 */

/*!
 * \defgroup            deep_compare Test deep compare helper function
 * @{
 */

void check_pal_handler_deep_compare_null_pointer(void) {
    TEST_ASSERT_FALSE_MESSAGE(pal_handler_deep_compare(NULL, &base_handler), "pal_handler_deep_compare should return false when given a NULL pointer");
    TEST_ASSERT_FALSE_MESSAGE(pal_handler_deep_compare(&base_handler, NULL), "pal_handler_deep_compare should return false when given a NULL pointer");
}

void check_pal_handler_deep_compare_different_values_and_function_pointers(void) {
    struct PalHandler *copy = pal_handler_deep_copy(&base_handler, &harena);

    copy->max_message_size = 999;
    TEST_ASSERT_FALSE_MESSAGE(pal_handler_deep_compare(&base_handler, copy), "pal_handler_deep_compare should return false when comparing different message sizes");
    copy->max_message_size = base_handler.max_message_size;

    copy->deserialize = NULL;
    TEST_ASSERT_FALSE_MESSAGE(pal_handler_deep_compare(&base_handler, copy), "pal_handler_deep_compare should return false when comparing different deserialize function pointers");
    copy->deserialize = base_handler.deserialize;

    copy->send = NULL;
    TEST_ASSERT_FALSE_MESSAGE(pal_handler_deep_compare(&base_handler, copy), "pal_handler_deep_compare should return false when comparing different send function pointers");
    copy->send = base_handler.send;
}

void check_pal_handler_deep_compare_different_ring_buffer_data(void) {
    struct PalHandler *copy = pal_handler_deep_copy(&base_handler, &harena);

    // Check RX queue modification
    ((uint8_t *)copy->rx_queue.data)[0] ^= 0xFF;
    TEST_ASSERT_FALSE_MESSAGE(pal_handler_deep_compare(&base_handler, copy), "pal_handler_deep_compare should return false when comparing different ring buffer data");

    // Restore and check TX queue modification
    ((uint8_t *)copy->rx_queue.data)[0] ^= 0xFF;
    ((uint8_t *)copy->tx_queue.data)[0] ^= 0xFF;
    TEST_ASSERT_FALSE_MESSAGE(pal_handler_deep_compare(&base_handler, copy), "pal_handler_deep_compare should return false when comparing different ring buffer data");
}

void check_pal_handler_deep_compare_different_message_payload(void) {
    struct PalHandler *copy = pal_handler_deep_copy(&base_handler, &harena);

    // Check RX message payload
    copy->add_to_rx_message->payload[0] ^= 0x55;
    TEST_ASSERT_FALSE_MESSAGE(pal_handler_deep_compare(&base_handler, copy), "pal_handler_deep_compare should return false when comparing different message payloads");

    // Restore and check TX message payload
    copy->add_to_rx_message->payload[0] ^= 0x55;
    copy->add_to_tx_message->payload[0] ^= 0x55;
    TEST_ASSERT_FALSE_MESSAGE(pal_handler_deep_compare(&base_handler, copy), "pal_handler_deep_compare should return false when comparing different message payloads");

    copy->add_to_tx_message->payload[0] ^= 0x55;
    copy->process_rx_message->payload[0] ^= 0x55;
    TEST_ASSERT_FALSE_MESSAGE(pal_handler_deep_compare(&base_handler, copy), "pal_handler_deep_compare should return false when comparing different message payloads");

    copy->process_rx_message->payload[0] ^= 0x55;
    copy->process_tx_message->payload[0] ^= 0x55;
    TEST_ASSERT_FALSE_MESSAGE(pal_handler_deep_compare(&base_handler, copy), "pal_handler_deep_compare should return false when comparing different message payloads");
}

/*!
 * @}
 */
int main(void) {
    UNITY_BEGIN();

    /*!
     * \defgroup            deep_compare Test deep compare helper function
     * @{
     */
    RUN_TEST(check_copy_pal_message_buffer_null_source);
    RUN_TEST(check_copy_pal_message_buffer_valid_copy);
    /*!
     * @}
     */

    /*!
     * \defgroup            compare_pal_message Test deep copy helper function
     * @{
     */
    RUN_TEST(check_pal_message_compare_null_pointer);
    RUN_TEST(check_pal_message_compare_same_pointer);
    RUN_TEST(check_pal_message_compare_different_size);
    RUN_TEST(check_pal_message_compare_different_message);
    RUN_TEST(check_pal_message_compare_same_message);
    RUN_TEST(check_pal_message_compare_zero_size);
    /*!
     * @}
     */

    /*!
     * \defgroup            deep_copy Test deep copy helper function
     * @{
     */
    RUN_TEST(check_pal_handler_deep_copy_null_pointer);
    RUN_TEST(check_pal_handler_deep_copy_null_arena);
    RUN_TEST(check_pal_handler_deep_copy_creates_exact_copy);
    RUN_TEST(check_pal_handler_deep_copy_pointers_distinct);
    /*!
     * @}
     */

    /*!
     * \defgroup            deep_compare Test deep compare helper function
     * @{
     */
    RUN_TEST(check_pal_handler_deep_compare_null_pointer);
    RUN_TEST(check_pal_handler_deep_compare_different_values_and_function_pointers);
    RUN_TEST(check_pal_handler_deep_compare_different_ring_buffer_data);
    RUN_TEST(check_pal_handler_deep_compare_different_message_payload);
    /*!
     * @}
     */

    return UNITY_END();
}
