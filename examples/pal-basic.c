/*!
 * \file    pal-basic.c
 * \date    2026-04-24
 * \authors Mario Mazzara [mario.mazzara@eagletrt.it], Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief   Basic example of PAL application-level usage.
 * \details Demonstrates how an application initializes PAL (via a driver),
 *          queues data for transmission, and processes received data in main.
 */

#include <string.h>
#include "stm32f4xx_hal.h"
#include "pal-api.h"
#include "arena-allocator-api.h"

#define UART_MAX_MSG_SIZE (64U)
#define RX_CAPACITY (10U)
#define TX_CAPACITY (10U)

UART_HandleTypeDef huart1;
static struct PalHandler hpal;
static uint8_t uart_rx_staging[UART_MAX_MSG_SIZE];

static enum PalReturnCode stm32_uart_transmit(const struct PalMessage *msg) {
    if (msg == NULL)
        return PAL_RC_NULL_POINTER;
    if (msg->size > UART_MAX_MSG_SIZE)
        return PAL_RC_MESSAGE_TOO_BIG;

    HAL_StatusTypeDef s = HAL_UART_Transmit(&huart1,
                                            (const uint8_t *)msg->payload,
                                            (uint16_t)msg->size,
                                            HAL_MAX_DELAY);
    return (s == HAL_OK) ? PAL_RC_OK : PAL_RC_IO_ERROR;
}

static enum PalReturnCode stm32_uart_driver_init(struct PalHandler *h,
                                                 struct ArenaAllocatorHandler *arena) {
    return pal_api_init(h,
                        RX_CAPACITY,
                        TX_CAPACITY,
                        UART_MAX_MSG_SIZE,
                        NULL, /* default memcpy deserializer */
                        stm32_uart_transmit,
                        NULL,
                        NULL,
                        arena);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance != huart1.Instance)
        return;

    uint32_t len = strlen((const char *)uart_rx_staging) + 1;
    pal_api_add_to_rx_queue(&hpal, uart_rx_staging, len);

    HAL_UART_Receive_IT(&huart1, uart_rx_staging, UART_MAX_MSG_SIZE);
}

int main(void) {
    struct ArenaAllocatorHandler arena;
    char rx_buffer[UART_MAX_MSG_SIZE];

    HAL_Init();

    arena_allocator_api_init(&arena);

    if (stm32_uart_driver_init(&hpal, &arena) != PAL_RC_OK) {
        /* handle error */
    }

    HAL_UART_Receive_IT(&huart1, uart_rx_staging, UART_MAX_MSG_SIZE);

    const char *msg_out = "Hello from Application";
    if (pal_api_add_to_tx_queue(&hpal, (void *)msg_out, strlen(msg_out) + 1) != PAL_RC_OK) {
        /* handle error */
    }
    if (pal_api_process_tx(&hpal) != PAL_RC_OK) {
        /* handle error */
    }

    while (1) {
        if (pal_api_process_rx(&hpal, rx_buffer) == PAL_RC_OK) {
            /* forward rx_buffer to command parser */
        }
    }
}
