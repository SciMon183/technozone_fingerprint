#ifndef DEFINES
#define DEFINES

#define SERIAL_PINS 5,4
#define LED_RED 13
#define LED_GREEN 12
#define LED_BLUE 14
#define BUTTON_PIN 16

#ifdef DEBUG
#define PRINT(X) Serial.print(X)
#define PRINTLN(X) Serial.println(X)
#else
#define PRINT(X) 
#define PRINTLN(X) 
#endif

#define TIMER_INTERRUPT_DEBUG 0
#define TIMERINTERRUPT_LOGLEVEL 0

#endif //DEFINES