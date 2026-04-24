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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TX_CAPACITY (1U)
#define RX_CAPACITY (1U)
#define MSG_MAX_SIZE (100U)
#define MSG_FILENAME ("message.txt")

struct Point {
    float x, y;
};

struct ArenaAllocatorHandler harena;
struct PalHandler hpal;
char out_msg_buff[MSG_MAX_SIZE];

enum PalReturnCode deserialize_error(const struct PalMessage *in, void *out) {
    (void)in;
    (void)out;
    return PAL_RC_DESERIALIZATION_ERR;
}

enum PalReturnCode send_default(const struct PalMessage *msg) {
    if (msg == NULL)
        return PAL_RC_NULL_PTR;

    (void)msg;

    return PAL_RC_OK;
}

enum PalReturnCode send_error(const struct PalMessage *msg) {
    if (msg == NULL)
        return PAL_RC_NULL_PTR;

    (void)msg;

    return PAL_RC_IO_ERR;
}

enum PalReturnCode serialize_error(const struct PalMessage *msg) {
    (void)msg;
    return PAL_RC_SERIALIZATION_ERR;
}

enum PalReturnCode send_global_buff(const struct PalMessage *msg) {
    if (msg == NULL)
        return PAL_RC_NULL_PTR;
    memcpy(out_msg_buff, msg->data, msg->size);
    return PAL_RC_OK;
}

void setUp() {
    arena_allocator_api_init(&harena);
    pal_api_init(&hpal, RX_CAPACITY, TX_CAPACITY, MSG_MAX_SIZE, NULL, send_default, NULL, NULL, &harena);
}

void tearDown() {
    arena_allocator_api_free(&harena);
}

/*!
 * \defgroup            pal_init Test P.A.L. init
 * @{
 */

void check_pal_api_init_null_pal_handler(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_init(NULL, RX_CAPACITY, TX_CAPACITY, MSG_MAX_SIZE, NULL, send_default, NULL, NULL, &harena));
}

void check_pal_api_init_message_size_zero(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_INVALID_PARAM, pal_api_init(&hpal, RX_CAPACITY, TX_CAPACITY, 0, NULL, send_default, NULL, NULL, &harena));
}

void check_pal_api_init_null_arena_handler(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_init(&hpal, RX_CAPACITY, TX_CAPACITY, MSG_MAX_SIZE, NULL, send_default, NULL, NULL, NULL));
}

void check_pal_api_init_null_send_function(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_init(&hpal, RX_CAPACITY, TX_CAPACITY, MSG_MAX_SIZE, NULL, NULL, NULL, NULL, &harena));
}

void check_pal_api_init_ok(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_OK, pal_api_init(&hpal, RX_CAPACITY, TX_CAPACITY, MSG_MAX_SIZE, NULL, send_default, NULL, NULL, &harena));
}

/*!
 * @}
 */

/*!
 * \defgroup            pal_add_to_rx_queue Test P.A.L. add_to_rx_queue
 * @{
 */

void check_pal_api_add_to_rx_queue_null_pal_handler(void) {
    uint8_t buff[MSG_MAX_SIZE];
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_add_to_rx_queue(NULL, buff, MSG_MAX_SIZE));
}

void check_pal_api_add_to_rx_queue_null_data(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_add_to_rx_queue(&hpal, NULL, 0U));
}

void check_pal_api_add_to_rx_queue_message_size_zero(void) {
    uint8_t buff[MSG_MAX_SIZE];
    TEST_ASSERT_EQUAL_INT(PAL_RC_INVALID_PARAM, pal_api_add_to_rx_queue(&hpal, buff, 0U));
}

void check_pal_api_add_to_rx_queue_message_size_too_big(void) {
    uint8_t buff[MSG_MAX_SIZE];
    TEST_ASSERT_EQUAL_INT(PAL_RC_MSG_TOO_BIG, pal_api_add_to_rx_queue(&hpal, buff, MSG_MAX_SIZE + 1U));
}

void check_pal_api_add_to_rx_queue_full_rx_queue(void) {
    uint8_t buff1[MSG_MAX_SIZE];
    uint8_t buff2[MSG_MAX_SIZE];

    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_OK, pal_api_add_to_rx_queue(&hpal, buff1, MSG_MAX_SIZE), "Incorrectly reporting rx queue as full");
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_QUEUE_FULL, pal_api_add_to_rx_queue(&hpal, buff2, MSG_MAX_SIZE), "Adding to rx queue didn't return queue full error");
}

void check_pal_api_add_to_rx_queue_ok(void) {
    uint8_t buff[MSG_MAX_SIZE];
    TEST_ASSERT_EQUAL_INT(PAL_RC_OK, pal_api_add_to_rx_queue(&hpal, buff, MSG_MAX_SIZE));
}

/*!
 * @}
 */

/*!
 * \defgroup            pal_add_to_tx_queue Test P.A.L. add_to_tx_queue
 * @{
 */

void check_pal_api_add_to_tx_queue_null_pal_handler(void) {
    uint8_t a = 0;
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_add_to_tx_queue(NULL, &a, sizeof(uint8_t)));
}

void check_pal_api_add_to_tx_queue_null_data(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_add_to_tx_queue(&hpal, NULL, 42U));
}

void check_pal_api_add_to_tx_queue_message_size_zero(void) {
    uint8_t a = 0;
    TEST_ASSERT_EQUAL_INT(PAL_RC_INVALID_PARAM, pal_api_add_to_tx_queue(&hpal, &a, 0U));
}

