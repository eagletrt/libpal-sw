/*!
 * \file            pal.h
 * \date            2025-09-26
 * \authors         Mario Mazzara [mario.mazzara@eagletrt.it]
 *                  Mirko Lana [mirko.lana@eagletrt.it]
 *
 * \brief           Peripheral Abstraction Layer (PAL) library, suited for all
 *                  embedded devices.
 *
 * \details         TODO
 */

#ifndef PAL_H
#define PAL_H

#include <stddef.h>
#include <stdint.h>

/*!
 * \brief           Enumeration with all possible return codes of the library.
 */
enum PalReturnCode {
    PAL_RC_OK,      /*!< Everything is fine */
    PAL_RC_NULL_PTR /*!< Unexpected NULL pointer detected */
};

/*!
 * \brief           A structure that encapsulate data and functions required to
 *                  handle the communication with the desired peripheral.
 *
 * \attention       This structure should not be used directly.
 */
struct PalHandler {
};

#endif /*! PAL_H */
