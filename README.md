# Peripheral Abstraction Layer (P.A.L.)
The Peripheral Abstraction Layer library implements the logic needed to abstract a generic communication peripheral and its operations especially in embedded projects.
It provides a unified interface for managing message reception and transmission.

The library does not operate independently; it requires a software layer to interact with the underlying hardware, which will henceforth be referred to as the "driver".
Drivers must implement low-level operations such as transmission, reception, serialization, and critical section control, while application logic interacts with a simple API.

## Application Usage

To use PAL each peripheral may use one or multiple handlers that needs to be initialized.
The handler requires at least a callback to send the messages and optionally another one for the deserialization.

PAL uses two buffers: one for message transmission and one for message reception.
To send a message add it to the TX buffer and execute the routine.
When a message is received it can be added to the RX buffer so it can be later deserializer inside the routine.

> [!WARNING]
> If using the default deserializer, ensure that the destination buffer is big enough to avoid a potential buffer overflow.

## Driver Setup
Drivers act as the bridge between the hardware and PAL.

A driver must:
- Add the received message to the RX buffer of PAL;
- Implement the low-level function to serialize and transmit the message to the correct peripheral;
- Properly handle errors returned by PAL;

## Examples
For more info check the [examples](./examples/) folder.
