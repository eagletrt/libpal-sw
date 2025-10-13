/*!
 * \file            test-pal-api.c
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

#include "pal-api.h"
#include "pal.h"
#include "ring-buffer.h"
#include "unity.h"
#include "arena-allocator.h"
#include "arena-allocator-api.h"
#include "unity_internals.h"

ArenaAllocatorHandler_t harena;
struct PalHandler hpal;

void setUp() {
    arena_allocator_api_init(&harena);
    // pal_api_init(struct PalHandler *hpal, enum PalProtocol protocol, pal_recv_raw_fn recv, pal_send_raw_fn send, void *driver_ctx, void (*enter_cs)(void), void (*exit_cs)(void), void (*app_rx_cb)(void), pal_serialize_fn serialize, pal_deserialize_fn deserialize, ArenaAllocatorHandler_t *arena)
}

void tearDown() {
    arena_allocator_api_free(&harena);
}

/*!
 * \defgroup            pal_init Test P.A.L. init
 * @{
 */

void check_pal_api_init_null_handler(void) {
    // UNITY_TEST_ASSERT_EQUAL_INT(PAL_OK, pal_api_init());
}

/*!
  * @}
  */

int main(void) {
    UNITY_BEGIN();

    /*! TODO: implemnt unit tests */

    UNITY_END();
}
