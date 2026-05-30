/*!
 * \file    pal-custom-protocol.c
 * \date    2026-04-24
 * \authors Mario Mazzara [mario.mazzara@eagletrt.it], Alessandro Bridi [ale.bridi15@gmai.com]
 *
 * \brief   PAL custom TLV protocol over STM32 HAL UART.
 */

#include <string.h>
#include "stm32f4xx_hal.h" /* Replace with your actual target header */
#include "pal-api.h"
#include "arena-allocator-api.h"

#define MAX_MSG_SIZE (32U)
#define HEADER_SIZE (sizeof(uint32_t) + sizeof(uint8_t))
#define PAYLOAD_MAX_BYTES (MAX_MSG_SIZE - HEADER_SIZE)

struct ApplicationPacket {
    uint32_t size;
    uint8_t command_id;
    uint8_t payload[PAYLOAD_MAX_BYTES];
};

UART_HandleTypeDef huart1;

static struct PalHandler hpal;
static uint8_t uart_rx_staging[MAX_MSG_SIZE];

static enum PalReturnCode custom_deserialize(const struct PalMessage *in, void *out) {
    if (in->size < HEADER_SIZE) {
        return PAL_RC_DESERIALIZATION_ERROR;
    }

    struct ApplicationPacket *dest = (struct ApplicationPacket *)out;
    const uint8_t *src = in->payload;

    dest->size = (uint32_t)src[0] |
                 ((uint32_t)src[1] << 8) |
                 ((uint32_t)src[2] << 16) |
                 ((uint32_t)src[3] << 24);

    if (dest->size > PAYLOAD_MAX_BYTES || in->size < (HEADER_SIZE + dest->size)) {
        return PAL_RC_DESERIALIZATION_ERROR;
    }

    dest->command_id = src[4];

    if (dest->size > 0) {
        memcpy(dest->payload, &src[5], dest->size);
    }

    return PAL_RC_OK;
}

static enum PalReturnCode serialize_and_send(struct PalHandler *h, const struct ApplicationPacket *pkt) {
    if (pkt->size > PAYLOAD_MAX_BYTES) {
        return PAL_RC_INVALID_ARGUMENT;
    }

    uint8_t buffer[MAX_MSG_SIZE];
    uint32_t total = HEADER_SIZE + pkt->size;

    buffer[0] = (uint8_t)(pkt->size & 0xFF);
    buffer[1] = (uint8_t)((pkt->size >> 8) & 0xFF);
    buffer[2] = (uint8_t)((pkt->size >> 16) & 0xFF);
    buffer[3] = (uint8_t)((pkt->size >> 24) & 0xFF);
    buffer[4] = pkt->command_id;

    if (pkt->size > 0) {
        memcpy(&buffer[5], pkt->payload, pkt->size);
    }

    return pal_api_add_to_tx_queue(h, buffer, total);
}

static enum PalReturnCode stm32_uart_transmit(const struct PalMessage *msg) {
    if (msg == NULL)
        return PAL_RC_NULL_POINTER;
    if (msg->size > MAX_MSG_SIZE)
        return PAL_RC_MESSAGE_TOO_BIG;

    HAL_StatusTypeDef s = HAL_UART_Transmit(&huart1,
                                            (const uint8_t *)msg->payload,
                                            (uint16_t)msg->size,
                                            HAL_MAX_DELAY);
    return (s == HAL_OK) ? PAL_RC_OK : PAL_RC_IO_ERROR;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance != huart1.Instance)
        return;

    /*
     * The TLV header carries the exact payload size, so in a real project
     * you would do a two-step receive: first 5 bytes for the header, then
     * dest->size bytes for the payload. For simplicity here we receive a
     * full MAX_MSG_SIZE frame and let the deserializer validate it.
     */
    pal_api_add_to_rx_queue(&hpal, uart_rx_staging, MAX_MSG_SIZE);

    HAL_UART_Receive_IT(&huart1, uart_rx_staging, MAX_MSG_SIZE);
}

int main(void) {
    struct ArenaAllocatorHandler arena;
    struct ApplicationPacket received_pkt;

    HAL_Init();

    arena_allocator_api_init(&arena);

    pal_api_init(&hpal,
                 5,
                 5,
                 MAX_MSG_SIZE,
                 custom_deserialize,
                 stm32_uart_transmit,
                 NULL,
                 NULL,
                 &arena);

    HAL_UART_Receive_IT(&huart1, uart_rx_staging, MAX_MSG_SIZE);

    struct ApplicationPacket pkt_out = {
        .command_id = 0x10,
        .size = 3,
        .payload = { 0xAA, 0xBB, 0xCC }
    };

    serialize_and_send(&hpal, &pkt_out);
    pal_api_process_tx(&hpal);

    while (1) {
        if (pal_api_process_rx(&hpal, &received_pkt) == PAL_RC_OK) {
            /* process received_pkt.command_id and received_pkt.payload */
        }
    }

    arena_allocator_api_free(&arena);
    return 0;
}
