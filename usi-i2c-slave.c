#include <USI-I2C-slave.h>

volatile uint8_t recBuffer[BUFFERSIZE];
volatile uint8_t tranBuffer[BUFFERSIZE];
volatile uint8_t recCount = 0;
volatile uint8_t tranCount = 0;
volatile uint8_t comMode = NONE; // communication mode - 0: NONE, 1: ADDRESS_MODE, 2: MASTER_READ_DATA, 3: MASTER_WRITE_DATA
uint8_t slaveAddress;
volatile uint8_t comState;

  /////////////////////////////////
 // INITIALIZE USI-I2C AS SLAVE //
/////////////////////////////////
void USI_I2C_slave_init(uint8_t address) {
	slaveAddress = address;
	// set SDA as INPUT and SCL as OUTPUT
	USI_DDR &= ~(1 << USI_SDA);
	USI_DDR |= (1 << USI_SCL);
	USI_PORT |= (1 << USI_SCL) | (1 << USI_SDA);
	
	// Start Condition Interrupt Enable, External, positive edge; shift data by External, Both Edges
	USICR = (1 << USISIE) | (1 << USIWM1) | (0 << USIWM0) | (1 << USICS1);

	// clear Interrupt Flags and Counter.
	USISR = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC);
	
	comMode = NONE;
}

  //////////////////////////
 // FLUSH RECEIVE BUFFER //
//////////////////////////
void flushRecBuffer() {
	for (uint8_t i = 0; i < BUFFERSIZE; i++) {
		recBuffer[i] = 0;
	}
}

ISR(USI_START_vect) {
	
	// set SDA as INPUT
	USI_DDR &= ~(1 << USI_SDA);
	
	comMode = ADDRESS_MODE;
	recCount = 0;
	tranCount = 0;
	flushRecBuffer();
	// wait until START condition completed
	while ((USI_PIN & (1 << USI_SCL)) && !(USI_PIN & (1 << USI_SDA)));
	
	// if STOP condition occurred
	if (USI_PIN & (1 << USI_SCL)) {
		USICR = (1 << USISIE) | (1 << USIWM1) | (1 << USICS1);
		comState = STOP;
	}
	else {
		comState = START;
		USICR = (1 << USISIE) | (1 << USIOIE) | (1 << USIWM1) | (1 << USIWM0) | (1 << USICS1);
	}
	// clear Flags and set 4-bit Counter for 8-bit SLA+R/W
	USISR = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC);
	USIDR = 0x00;
}

ISR(USI_OVERFLOW_vect) {
	switch (comMode) {
		case ADDRESS_MODE:	if ((USIDR >> 1) == slaveAddress) {
								if (USIDR & 0x01)
									comMode = MASTER_READ_DATA;
								else
									comMode = MASTER_WRITE_DATA;
								
								// send ACK to Master
								SEND_ACK();
							}
							// init START condition mode
							else {
								INIT_START_CONDITION_MODE();
							}
							USIDR = 0x00;
							break;
		
		case MASTER_WRITE_DATA:	
							comMode = SLAVE_RECEIVE_DATA;
							INIT_DATA_RECEPTION();
							// STOP condition detector
							// wait for SDA HIGH
							while (!(USI_PIN & (1 << USI_SDA)));
							// if STOP condition
							if (USI_PIN & (1 << USI_SCL)) {
								INIT_START_CONDITION_MODE();
								comState = STOP;
							}
							break;
							
		case SLAVE_RECEIVE_DATA:
							comMode = MASTER_WRITE_DATA;
							if (recCount < BUFFERSIZE) {
								recBuffer[recCount++] = USIDR;
								// send ACK to Master
								SEND_ACK();
							}
							else {
								SEND_NACK();
							}
							break;
							
		case MASTER_READ_DATA:
							// if received NACK
							if (USIDR) {
								INIT_START_CONDITION_MODE();
							}
							else {
								comMode = SLAVE_TRANSMIT_DATA;
								if (tranCount < BUFFERSIZE) {
									USIDR = tranBuffer[tranCount];
									INIT_DATA_TRANSMITTION();
								}
							}
							break;
							
		case SLAVE_TRANSMIT_DATA:
							comMode = MASTER_READ_DATA;
							RECEIVE_ACK();
							break;
	}
}



