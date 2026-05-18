/*!
 * \file            test-pal-api.c
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
#include "unity.h"
#include "fff.h"
#include "utils.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TX_CAPACITY (1U)
#define RX_CAPACITY (1U)
#define MESSAGE_MAX_SIZE (100U)
#define MESSAGE_FILENAME ("message.txt")

DEFINE_FFF_GLOBALS

struct ArenaAllocatorHandler harena;
struct PalHandler hpal;
char out_msg_buff[MESSAGE_MAX_SIZE] = { 0 };

FAKE_VALUE_FUNC(enum PalReturnCode, deserialize, const struct PalMessage *, void *)
FAKE_VALUE_FUNC(enum PalReturnCode, send, const struct PalMessage *)
FAKE_VOID_FUNC(cs_enter)
FAKE_VOID_FUNC(cs_exit)

enum PalReturnCode send_global_buff(const struct PalMessage *msg) {
    if (msg == NULL)
        return PAL_RC_NULL_POINTER;
    memcpy(out_msg_buff, msg->payload, msg->size);
    return PAL_RC_OK;
}

void setUp() {

    arena_allocator_api_init(&harena);
    send_fake.return_val = PAL_RC_OK;
    pal_api_init(&hpal, RX_CAPACITY, TX_CAPACITY, MESSAGE_MAX_SIZE, NULL, send, NULL, NULL, &harena);
}

void tearDown() {
    arena_allocator_api_free(&harena);
    RESET_FAKE(send);
    RESET_FAKE(deserialize);
}

/*!
 * \defgroup            pal_init Test P.A.L. init
 * @{
 */

void check_pal_api_init_null_pal_handler(void) {
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_NULL_POINTER, pal_api_init(NULL, RX_CAPACITY, TX_CAPACITY, MESSAGE_MAX_SIZE, NULL, send, NULL, NULL, &harena), "Didn't return null pointer error");
}

void check_pal_api_init_message_size_zero(void) {
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_INVALID_ARGUMENT, pal_api_init(&hpal, RX_CAPACITY, TX_CAPACITY, 0, NULL, send, NULL, NULL, &harena), "Didn't return invalid argument error");
}

void check_pal_api_init_null_arena_handler(void) {
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_NULL_POINTER, pal_api_init(&hpal, RX_CAPACITY, TX_CAPACITY, MESSAGE_MAX_SIZE, NULL, send, NULL, NULL, NULL), "Didn't return null pointer error");
}

void check_pal_api_init_null_send_function(void) {
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_NULL_POINTER, pal_api_init(&hpal, RX_CAPACITY, TX_CAPACITY, MESSAGE_MAX_SIZE, NULL, NULL, NULL, NULL, &harena), "Didn't return null pointer error");
}

void check_pal_api_init_ok(void) {
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_OK, pal_api_init(&hpal, RX_CAPACITY, TX_CAPACITY, MESSAGE_MAX_SIZE, deserialize, send, cs_enter, cs_exit, &harena), "pal_api_init should return PAL_RC_OK");
    TEST_ASSERT_EQUAL_MESSAGE(hpal.rx_queue.capacity, RX_CAPACITY, "rx_queue capacity should match RX_CAPACITY");
    TEST_ASSERT_EQUAL_MESSAGE(hpal.tx_queue.capacity, TX_CAPACITY, "tx_queue capacity should match TX_CAPACITY");
    TEST_ASSERT_EQUAL_MESSAGE(hpal.max_message_size, MESSAGE_MAX_SIZE, "max_message_size should match MESSAGE_MAX_SIZE");
    TEST_ASSERT_EQUAL_MESSAGE(hpal.deserialize, deserialize, "deserialize function should match");
    TEST_ASSERT_EQUAL_MESSAGE(hpal.send, send, "send function should match");
    TEST_ASSERT_EQUAL_MESSAGE(hpal.rx_queue.cs_enter, cs_enter, "rx_queue cs_enter function should match");
    TEST_ASSERT_EQUAL_MESSAGE(hpal.rx_queue.cs_exit, cs_exit, "rx_queue cs_exit function should match");
}

/*!
 * @}
 */

/*!
 * \defgroup            pal_add_to_rx_queue Test P.A.L. add_to_rx_queue
 * @{
 */

void check_pal_api_add_to_rx_queue_null_pal_handler(void) {
    uint8_t a = 1;
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_NULL_POINTER, pal_api_add_to_rx_queue(NULL, &a, MESSAGE_MAX_SIZE), "Didn't return a null pointer error");
}

void check_pal_api_add_to_rx_queue_null_data(void) {
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_NULL_POINTER, pal_api_add_to_rx_queue(&hpal, NULL, 0U), "Didn't return a null pointer error");
}

void check_pal_api_add_to_rx_queue_message_size_zero(void) {
    uint8_t a = 1;
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_INVALID_ARGUMENT, pal_api_add_to_rx_queue(&hpal, &a, 0U), "Didn't return invalid argument error");
}

void check_pal_api_add_to_rx_queue_message_size_too_big(void) {
    uint8_t buff[MESSAGE_MAX_SIZE];
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_MESSAGE_TOO_BIG, pal_api_add_to_rx_queue(&hpal, buff, MESSAGE_MAX_SIZE + 1U), "Didn't return message too big error");
}

