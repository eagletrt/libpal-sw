/*!
 * \file            pal-driver-mock.h
 * \date            2026-04-24
 * \authors         Mario Mazzara
 *
 * \brief           Header file for the example PAL driver implementation.
 * \details         Declares the initialization functions and interrupt service routine
 * handlers required to interface the mock hardware with the PAL layer.
 */

#ifndef PAL_DRIVER_MOCK_H
#define PAL_DRIVER_MOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "pal-api.h"

/**
 * \brief Initializes the PAL handler with driver-specific configurations.
 * \param[out]  hpal   Pointer to the PAL handler structure to be initialized.
 * \param[in]   arena  Pointer to the Arena Allocator handler for memory allocation.
 * \reval       PAL_RC_OK on success
 * \retval error code on failure.
 */
enum PalReturnCode mock_driver_init(struct PalHandler *hpal, struct ArenaAllocatorHandler *arena);

/**
 * \brief Hardware Interrupt Service Routine (ISR) mapping.
 * \details Captures data from the simulated hardware register and pushes it to 
 * the PAL reception queue.
 * \param[out] hpal Pointer to the initialized PAL handler structure.
 */
void MOCK_HW_IRQHandler(struct PalHandler *hpal);

#ifdef __cplusplus
}
#endif

#endif /* PAL_DRIVER_MOCK_H */
