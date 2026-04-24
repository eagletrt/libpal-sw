/*!
 * \file        pal-custom-protocol.c
 * \date        2026-04-24
 * \authors     Mario Mazzara
 *
 * \brief       Example of PAL using variable-length binary serialization.
 * \details     Demonstrates how to handle a common Type-Length-Value (TLV) style 
 *              structure, ensuring safe binary packing and unpacking without relying on 
 *              null terminators or compiler-specific struct packing.
 */

#include <stdio.h>
#include <string.h>
#include "pal-api.h"
#include "arena-allocator-api.h"

#define MAX_MSG_SIZE (32U)
#define HEADER_SIZE (sizeof(uint32_t) + sizeof(uint8_t))
#define PAYLOAD_MAX_BYTES (MAX_MSG_SIZE - HEADER_SIZE)

// The structured data used by the application
struct ApplicationPacket {
    uint32_t size;                      // Number of valid bytes in the payload
    uint8_t command_id;                 // Command identifier
    uint8_t payload[PAYLOAD_MAX_BYTES]; // Variable length payload
};

/**
 * \brief Custom deserializer for variable-length binary data.
 */
enum PalReturnCode custom_deserialize(const struct PalMessage *in, void *out) {
    // Ensure we have at least enough bytes for the header
    if (in->size < HEADER_SIZE)
        return PAL_RC_DESERIALIZATION_ERR;

    struct ApplicationPacket *dest = (struct ApplicationPacket *)out;
    const uint8_t *src = in->data;

    // Unpack size safely (Little Endian mapping)
    dest->size = (uint32_t)src[0] |
                 ((uint32_t)src[1] << 8) |
                 ((uint32_t)src[2] << 16) |
                 ((uint32_t)src[3] << 24);

    // Validate the extracted size
    if (dest->size > PAYLOAD_MAX_BYTES || in->size < (HEADER_SIZE + dest->size)) {
        return PAL_RC_DESERIALIZATION_ERR;
    }

    dest->command_id = src[4];

    // Copy only the valid payload bytes
    if (dest->size > 0) {
        memcpy(dest->payload, &src[5], dest->size);
    }

    return PAL_RC_OK;
}

/**
 * \brief Serializes the ApplicationPacket into a byte array for transmission.
 */
enum PalReturnCode serialize_and_send(struct PalHandler *hpal, const struct ApplicationPacket *pkt) {
    if (pkt->size > PAYLOAD_MAX_BYTES)
        return PAL_RC_INVALID_PARAM;

    uint8_t buffer[MAX_MSG_SIZE];
    uint32_t total_transmission_size = HEADER_SIZE + pkt->size;

    // Pack size safely (Little Endian mapping)
    buffer[0] = (uint8_t)(pkt->size & 0xFF);
    buffer[1] = (uint8_t)((pkt->size >> 8) & 0xFF);
    buffer[2] = (uint8_t)((pkt->size >> 16) & 0xFF);
    buffer[3] = (uint8_t)((pkt->size >> 24) & 0xFF);

    buffer[4] = pkt->command_id;

    // Copy payload
    if (pkt->size > 0) {
        memcpy(&buffer[5], pkt->payload, pkt->size);
    }

    // Queue only the necessary bytes, not the full MAX_MSG_SIZE
    return pal_api_add_to_tx_queue(hpal, buffer, total_transmission_size);
}

// Dummy driver send function for compilation
enum PalReturnCode dummy_send(const struct PalMessage *msg) {
    (void)msg;
    return PAL_RC_OK;
}

int main(void) {
    struct PalHandler hpal;
    struct ArenaAllocatorHandler arena;
    struct ApplicationPacket received_pkt;

    arena_allocator_api_init(&arena);

    // Initialize PAL with the custom deserializer callback
    pal_api_init(&hpal, 5, 5, MAX_MSG_SIZE, custom_deserialize, dummy_send, NULL, NULL, &arena);

    // --- Serialization Phase ---
    struct ApplicationPacket pkt_out;
    pkt_out.command_id = 0x10;
    pkt_out.size = 3; // We only want to send 3 bytes of payload
    pkt_out.payload[0] = 0xAA;
    pkt_out.payload[1] = 0xBB;
    pkt_out.payload[2] = 0xCC;

    serialize_and_send(&hpal, &pkt_out);
    pal_api_exec_tx(&hpal);

    // --- Simulated Reception Phase ---
    // Simulating hardware receiving a packet: Size (3), Cmd (0x10), Payload (AA BB CC)
    uint8_t raw_incoming[8] = { 0x03, 0x00, 0x00, 0x00, 0x10, 0xAA, 0xBB, 0xCC };
    pal_api_add_to_rx_queue(&hpal, raw_incoming, sizeof(raw_incoming));

    // --- Processing Phase ---
    if (pal_api_exec_rx(&hpal, &received_pkt) == PAL_RC_OK) {
        printf("[APP] Received Command: 0x%X\n", received_pkt.command_id);
        printf("[APP] Payload Size: %u bytes\n", received_pkt.size);
        printf("[APP] Payload Data: ");
        for (uint32_t i = 0; i < received_pkt.size; i++) {
            printf("%02X ", received_pkt.payload[i]);
        }
        printf("\n");
    }

    arena_allocator_api_free(&arena);
    return 0;
}
