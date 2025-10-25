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

ArenaAllocatorHandler_t harena;
struct PalHandler hpal;

enum PalReturnCode send_default(const uint8_t *buff, size_t size) {
    if (!buff)
        return PAL_RC_NULL_PTR;

    (void)buff;
    (void)size;

    return PAL_RC_OK;
}

enum PalReturnCode send_error(const uint8_t *buff, size_t size) {
    if (!buff)
        return PAL_RC_NULL_PTR;

    (void)buff;
    (void)size;

    return PAL_RC_IO_ERR;
}

enum PalReturnCode serialize_default(const void *in, uint8_t *out, size_t size) {
    if (!in || !out)
        return -1;

    (void)in;
    (void)size;
    (void)out;

    return PAL_RC_OK;
}

enum PalReturnCode serialize_error(const void *in, uint8_t *out, size_t size) {
    (void)in;
    (void)size;
    (void)out;
    return PAL_RC_SER_ERR;
}

void setUp() {
    arena_allocator_api_init(&harena);
    pal_api_init(&hpal, TX_CAPACITY, send_default, serialize_default, NULL, NULL, &harena);
}

void tearDown() {
    arena_allocator_api_free(&harena);
}

/*!
 * \defgroup            pal_init Test P.A.L. init
 * @{
 */

void check_pal_api_init_null_pal_handler(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_init(NULL, TX_CAPACITY, send_default, serialize_default, NULL, NULL, &harena));
}

void check_pal_api_init_null_arena_handler(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_init(&hpal, TX_CAPACITY, send_default, serialize_default, NULL, NULL, NULL));
}

void check_pal_api_init_null_send_function(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_init(&hpal, TX_CAPACITY, NULL, serialize_default, NULL, NULL, &harena));
}

void check_pal_api_init_null_serialize_function(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_init(&hpal, TX_CAPACITY, send_default, NULL, NULL, NULL, &harena));
}

void check_pal_api_init_ok(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_OK, pal_api_init(&hpal, TX_CAPACITY, send_default, serialize_default, NULL, NULL, &harena));
}

/*!
 * @}
 */

/*!
 * \defgroup            pal_add_to_tx_queue Test P.A.L. add_to_tx_queue
 * @{
 */

void check_pal_api_add_to_tx_queue_null_pal_handler(void) {
    int a = 0;
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_add_to_tx_queue(NULL, &a));
}

void check_pal_api_add_to_tx_queue_null_data(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_add_to_tx_queue(&hpal, NULL));
}

void check_pal_api_add_to_tx_queue_full_tx_queue(void) {
    int a = 1;
    int b = 2;

    TEST_ASSERT_EQUAL_INT(PAL_RC_OK, pal_api_add_to_tx_queue(&hpal, &a));
    TEST_ASSERT_EQUAL_INT(PAL_RC_QUEUE_FULL, pal_api_add_to_tx_queue(&hpal, &b));
}

void check_pal_api_add_to_tx_queue_ok(void) {
    int a = 9;
    TEST_ASSERT_EQUAL_INT(PAL_RC_OK, pal_api_add_to_tx_queue(&hpal, &a));
}

/*!
 * @}
 */

/*!
 * \defgroup            pal_exec_tx Test P.A.L. exec_tx
 * @{
 */

void check_pal_api_exec_tx_null_pal_handler(void) {
    uint8_t buff[69U];
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_exec_tx(NULL, buff, sizeof(buff)));
}

void check_pal_api_exec_tx_null_buffer(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_exec_tx(&hpal, NULL, 69U));
}

void check_pal_api_exec_tx_queue_empty(void) {
    uint8_t buff[16];
    TEST_ASSERT_EQUAL_INT(PAL_RC_QUEUE_EMPTY, pal_api_exec_tx(&hpal, buff, sizeof(buff)));
}

void check_pal_api_exec_tx_serialize_error(void) {
    int a = 9;
    uint8_t buff[69U];

    hpal.serialize = serialize_error;
    hpal.send = send_default;

    TEST_ASSERT_EQUAL_INT(PAL_RC_OK, pal_api_add_to_tx_queue(&hpal, &a));
    TEST_ASSERT_EQUAL_INT(PAL_RC_IO_ERR, pal_api_exec_tx(&hpal, buff, sizeof(buff)));
}

void check_pal_api_exec_tx_send_error(void) {
    int a = 9;
    uint8_t buff[69U];

    hpal.serialize = serialize_default;
    hpal.send = send_error;

    TEST_ASSERT_EQUAL_INT(PAL_RC_OK, pal_api_add_to_tx_queue(&hpal, &a));
    TEST_ASSERT_EQUAL_INT(PAL_RC_IO_ERR, pal_api_exec_tx(&hpal, buff, sizeof(buff)));
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
    RUN_TEST(check_pal_api_init_null_serialize_function);

    /*!
     * @}
     */

    /*!
     * \defgroup            pal_add_to_tx_queue Test P.A.L. add_to_tx_queue
     * @{
     */

    RUN_TEST(check_pal_api_add_to_tx_queue_null_pal_handler);
    RUN_TEST(check_pal_api_add_to_tx_queue_null_data);
    RUN_TEST(check_pal_api_add_to_tx_queue_full_tx_queue);
    RUN_TEST(check_pal_api_add_to_tx_queue_ok);

    /*!
     * @}
     */

    /*!
     * \defgroup            pal_exec_tx Test P.A.L. exec_tx
     * @{
     */

    RUN_TEST(check_pal_api_exec_tx_null_pal_handler);
    RUN_TEST(check_pal_api_exec_tx_null_buffer);
    RUN_TEST(check_pal_api_exec_tx_queue_empty);
    RUN_TEST(check_pal_api_exec_tx_serialize_error);
    RUN_TEST(check_pal_api_exec_tx_send_error);

    /*!
     * @}
     */

    UNITY_END();
}
