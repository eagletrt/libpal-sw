# Peripheral Abstraction Layer (P.A.L.)
PAL is a library designed to abstract communication protocol handling for embedded projects. It provides a unified interface for managing data reception and transmission.

The library does not operate independently; it requires a software layer to interact with the underlying hardware, which will henceforth be referred to as the "driver." Drivers must implement low-level operations such as transmission, reception, serialization, and critical section control, while application logic interacts with a simple API.

## Dependencies
PAL depends on [ArenaAllocator](https://github.com/eagletrt/libarena-allocator-sw.git) for memory management.
Ensure that you initialize the allocator handler before initializing the PAL handler.

Additionally, it requires [RingBuffer](https://github.com/eagletrt/libring-buffer-sw/tree/dev) for internal buffer management.

## Application Usage
### Initialization
To use PAL,first declare a handler using `PalHandler`.
In order to initialize it, the following need to be provided:
- `rx_capacity`: Number of elements in the reception queue.
- `tx_capacity`: Number of elements in the transmission queue.
- `max_msg_size`: Maximum size in bytes of a single message.
- `deserialize`: Function pointer for data deserialization (can be NULL).
- `send`: Function pointer for the driver-level transmission.
- `cs_enter` / `cs_exit`: Functions to manage critical sections (can be NULL).
- `arena`: A pointer to the initialized Arena Allocator.

For example:
```c
#define RX_CAPACITY (10U)
#define TX_CAPACITY (10U)  
#define CAN_MAX_MSG_SIZE (64U)
#define UART_MAX_MSG_SIZE (32U)
struct PalHandler hpal_can;
struct PalHandler hpal_uart;
struct ArenaAllocatorHandler arena;

enum PalReturnCode deserialize_custom(const struct PalMessage *in, void *out) {
    // deserialization implementation here
}

// Driver-specific send implementation
enum PalReturnCode uart_send(const struct PalMessage *msg) {
    // send implementation here
}

// Driver-specific send implementation
enum PalReturnCode can_send(const struct PalMessage *msg) {
    // send implementation here
}

arena_allocator_api_init(&arena);
pal_api_init(&hpal_can, RX_CAPACITY, TX_CAPACITY, CAN_MAX_MSG_SIZE, NULL, can_send, NULL, NULL, &arena);
pal_api_init(&hpal_uart, RX_CAPACITY, TX_CAPACITY, UART_MAX_MSG_SIZE, deserialize_custom, uart_send, cs_enter, cs_exit, &arena);

```
> [!WARNING]
> If using the default deserializer, ensure the destination buffer is at least `max_msg_size` to avoid a potential buffer overflow.

> [!NOTE]
> `NULL` can be passed in place of the `cs_enter` and `cs_exit` functions, in that case
> communications done through that handle are not guaranteed to always work in case of interrupts, an example implementation can be found in [RingBuffer's README](https://github.com/eagletrt/libring-buffer-sw)

### Reception
To process incoming data:
1. The driver populates the queue (see Driver Setup).
2. The application calls `pal_api_exec_rx` in a processing loop to pop the first message and execute the deserialization into a provided buffer.

### Transmission

To send data through the peripheral:
1. Queue Data: Use `pal_api_add_to_tx_queue` to copy structured data into the internal transmission buffer.
2. Execute Transmission: Call `pal_api_exec_tx` to trigger the send function provided during initialization.

## Driver Setup
Drivers act as the bridge between the hardware and PAL. A driver must:
- In ISR or receive callback: Capture hardware data and call `pal_api_add_to_rx_queue` to copy raw bytes into the PAL reception queue.
- Provide a Send Implementation: Implement a function matching the `pal_send_callback` signature that handles the actual hardware-level transmission and/or serialization.
- Handle Errors: Properly map hardware status codes to the PalReturnCode enumeration.

## Examples

For more info check the [examples](./examples/) folder.
