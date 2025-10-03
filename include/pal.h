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

#include "ring-buffer.h"

#include <stddef.h>
#include <stdint.h>

#ifndef PAL_MESSAGE_BUFFER_SIZE
/*!
 * \brief           The default message buffer size.
 */
#define PAL_MESSAGE_BUFFER_SIZE 0xFF
#endif /*! PAL_MESSAGE_BUFFER_SIZE */

/*!
 * \brief           Enumeration with all possible return codes of the library.
 */
enum PalReturnCode {
    PAL_RC_OK,       /*!< Everything is fine */
    PAL_RC_NULL_PTR, /*!< Unexpected NULL pointer detected */
};

/*!
 * \brief           Type definition for a function pointer used to send data.
 *
 * \param[in]       raw_data: Pointer to the data to be sent.
 * \param[in]       size: Size of the data buffer in bytes.
 * \return          0 on success, -1 otherwise.
 */
typedef int (*pal_send_raw_fn)(const uint8_t *raw_data, size_t size);

/*!
 * \brief           Type definition for a function pointer used to receive data.
 *
 * \param[out]      raw_data: Pointer to the data to be received.
 * \param[out]      size: Size of the data buffer in bytes.
 * \return          The number of bytes received on success, -1 otherwise.
 */
typedef int (*pal_recv_raw_fn)(uint8_t *raw_data, size_t size);

/*!
 * \brief           A structure representing a message.
 */
struct PalMessage {
    char raw_data[PAL_MESSAGE_BUFFER_SIZE]; /*!< Raw data buffer */
    size_t size;                            /*!< Data size */
};

/*!
 * \brief           A structure that encapsulate data and functions required to
 *                  handle the communication with the desired peripheral.
 *
 * \attention       This structure should not be used directly.
 */
struct PalHandler {
    pal_send_raw_fn send;
    pal_recv_raw_fn recv;
    RingBufferHandler_t tx_buffer;
    RingBufferHandler_t rx_buffer;
};

#endif /*! PAL_H */
