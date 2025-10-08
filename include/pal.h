/*!
 * \file            pal.h
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


#ifndef PAL_TX_BUFFER_SIZE
/*!
 * \brief           TDB
 */
#define PAL_TX_BUFFER_SIZE 0xA
#endif /*! PAL_TX_BUFFER_SIZE */

#ifndef PAL_RX_BUFFER_SIZE
/*!
 * \brief           TBD
 */
#define PAL_RX_BUFFER_SIZE PAL_TX_BUFFER_SIZE
#endif /*! PAL_RX_BUFFER_SIZE */

/*!
 * \brief           Enumeration with all possible return codes of the library.
 */
enum PalReturnCode {
    PAL_RC_OK,         /*!< Everything is fine */
    PAL_RC_NULL_PTR,   /*!< Unexpected NULL pointer detected */
    PAL_RC_WRONG_SIZE, /*!< Size mismatch */
    PAL_RC_IO_ERR,     /*!< Generic I/O error */
    PAL_RC_BUFF_FULL,  /*!< Rx or Tx buffer is full */
};

enum PalProtocol{
    PAL_PROTO_CAN,
    PAL_PROTO_SPI,
    PAL_PROTO_I2C,
    PAL_PROTO_UART,
    PAL_PROTO_USART,
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
 * \param[in]       size: Size of the data buffer in bytes.
 * \return          The number of bytes received on success, -1 otherwise.
 */
typedef int (*pal_recv_raw_fn)(uint8_t *raw_data, size_t size);

/*!
 * \brief           Type definition for a function pointer used to serialize data.
 *
 * \param[in]       data: Pointer to the data to be serialized
 * \param[out]      size: Size of the buffer in bytes.
 * \return          The number of bytes received on success, -1 otherwise.
 */
typedef int (*pal_serialize_fn)(const void *data, uint8_t *buff, size_t size);

/*!
 * \brief           Type definition for a function pointer used to deserialize data.
 *
 * \param[out]      data: Pointer to the data to be deserialized.
 * \param[in]       size: Size of the buffer in bytes.
 * \return          PAL_RC_OK on success, an error code otherwise.
 */
typedef enum PalReturnCode (*pal_deserialize_fn)(void *data, const uint8_t *buff, size_t size);

/*!
 * \brief           A structure representing a message.
 */
struct PalMessage {
    uint8_t raw_data[PAL_MESSAGE_BUFFER_SIZE]; /*!< Raw data buffer */
    size_t size;                               /*!< Data size */
};

/*!
 * \brief           A structure that encapsulate data and functions required to
 *                  handle the communication with the desired peripheral.
 *
 * \attention       This structure should not be used directly.
 */
struct PalHandler {
    enum PalProtocol protocol;
    pal_send_raw_fn send;
    pal_recv_raw_fn recv;
    void *driver_ctx;
    RingBufferHandler_t tx_buffer; // Owned by PAL
    RingBufferHandler_t rx_buffer; // Owned by PAL
    void (*enter_cs)(void);
    void (*exit_cs)(void);
    void (*app_rx_cb)(void);  // to be called by pal_api_driver_receive_callback when data is received TODO: define the function signature
};

#endif /*! PAL_H */
