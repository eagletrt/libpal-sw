/*!
 * \file            test-pal-api.c
 * \date            2025-10-24
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

#define TX_CAPACITY (1U)

struct Point {
    float x, y;
};

ArenaAllocatorHandler_t harena;
struct PalHandler hpal;

enum PalReturnCode deserialize_default(const struct PalMessage *in, void *out) {
    if (!in || !out)
        return -1;

    (void)in;
    return 0;
}

enum PalReturnCode deserialize_error(const struct PalMessage *in, void *out) {
    (void)in;
    (void)out;
    return PAL_RC_DESER_ERR;
}

void setUp() {
    arena_allocator_api_init(&harena);
    pal_api_init(&hpal, TX_CAPACITY, deserialize_default, NULL, NULL, &harena);
}

void tearDown() {
    arena_allocator_api_free(&harena);
}

/*!
 * \defgroup            pal_init Test P.A.L. init
 * @{
 */

void check_pal_api_init_null_pal_handler(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_init(NULL, TX_CAPACITY, deserialize_default, NULL, NULL, &harena));
}

void check_pal_api_init_null_arena_handler(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_init(&hpal, TX_CAPACITY, deserialize_default, NULL, NULL, NULL));
}

void check_pal_api_init_null_deserialize_function(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_init(&hpal, TX_CAPACITY, NULL, NULL, NULL, &harena));
}

void check_pal_api_init_ok(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_OK, pal_api_init(&hpal, TX_CAPACITY, deserialize_default, NULL, NULL, &harena));
}

/*!
 * @}
 */

/*!
 * \defgroup            pal_add_to_rx_queue Test P.A.L. add_to_rx_queue
 * @{
 */

void check_pal_api_add_to_rx_queue_null_pal_handler(void) {
    uint8_t buff[69U];
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_add_to_rx_queue(NULL, buff, 69U));
}

void check_pal_api_add_to_rx_queue_null_data(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_add_to_rx_queue(&hpal, NULL, 0U));
}

void check_pal_api_add_to_rx_queue_full_rx_queue(void) {
    uint8_t buff1[69U];
    uint8_t buff2[96U];

    TEST_ASSERT_EQUAL_INT(PAL_RC_OK, pal_api_add_to_rx_queue(&hpal, buff1, 69U));
    TEST_ASSERT_EQUAL_INT(PAL_RC_QUEUE_FULL, pal_api_add_to_rx_queue(&hpal, buff2, 96U));
}

void check_pal_api_add_to_rx_queue_ok(void) {
    uint8_t buff[69U];
    TEST_ASSERT_EQUAL_INT(PAL_RC_OK, pal_api_add_to_rx_queue(&hpal, buff, 69U));
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
    uint8_t buff[69U];
    struct Point point = { 0 };

    hpal.deserialize = deserialize_error;

    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_OK, pal_api_add_to_rx_queue(&hpal, buff, 69U),"Something went wrong when adding to the rx_queue");
    TEST_ASSERT_EQUAL_INT_MESSAGE(PAL_RC_DESER_ERR, pal_api_exec_rx(&hpal, &point),"Something went wrong when deserializing the message");
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
    RUN_TEST(check_pal_api_init_null_deserialize_function);

    /*!
     * @}
     */

    /*!
     * \defgroup            pal_add_to_rx_queue Test P.A.L. add_to_rx_queue
     * @{
     */

    RUN_TEST(check_pal_api_add_to_rx_queue_null_pal_handler);
    RUN_TEST(check_pal_api_add_to_rx_queue_null_data);
    RUN_TEST(check_pal_api_add_to_rx_queue_full_rx_queue);
    RUN_TEST(check_pal_api_add_to_rx_queue_ok);

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

    UNITY_END();
}
