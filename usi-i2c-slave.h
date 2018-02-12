#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#if defined(__AVR_ATtiny2313__)
	#define USI_DDR		DDRB
	#define USI_PORT	PORTB
	#define USI_PIN		PINB
	#define USI_SDA		PB5
	#define USI_SCL		PB7
#endif

#if defined(__AVR_ATtiny85__)
	#define USI_DDR		DDRB
	#define USI_PORT	PORTB
	#define USI_PIN		PINB
	#define USI_SDA		PB0
	#define USI_SCL		PB2
#endif

#define releaseSCL()	USI_PORT |= (1 << USI_SCL)
#define releaseSDA()	USI_PORT |= (1 << USI_SDA)
#define pullSCL()	USI_PORT &= ~(1 << USI_SCL)
#define pullSDA()	USI_PORT &= ~(1 << USI_SDA)

#define USI_ACK		0
#define USI_DATA	1

#define USI_READ	1
#define USI_WRITE	0
#define NOTLAST		0
#define LAST		1

#define LOW_PERIOD	4
#define HIGH_PERIOD	5

#define NONE				0
#define ADDRESS_MODE		1
#define MASTER_READ_DATA	2
#define MASTER_WRITE_DATA	3
#define SLAVE_RECEIVE_DATA	4	
#define SLAVE_TRANSMIT_DATA	5

#define START				0
#define STOP				1
#define BUFFERSIZE			8

extern volatile uint8_t tranCount;
extern volatile uint8_t recCount;
extern volatile uint8_t recBuffer[];
extern volatile uint8_t tranBuffer[];
extern volatile uint8_t comState;

void USI_I2C_slave_init(uint8_t);

#define INIT_START_CONDITION_MODE() {							\
	/* Start Condition Interrupt Enable, External, positive edge; shift data by External, Both Edges */ \
	USICR = (1 << USISIE) | (1 << USIWM1) | (0 << USIWM0) | (1 << USICS1);		\
	/* clear Interrupt Flags and Counter. */					\
	USISR = (1 << USIOIF) | (1 << USIPF) | (1 << USIDC);		\
}

#define SEND_ACK() { 											\
	/* pull SDA to LOW */ 										\
	USI_DDR |= (1 << USI_SDA); 									\
	pullSDA(); 													\
	/* 1-bit counter for ACK */ 								\
	USISR = (1 << USIOIF) | (1 << USIPF) | (1 << USIDC) | 0x0E; \
}

#define SEND_NACK() { 											\
	/* release SDA */ 											\
	USI_DDR |= (1 << USI_SDA); 									\
	releaseSDA;													\
	/* 1-bit counter for ACK */ 								\
	USISR = (1 << USIOIF) | (1 << USIPF) | (1 << USIDC) | 0x0E; \
}									

#define RECEIVE_ACK() {											\
	USI_DDR &= ~(1 << USI_SDA);									\
	USIDR = 0x00;												\
	/* 1-bit counter for ACK */ 								\
	USISR = (1 << USIOIF) | (1 << USIPF) | (1 << USIDC) | 0x0E; \
}

#define INIT_DATA_RECEPTION() {									\
	USI_DDR &= ~(1 << USI_SDA);									\
	USISR = (1 << USIOIF) | (1 << USIPF) | (1 << USIDC);		\
}

#define INIT_DATA_TRANSMITTION() {								\
	USI_DDR |= (1 << USI_SDA);									\
	USISR = (1 << USIOIF) | (1 << USIPF) | (1 << USIDC);		\
}	