void check_pal_api_add_to_rx_queue_full_rx_queue(void) {
    // Relies on RX_CAPACITY being 1
    uint8_t a = 1;
    uint8_t b = 2;

    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_OK, pal_api_add_to_rx_queue(&hpal, &a, sizeof(uint8_t)), "Incorrectly reporting rx queue as full");
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_QUEUE_FULL, pal_api_add_to_rx_queue(&hpal, &b, sizeof(uint8_t)), "Adding to rx queue didn't return queue full error");
}

void check_pal_api_add_to_rx_queue_ok(void) {
    uint8_t buff[MESSAGE_MAX_SIZE];
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_OK, pal_api_add_to_rx_queue(&hpal, buff, MESSAGE_MAX_SIZE), "Adding to queue failed");
    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(buff, hpal.add_to_rx_message->payload, MESSAGE_MAX_SIZE, "buffer is different inside handler, something corrupted the data when adding to the queue");
}

/*!
 * @}
 */

/*!
 * \defgroup            pal_add_to_tx_queue Test P.A.L. add_to_tx_queue
 * @{
 */

void check_pal_api_add_to_tx_queue_null_pal_handler(void) {
    uint8_t a = 1;
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_NULL_POINTER, pal_api_add_to_tx_queue(NULL, &a, sizeof(uint8_t)), "Didn't return null pointer error");
}

void check_pal_api_add_to_tx_queue_null_data(void) {
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_NULL_POINTER, pal_api_add_to_tx_queue(&hpal, NULL, 42U), "Didn't return null pointer error");
}

void check_pal_api_add_to_tx_queue_message_size_zero(void) {
    uint8_t a = 1;
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_INVALID_ARGUMENT, pal_api_add_to_tx_queue(&hpal, &a, 0U), "Didn't return invalid argument error");
}

void check_pal_api_add_to_tx_queue_message_size_too_big(void) {
    uint8_t buff[MESSAGE_MAX_SIZE];
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_MESSAGE_TOO_BIG, pal_api_add_to_tx_queue(&hpal, buff, MESSAGE_MAX_SIZE + 1U), "Didn't return message to big error");
}

void check_pal_api_add_to_tx_queue_full_tx_queue(void) {
    // Relies on TX_CAPACITY being 1
    uint8_t a = 1;
    uint8_t b = 2;

    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_OK, pal_api_add_to_tx_queue(&hpal, &a, sizeof(uint8_t)), "Incorrectly reporting tx queue as full");
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_QUEUE_FULL, pal_api_add_to_tx_queue(&hpal, &b, sizeof(uint8_t)), "Adding to tx queue didn't return queue full error");
}

void check_pal_api_add_to_tx_queue_ok(void) {
    uint8_t buff[MESSAGE_MAX_SIZE];
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_OK, pal_api_add_to_tx_queue(&hpal, buff, MESSAGE_MAX_SIZE), "Adding to queue failed");
    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(buff, hpal.add_to_tx_message->payload, MESSAGE_MAX_SIZE, "buffer is different inside handler, something corrupted the data when adding to the queue");
}

/*!
 * @}
 */

/*!
 * \defgroup            pal_process_rx Test P.A.L. process_rx
 * @{
 */

void check_pal_api_process_rx_null_pal_handler(void) {
    uint32_t test = 1;
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_POINTER, pal_api_process_rx(NULL, &test));
}

void check_pal_api_process_rx_null_out(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_POINTER, pal_api_process_rx(&hpal, NULL));
}

void check_pal_api_process_rx_queue_empty(void) {
    uint32_t test = 1;
    TEST_ASSERT_EQUAL_INT(PAL_RC_QUEUE_EMPTY, pal_api_process_rx(&hpal, &test));
}

void check_pal_api_process_rx_deserialize_error(void) {
    uint8_t a = 1;
    uint32_t test = 1;

    deserialize_fake.return_val = PAL_RC_DESERIALIZATION_ERROR;
    hpal.deserialize = deserialize;

    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_OK, pal_api_add_to_rx_queue(&hpal, &a, MESSAGE_MAX_SIZE), "Something went wrong when adding to the rx_queue");
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_DESERIALIZATION_ERROR, pal_api_process_rx(&hpal, &test), "Ignored deserialize error");
}

/*!
 * @}
 */

/*!
 * \defgroup            pal_process_tx Test P.A.L. process_tx
 * @{
 */

void check_pal_api_process_tx_null_pal_handler(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_POINTER, pal_api_process_tx(NULL));
}

void check_pal_api_process_tx_queue_empty(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_QUEUE_EMPTY, pal_api_process_tx(&hpal));
}

void check_pal_api_process_tx_serialize_error(void) {
    uint8_t a = 9;

    send_fake.return_val = PAL_RC_SERIALIZATION_ERROR;

    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_OK, pal_api_add_to_tx_queue(&hpal, &a, sizeof(uint8_t)), "Failed to add message to queue");
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_SERIALIZATION_ERROR, pal_api_process_tx(&hpal), "Serialization succeeded but serialization should fail");
}