void check_pal_api_add_to_tx_queue_message_size_too_big(void) {
    uint8_t a = 0;
    TEST_ASSERT_EQUAL_INT(PAL_RC_MSG_TOO_BIG, pal_api_add_to_tx_queue(&hpal, &a, MSG_MAX_SIZE + 1U));
}

void check_pal_api_add_to_tx_queue_full_tx_queue(void) {
    // Relies on TX_CAPACITY being 1
    uint8_t a = 1;
    uint8_t b = 2;

    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_OK, pal_api_add_to_tx_queue(&hpal, &a, sizeof(uint8_t)), "Incorrectly reporting tx queue as full");
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_QUEUE_FULL, pal_api_add_to_tx_queue(&hpal, &b, sizeof(uint8_t)), "Adding to tx queue didn't return queue full error");
}

void check_pal_api_add_to_tx_queue_ok(void) {
    uint8_t a = 9;
    TEST_ASSERT_EQUAL_INT(PAL_RC_OK, pal_api_add_to_tx_queue(&hpal, &a, sizeof(uint8_t)));
}

/*!
 * @}
 */

/*!
 * \defgroup            pal_exec_rx Test P.A.L. exec_rx
 * @{
 */

void check_pal_api_exec_rx_null_pal_handler(void) {
    struct Point point = { 0 };
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_exec_rx(NULL, &point));
}

void check_pal_api_exec_rx_null_out(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_exec_rx(&hpal, NULL));
}

void check_pal_api_exec_rx_queue_empty(void) {
    struct Point point = { 0 };
    TEST_ASSERT_EQUAL_INT(PAL_RC_QUEUE_EMPTY, pal_api_exec_rx(&hpal, &point));
}

void check_pal_api_exec_rx_deserialize_error(void) {
    uint8_t buff[MSG_MAX_SIZE];
    struct Point point = { 0 };

    hpal.deserialize = deserialize_error;

    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_OK, pal_api_add_to_rx_queue(&hpal, buff, MSG_MAX_SIZE), "Something went wrong when adding to the rx_queue");
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_DESERIALIZATION_ERR, pal_api_exec_rx(&hpal, &point), "Ignored deserialize error");
}

/*!
 * @}
 */

/*!
 * \defgroup            pal_exec_tx Test P.A.L. exec_tx
 * @{
 */

void check_pal_api_exec_tx_null_pal_handler(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_exec_tx(NULL));
}

void check_pal_api_exec_tx_queue_empty(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_QUEUE_EMPTY, pal_api_exec_tx(&hpal));
}

void check_pal_api_exec_tx_serialize_error(void) {
    uint8_t a = 9;

    hpal.send = serialize_error;

    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_OK, pal_api_add_to_tx_queue(&hpal, &a, sizeof(uint8_t)), "Failed to add message to queue");
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_SERIALIZATION_ERR, pal_api_exec_tx(&hpal), "Serialization succeeded but serialization should fail");
}

void check_pal_api_exec_tx_send_error(void) {
    uint8_t a = 9;

    hpal.send = send_error;

    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_OK, pal_api_add_to_tx_queue(&hpal, &a, sizeof(uint8_t)), "Failed to add message to queue");
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_IO_ERR, pal_api_exec_tx(&hpal), "Send succeeded but send should fail");
}

void check_pal_api_exec_tx_send_ok(void) {
    uint8_t a = 9;

    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_OK, pal_api_add_to_tx_queue(&hpal, &a, sizeof(uint8_t)), "Failed to add message to queue");
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_OK, pal_api_exec_tx(&hpal), "Send failed but send should succeed");
}

/*!
 * @}
 */

/*!
 * \defgroup            Test P.A.L. message reception
 * @{
 */
void check_pal_api_message_reception(void) {
    uint8_t in_msg[] = {
        'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', '\0'
    };
    uint32_t size = 14;

    char out_msg[MSG_MAX_SIZE];
    pal_api_add_to_rx_queue(&hpal, in_msg, size);
    pal_api_exec_rx(&hpal, out_msg);
    TEST_ASSERT_EQUAL_STRING(in_msg, out_msg);
}

/*!
 * @}
 */

/*!
 * \defgroup            Test P.A.L. message transmission
 * @{
 */
void check_pal_api_message_transmission(void) {
    pal_api_init(&hpal, RX_CAPACITY, TX_CAPACITY, MSG_MAX_SIZE, NULL, send_global_buff, NULL, NULL, &harena);
    uint8_t in_msg[] = {
        'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', '\0'
    };
    uint32_t size = 14;
    pal_api_add_to_tx_queue(&hpal, in_msg, size);
    pal_api_exec_tx(&hpal);
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
     * \defgroup            pal_exec_rx Test P.A.L. exec_rx
     * @{
     */

    RUN_TEST(check_pal_api_exec_rx_null_pal_handler);
    RUN_TEST(check_pal_api_exec_rx_null_out);
    RUN_TEST(check_pal_api_exec_rx_queue_empty);
    RUN_TEST(check_pal_api_exec_rx_deserialize_error);
    /*!
     * @}
     */

    /*!
     * \defgroup            pal_exec_tx Test P.A.L. exec_tx
     * @{
     */

    RUN_TEST(check_pal_api_exec_tx_null_pal_handler);
    RUN_TEST(check_pal_api_exec_tx_queue_empty);
    RUN_TEST(check_pal_api_exec_tx_serialize_error);
    RUN_TEST(check_pal_api_exec_tx_send_error);
    RUN_TEST(check_pal_api_exec_tx_send_ok);

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
