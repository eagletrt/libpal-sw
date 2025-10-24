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

ArenaAllocatorHandler_t harena;
struct PalHandler hpal;

void setUp() {
    arena_allocator_api_init(&harena);
}

void tearDown() {
    arena_allocator_api_free(&harena);
}

/*!
 * \defgroup            pal_init Test P.A.L. init
 * \{
 */

void check_pal_api_init_null_handler(void) {
    // UNITY_TEST_ASSERT_EQUAL_INT(PAL_OK, pal_api_init());
}

/*!
  * \}
  */

int main(void) {
    UNITY_BEGIN();

    UNITY_END();
}
