/*!
 * \file            pal-basic.c
 * \date            2026-04-24
 * \authors         Mario Mazzara
 *
 * \brief           Basic example of PAL application-level usage.
 * \details         Demonstrates how an application initializes PAL (via a driver),
 *                  queues data for transmission, and processes received data in main.
 */

#include <stdio.h>
#include <string.h>
#include "../include/pal-api.h"
#include "arena-allocator-api.h"
#include "pal-driver-mock.h"

// Forward declarations for the mock driver interface
//extern enum PalReturnCode mock_driver_init(struct PalHandler *hpal, struct ArenaAllocatorHandler *arena);
//extern void MOCK_HW_IRQHandler(struct PalHandler *hpal); // Used here purely for simulation

#define MAX_SIZE (64U)

int main(void) {
    struct PalHandler hpal;
    struct ArenaAllocatorHandler arena;
    char rx_buffer[MAX_SIZE];

    // Initialize dependencies
    arena_allocator_api_init(&arena);

    // Initialize PAL via the driver implementation
    if (mock_driver_init(&hpal, &arena) != PAL_RC_OK) {
        printf("[ERROR] Failed to initialize PAL driver.\n");
        return -1;
    }

    // --- Transmission Phase ---
    const char *msg_out = "Hello from Application";
    enum PalReturnCode res_add_tx = pal_api_add_to_tx_queue(&hpal, (void *)msg_out, strlen(msg_out) + 1);
    if (res_add_tx != PAL_RC_OK) {
        //handle error
    }
    enum PalReturnCode res_process_tx = pal_api_process_tx(&hpal); // Triggers driver's send function
    if (res_process_tx != PAL_RC_OK) {
        //handle error
    }

    // --- Simulated Reception Phase ---
    // In a real application, this is triggered asynchronously by hardware
    MOCK_HW_IRQHandler(&hpal);

    // --- Processing Phase ---
    // Typically placed inside an infinite while(1) loop
    if (pal_api_process_rx(&hpal, rx_buffer) == PAL_RC_OK) {
        printf("[APP] Successfully processed received data: %s\n", rx_buffer);
    }

    arena_allocator_api_free(&arena);
    return 0;
}
