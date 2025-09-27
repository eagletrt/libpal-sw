/*!
 * \file            pal-api.c
 * \date            2025-09-26
 * \authors         Mario Mazzara [mario.mazzara@eagletrt.it]
 *                  Mirko Lana [mirko.lana@eagletrt.it]
 *
 * \brief           Peripheral Abstraction Layer (PAL) library, suited for all
 *                  embedded devices.
 *
 * \details         TODO
 */

#include "pal.h"
#include "pal-api.h"

PalReturnCode_t pal_api_init(PalHandler_t *hpal) {
    if (hpal == NULL)
        return PAL_NULL_PTR;

    (void)hpal;

    return PAL_OK;
}