void check_pal_api_process_tx_send_error(void) {
    uint8_t a = 9;

    send_fake.return_val = PAL_RC_IO_ERROR;

    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_OK, pal_api_add_to_tx_queue(&hpal, &a, sizeof(uint8_t)), "Failed to add message to queue");
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_IO_ERROR, pal_api_process_tx(&hpal), "Send succeeded but send should fail");
}

void check_pal_api_process_tx_send_ok(void) {
    uint8_t a = 9;

    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_OK, pal_api_add_to_tx_queue(&hpal, &a, sizeof(uint8_t)), "Failed to add message to queue");
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_OK, pal_api_process_tx(&hpal), "Send failed but send should succeed");
}

/*!
 * @}
 */

/*!
 * \defgroup            Test P.A.L. message reception
 * @{
 */
//TODO: move to a file specific for functional testing
void check_pal_api_message_reception(void) {
    uint8_t in_msg[] = {
        'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', '\0'
    };
    uint32_t size = 14;

    char out_msg[MESSAGE_MAX_SIZE];
    pal_api_add_to_rx_queue(&hpal, in_msg, size);
    pal_api_process_rx(&hpal, out_msg);
    TEST_ASSERT_EQUAL_STRING(in_msg, out_msg);
}

/*!
 * @}
 */

/*!
 * \defgroup            Test P.A.L. message transmission
 * @{
 */
//TODO: move to a file specific for functional testing
void check_pal_api_message_transmission(void) {
    pal_api_init(&hpal, RX_CAPACITY, TX_CAPACITY, MESSAGE_MAX_SIZE, NULL, send_global_buff, NULL, NULL, &harena);
    uint8_t in_msg[] = {
        'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', '\0'
    };
    uint32_t size = 14;
    pal_api_add_to_tx_queue(&hpal, in_msg, size);
    pal_api_process_tx(&hpal);
    TEST_ASSERT_EQUAL_STRING(in_msg, out_msg_buff);
}

/*!
 * @}
 */

int main(void) {
    UNITY_BEGIN();

    /*!
     * \defgroup            pal_init Test P.A.L. init
     * @{
     */

    RUN_TEST(check_pal_api_init_null_pal_handler);
    RUN_TEST(check_pal_api_init_null_arena_handler);
    RUN_TEST(check_pal_api_init_null_send_function);
    RUN_TEST(check_pal_api_init_message_size_zero);
    RUN_TEST(check_pal_api_init_ok);

    /*!
     * @}
     */

    /*!
     * \defgroup            pal_add_to_rx_queue Test P.A.L. add_to_rx_queue
     * @{
     */

    RUN_TEST(check_pal_api_add_to_rx_queue_null_pal_handler);
    RUN_TEST(check_pal_api_add_to_rx_queue_null_data);
    RUN_TEST(check_pal_api_add_to_rx_queue_message_size_zero);
    RUN_TEST(check_pal_api_add_to_rx_queue_message_size_too_big);
    RUN_TEST(check_pal_api_add_to_rx_queue_full_rx_queue);
    RUN_TEST(check_pal_api_add_to_rx_queue_ok);

    /*!
     * @}
     */

    /*!
     * \defgroup            pal_add_to_tx_queue Test P.A.L. add_to_tx_queue
     * @{
     */

    RUN_TEST(check_pal_api_add_to_tx_queue_null_pal_handler);
    RUN_TEST(check_pal_api_add_to_tx_queue_null_data);
    RUN_TEST(check_pal_api_add_to_tx_queue_message_size_zero);
    RUN_TEST(check_pal_api_add_to_tx_queue_message_size_too_big);
    RUN_TEST(check_pal_api_add_to_tx_queue_full_tx_queue);
    RUN_TEST(check_pal_api_add_to_tx_queue_ok);

    /*!
     * @}
     */

    /*!
     * \defgroup            pal_process_rx Test P.A.L. process_rx
     * @{
     */

    RUN_TEST(check_pal_api_process_rx_null_pal_handler);
    RUN_TEST(check_pal_api_process_rx_null_out);
    RUN_TEST(check_pal_api_process_rx_queue_empty);
    RUN_TEST(check_pal_api_process_rx_deserialize_error);
    /*!
     * @}
     */

    /*!
     * \defgroup            pal_process_tx Test P.A.L. process_tx
     * @{
     */

    RUN_TEST(check_pal_api_process_tx_null_pal_handler);
    RUN_TEST(check_pal_api_process_tx_queue_empty);
    RUN_TEST(check_pal_api_process_tx_serialize_error);
    RUN_TEST(check_pal_api_process_tx_send_error);
    RUN_TEST(check_pal_api_process_tx_send_ok);

    /*!
     * @}
     */

    /*!
     * \defgroup            Test P.A.L. message reception
     * @{
     */
    RUN_TEST(check_pal_api_message_reception);
    /*!
     * @}
     */

    /*!
     * \defgroup            Test P.A.L. message transmission
     * @{
     */

    RUN_TEST(check_pal_api_message_transmission);

    /*!
     * @}
     */
    UNITY_END();
}
