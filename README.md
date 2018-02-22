# Attiny-USI-I2C-slave
Using the USI module as a I2C slave.

## Usage & API
- The core of the library is interrupt driven and therefore “runs in parallel” to the other processes in the application.
- Include the library to main program:
```C 
#include <USI-I2C-slave.h>
```
- **void USI_I2C_slave_init(uint8_t address)** - Initialize the I2C slave device.
  - address - Slave's address.
- **recBuffer[BUFFERSIZE]** - receive buffer.
- **tranBuffer[BUFFERSZIE]** - transmit buffer.
- **recCount** - receive buffer counter.
- **tranCount** - transmit buffer counter.
## Notes
- Master Transmitter Mode works properly.
- Master Receiver Mode isn't tested.
