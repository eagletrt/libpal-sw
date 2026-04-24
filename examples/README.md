# PAL Examples
These examples are not tied to any frameworks and are mocks to ensure portability across any development environment without requiring physical hardware.

## Content Overview

The examples are structured into single-purpose files to illustrate specific layers of the architecture:

[pal-basic.c](./pal-basic.c)

Demonstrates the standard application-level workflow:
- Initializing the PAL handler via a driver interface.
- Queuing data for transmission using pal_api_add_to_tx_queue.
- Processing received messages within a main execution loop via pal_api_exec_rx.

[pal-driver-mock.c](./pal-driver-mock.c)

Provides a template for hardware driver development:

- Mapping hardware transmission logic to the pal_send_fn signature.
- Bridging hardware interrupts (ISR) to the PAL reception queue using pal_api_add_to_rx_queue.

[pal-custom-protocol.c](./pal-custom-protocol.c)

Illustrates handling of structured data:
- Manual serialization of C structures into byte arrays for transmission.
- Implementation of a custom pal_deserialize_fn callback to reconstruct structured data upon reception.

## Running the examples:

- Dependencies: To run the examples they require pal and its dependencies.
- Environment: These examples are platform-independent and can be compiled with any standard C compiler (GCC, Clang, etc.).

### Compilation:

If PAL has been compiled atleast once in the debug eviroment the examples can be compiled using the following commands:
> [!NOTE]
> The following commands assume examples/ to be the working directory

#### `pal-basic.c`

```
gcc -g -Wall -Wextra \
    -I../include \
    -I../src \
    -I../.pio/libdeps/debug/ArenaAllocator/include \
    -I../.pio/libdeps/debug/RingBuffer/include \
    -I../.pio/libdeps/debug/libeagletrt/include \
    pal-basic.c \
    ../src/pal-api.c \
    ../.pio/libdeps/debug/ArenaAllocator/src/arena-allocator-api.c \
    ../.pio/libdeps/debug/RingBuffer/src/ring-buffer-api.c \
    ../.pio/libdeps/debug/libeagletrt/src/eagletrt-api.c \
    -o pal-basic-exec
```

#### `pal-custom-protocol.c`

```
gcc -g -Wall -Wextra \
    -I../include \
    -I../src \
    -I../.pio/libdeps/debug/ArenaAllocator/include \
    -I../.pio/libdeps/debug/RingBuffer/include \
    -I../.pio/libdeps/debug/libeagletrt/include \
    pal-custom-protocol.c \
    ../src/pal-api.c \
    ../.pio/libdeps/debug/ArenaAllocator/src/arena-allocator-api.c \
    ../.pio/libdeps/debug/RingBuffer/src/ring-buffer-api.c \
    ../.pio/libdeps/debug/libeagletrt/src/eagletrt-api.c \
    -o pal-custom-protocol-exec
```
