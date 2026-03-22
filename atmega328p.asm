.include "m328pdef.inc"

.equ THRESHOLD_L = low(700)
.equ THRESHOLD_H = high(700)

.dseg
relayState:      .byte 1
lastRelayState:  .byte 1

.cseg
.org 0x0000
rjmp RESET

RESET:
    ldi r16, low(RAMEND)
    out SPL, r16
    ldi r16, high(RAMEND)
    out SPH, r16

    sbi DDRD, PD5

    ldi r16, 103
    out UBRR0L, r16
    clr r16
    out UBRR0H, r16

    ldi r16, (1<<TXEN0)|(1<<RXEN0)
    out UCSR0B, r16

    ldi r16, (1<<UCSZ01)|(1<<UCSZ00)
    out UCSR0C, r16

    ldi r16, (1<<REFS0)
    out ADMUX, r16

    ldi r16, (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)
    out ADCSRA, r16

    cbi PORTD, PD5

MAIN_LOOP:
    sbi ADCSRA, ADSC

WAIT_ADC:
    sbis ADCSRA, ADIF
    rjmp WAIT_ADC
    sbi ADCSRA, ADIF

    in r24, ADCL
    in r25, ADCH

    ldi r18, THRESHOLD_L
    ldi r19, THRESHOLD_H

    cp r24, r18
    cpc r25, r19
    brlo WET

DRY:
    ldi r20, 1
    rjmp UPDATE

WET:
    clr r20

UPDATE:
    lds r21, relayState
    sts lastRelayState, r21
    sts relayState, r20

    tst r20
    breq RELAY_OFF

RELAY_ON:
    sbi PORTD, PD5
    rjmp CHECK_CHANGE

RELAY_OFF:
    cbi PORTD, PD5

CHECK_CHANGE:
    lds r22, lastRelayState
    cp r22, r20
    breq NO_SMS

    rcall SEND_AT
    rcall SEND_CMGS

NO_SMS:
    rcall DELAY_1S
    rjmp MAIN_LOOP

USART_SEND:
    sbis UCSR0A, UDRE0
    rjmp USART_SEND
    out UDR0, r16
    ret

SEND_AT:
    ldi r16, 'A'
    rcall USART_SEND
    ldi r16, 'T'
    rcall USART_SEND
    ldi r16, 13
    rcall USART_SEND
    ret

SEND_CMGS:
    ldi r16, 'A'
    rcall USART_SEND
    ret

DELAY_1S:
    ldi r18, 100
D1:
    ldi r19, 200
D2:
    ldi r20, 200
D3:
    dec r20
    brne D3
    dec r19
    brne D2
    dec r18
    brne D1
    ret