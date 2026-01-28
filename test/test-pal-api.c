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
#include <stdio.h>
#include <string.h>

#define TX_CAPACITY (1U)
#define MAX_MSG_SIZE (100U)
#define MSG_FILENAME ("message.txt")

ArenaAllocatorHandler_t harena;
struct PalHandler hpal;

char out_msg_buff[MAX_MSG_SIZE];

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

enum PalReturnCode send_global_buff(const struct PalMessage *msg){
    if (msg == NULL)
        return PAL_RC_NULL_PTR;
    memcpy(out_msg_buff,msg->data,msg->size);
    return PAL_RC_OK;
}

void setUp() {
    arena_allocator_api_init(&harena);
    pal_api_init(&hpal, TX_CAPACITY,MAX_MSG_SIZE, send_default, NULL, NULL, &harena);
}

void tearDown() {
    arena_allocator_api_free(&harena);
}

/*!
 * \defgroup            pal_init Test P.A.L. init
 * @{
 */

void check_pal_api_init_null_pal_handler(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_init(NULL, TX_CAPACITY,MAX_MSG_SIZE, send_default, NULL, NULL, &harena));
}

void check_pal_api_init_null_arena_handler(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_init(&hpal, TX_CAPACITY,MAX_MSG_SIZE, send_default, NULL, NULL, NULL));
}

void check_pal_api_init_null_send_function(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_init(&hpal, TX_CAPACITY,MAX_MSG_SIZE, NULL, NULL, NULL, &harena));
}

void check_pal_api_init_message_size_zero(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_INVALID_PARAM, pal_api_init(&hpal, TX_CAPACITY,0, send_default,NULL, NULL, &harena));
}

void check_pal_api_init_ok(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_OK, pal_api_init(&hpal, TX_CAPACITY,MAX_MSG_SIZE, send_default, NULL, NULL, &harena));
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
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_add_to_tx_queue(NULL, &a,sizeof(uint8_t)));
}

void check_pal_api_add_to_tx_queue_null_data(void) {
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_add_to_tx_queue(&hpal, NULL,42U));
}

void check_pal_api_add_to_tx_queue_message_size_zero(void) {
    uint8_t a = 0;
    TEST_ASSERT_EQUAL_INT(PAL_RC_NULL_PTR, pal_api_add_to_tx_queue(NULL, &a,0U));
}

void check_pal_api_add_to_tx_queue_message_size_too_big(void) {
    uint8_t a = 0;
    TEST_ASSERT_EQUAL_INT(PAL_RC_MSG_TOO_BIG, pal_api_add_to_tx_queue(&hpal, &a,MAX_MSG_SIZE+1U));
}

void check_pal_api_add_to_tx_queue_full_tx_queue(void) {
    // Relies on TX_CAPACITY being 1
    uint8_t a = 1;
    uint8_t b = 2;

    TEST_ASSERT_EQUAL_INT(PAL_RC_OK, pal_api_add_to_tx_queue(&hpal, &a,sizeof(uint8_t)));
    TEST_ASSERT_EQUAL_INT(PAL_RC_QUEUE_FULL, pal_api_add_to_tx_queue(&hpal, &b,sizeof(uint8_t)));
}

void check_pal_api_add_to_tx_queue_ok(void) {
    uint8_t a = 9;
    TEST_ASSERT_EQUAL_INT(PAL_RC_OK, pal_api_add_to_tx_queue(&hpal, &a,sizeof(uint8_t)));
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

    TEST_ASSERT_EQUAL_INT(PAL_RC_OK, pal_api_add_to_tx_queue(&hpal, &a,sizeof(uint8_t)));
    TEST_ASSERT_EQUAL_INT(PAL_RC_SERIALIZATION_ERR, pal_api_exec_tx(&hpal));
}

void check_pal_api_exec_tx_send_error(void) {
    uint8_t a = 9;

    hpal.send = send_error;

    TEST_ASSERT_EQUAL_INT(PAL_RC_OK, pal_api_add_to_tx_queue(&hpal, &a,sizeof(uint8_t)));
    TEST_ASSERT_EQUAL_INT(PAL_RC_IO_ERR, pal_api_exec_tx(&hpal));
}

void check_pal_api_exec_tx_send_ok(void) {
    uint8_t a = 9;

    TEST_ASSERT_EQUAL_INT(PAL_RC_OK, pal_api_add_to_tx_queue(&hpal, &a,sizeof(uint8_t)));
    TEST_ASSERT_EQUAL_INT(PAL_RC_OK, pal_api_exec_tx(&hpal));
}

/*!
 * @}
 */

/*!
 * \defgroup            Test P.A.L. message transmission
 * @{
 */
void check_pal_api_message_transmission(void) {
    pal_api_init(&hpal,TX_CAPACITY,MAX_MSG_SIZE,send_global_buff,NULL,NULL ,&harena);
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
     * \defgroup            Test P.A.L. message transmission
     * @{
     */

    RUN_TEST(check_pal_api_message_transmission);

    /*!
     * @}
     */
    UNITY_END();
}
