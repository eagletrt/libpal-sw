# Peripheral Abstraction Layer (P.A.L.)
PAL is a library designed to abstract communication protocol handling for embedded projects. It provides a unified interface for managing data reception and transmission.

The library does not operate independently; it requires a software layer to interact with the underlying hardware, which will henceforth be referred to as the "driver." Drivers must implement low-level operations such as transmission, reception, serialization, and critical section control, while application logic interacts with a simple API.

## Dependencies
PAL depends on [ArenaAllocator](https://github.com/eagletrt/libarena-allocator-sw.git) for memory management.
Ensure that you initialize the allocator handler before initializing the PAL handler.

Additionally, it requires [RingBuffer](https://github.com/eagletrt/libring-buffer-sw/tree/dev) for internal buffer management.

## Application Usage
### Initialization
To use PAL,first declare an handler using `PalHandler`.
In order to initialize it, the following need to be provided:
- `rx_queue` capacity
- Deserialize function
- Functions to enter and exit critical section (can be null)
- Arena allocator

For example:
```c
#define RX_CAPACITY (10U)
PalHandler hcan;
PalHandler hspi;
ArenaAllocatorHandler_t arena;

enum PalReturnCode deserialize_default(const struct PalMessage *in, void *out) {
    if (!in || !out)
        return -1;

    (void)in;
    return 0;
}

arena_allocator_api_init(&arena);
pal_api_init(&hcan, RX_CAPACITY, deserialize_default, NULL, NULL, &arena);
pal_api_init(&hspi, RX_CAPACITY, deserialize_default, cs_enter, cs_exit, &arena);

```
> [!NOTE]
> `NULL` can be passed in place of the `cs_enter` and `cs_exit` functions, in that case
> communications done throught that handle are not guaranteed to always work in case of interrupts, an example implementation can be found in [RingBuffer's README](https://github.com/eagletrt/libring-buffer-sw)

### Reception
After initial setup `pal_api_exec_rx` can be executed in a loop to process the messages in queue one at a time.

## Driver Setup
Drivers must provide:
- In ISR or receive callback: call pal_api_add_to_rx_queue.
- Low-level transmit, serialization and deserialization functions.

## Examples

For more info check the [examples](./examples/) folder.
