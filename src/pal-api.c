/*!
 * \file            pal-api.c
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

#include "pal.h"
#include "pal-api.h"
#include "arena-allocator.h"
#include "ring-buffer.h"
#include "ring-buffer-api.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static int prv_pal_api_dummy_send(const uint8_t *raw_data, size_t size) {
    /*! Avoid "unused variable" warning messages */
    (void)raw_data;
    (void)size;
    return 0;
}

static int prv_pal_api_dummy_recv(uint8_t *raw_data, size_t size) {
    /*! Avoid "unused variable" warning messages */
    (void)raw_data;
    (void)size;
    return 0;
}

static void prv_pal_api_dummy_fn(void) {
}

enum PalReturnCode pal_api_init(
    struct PalHandler *hpal,
    pal_recv_raw_fn recv,
    pal_send_raw_fn send,
    void *driver_ctx,
    void (*enter_cs)(void),
    void (*exit_cs)(void),
    void (*app_cb)(void),
    ArenaAllocatorHandler_t *arena) {
    if (hpal == NULL)
        return PAL_RC_NULL_PTR;

    hpal->send = send == NULL ? prv_pal_api_dummy_send : send;
    hpal->recv = recv == NULL ? prv_pal_api_dummy_recv : recv;
    hpal->enter_cs = enter_cs == NULL ? prv_pal_api_dummy_fn : enter_cs;
    hpal->exit_cs = exit_cs == NULL ? prv_pal_api_dummy_fn : exit_cs;

    hpal->driver_ctx = driver_ctx;

    if (app_cb == NULL)
        return PAL_RC_NULL_PTR;

    int res = ring_buffer_api_init(&hpal->tx_buffer, 1U, PAL_TX_BUFFER_SIZE, enter_cs, exit_cs, arena);
    if (res != RING_BUFFER_OK)
        return PAL_RC_NULL_PTR;

    res = ring_buffer_api_init(&hpal->rx_buffer, 1U, PAL_RX_BUFFER_SIZE, enter_cs, exit_cs, arena);
    if (res != RING_BUFFER_OK)
        return PAL_RC_NULL_PTR;

    return PAL_RC_OK;
}
