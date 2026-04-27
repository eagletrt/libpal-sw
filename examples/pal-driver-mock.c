/*!
 * \file            pal-driver-mock.c
 * \date            2026-04-24
 * \authors         Mario Mazzara
 *
 * \brief           Example of a PAL driver implementation.
 * \details         Demonstrates the required hardware-to-PAL mappings, including 
 *                  the transmission callback and the interrupt service routine (ISR) handler.
 */

#include <stdint.h>
#include <string.h>
#include "pal-api.h"

#define MOCK_RX_CAPACITY (10U)
#define MOCK_TX_CAPACITY (10U)
#define MOCK_MAX_MSG_SIZE (64U)

// Simulated hardware register
static uint8_t HW_REGISTER[MOCK_MAX_MSG_SIZE];

/**
 * \brief Hardware-specific transmission logic. In this example it just puts data in an array for the mock_IRQ to read from
 */
static enum PalReturnCode mock_hw_transmit(const struct PalMessage *msg) {
    if (msg == NULL)
        return PAL_RC_NULL_PTR;
    if (msg->size > MOCK_MAX_MSG_SIZE)
        return PAL_RC_MSG_TOO_BIG;

    // Simulate pushing data to a hardware peripheral (e.g., UART TX register)
    memcpy(HW_REGISTER, msg->payload, msg->size);

    return PAL_RC_OK;
}

/**
 * \brief Hardware Interrupt Service Routine (ISR) mapping.
 * \details Captures data from hardware and pushes it to the PAL reception queue.
 */
void MOCK_HW_IRQHandler(struct PalHandler *hpal) {
    // Simulate reading from a hardware peripheral

    // Assumes all messages sent in the example are null-terminated strings.
    // In a real project, use a serialization protocol that provides the exact size of the data.
    uint32_t len = strlen((const char *)HW_REGISTER) + 1;

    // Push raw bytes into the PAL queue
    enum PalReturnCode res = pal_api_add_to_rx_queue(hpal, HW_REGISTER, len);
    if (res != PAL_RC_OK) {
        // handle error
    }
}

/**
 * \brief Initializes the PAL handler with driver-specific configurations.
 */
enum PalReturnCode mock_driver_init(struct PalHandler *hpal, struct ArenaAllocatorHandler *arena) {
    // Initialize PAL with the mock_hw_transmit function and default deserializer
    return pal_api_init(
        hpal,
        MOCK_RX_CAPACITY,
        MOCK_TX_CAPACITY,
        MOCK_MAX_MSG_SIZE,
        NULL, // Default memcpy deserializer
        mock_hw_transmit,
        NULL, // No critical section enter
        NULL, // No critical section exit
        arena);
}
