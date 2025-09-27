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

#define PAL_BUFF_SIZE 0xfff

/*!
 * \brief           Enumeration with all possible return codes of the library.
 */
typedef enum PalReturnCode {
    PAL_OK,      /*!< Everything is fine */
    PAL_NULL_PTR /*!< Unexpected NULL pointer detected */
} PalReturnCode_t;

/*!
 * \brief           A structure that encapsulate data and functions required to
 *                  handle the communication with the desired peripheral.
 *
 * \attention       This structure should not be used directly.
 */
typedef struct PalHandler {
    uint8_t buff[PAL_BUFF_SIZE]; /*!< Raw data buffer */
} PalHandler_t;

#endif /*! PAL_H */
