#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

#define THRESHOLD 700

uint8_t relayState = 0;
uint8_t lastRelayState = 0;

void UART_init() {
    uint16_t ubrr = 103;
    UBRR0H = (ubrr >> 8);
    UBRR0L = ubrr;

    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_tx(char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void UART_print(const char *str) {
    while (*str) {
        UART_tx(*str++);
    }
}

void ADC_init() {
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | 
             (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t ADC_read() {
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

void sendAT(const char *cmd) {
    UART_print(cmd);
    UART_print("\r\n");
    _delay_ms(600);
}

void sendSMS(const char *number, const char *message) {
    sendAT("AT");
    sendAT("AT+CMGF=1");

    UART_print("AT+CMGS=\"");
    UART_print(number);
    UART_print("\"\r\n");
    _delay_ms(500);

    UART_print(message);
    UART_tx(26);
    _delay_ms(5000);
}

int main(void) {

    DDRD |= (1 << PD5);
    PORTD &= ~(1 << PD5);

    UART_init();
    ADC_init();

    const char phoneNumber[] = "+94766202660";

    char msg[64];
    uint16_t moisture;

    while (1) {

        moisture = ADC_read();

        uint8_t isDry = (moisture >= THRESHOLD);

        lastRelayState = relayState;
        relayState = isDry;

        if (relayState)
            PORTD |= (1 << PD5);
        else
            PORTD &= ~(1 << PD5);

        if (relayState != lastRelayState) {

            if (relayState) {
                sprintf(msg, "DRY detected! Relay ON. ADC=%u", moisture);
            } else {
                sprintf(msg, "Soil OK/WET. Relay OFF. ADC=%u", moisture);
            }

            sendSMS(phoneNumber, msg);
        }

        _delay_ms(1000);
    }
}