//#include <Arduino.h>
#include <Adafruit_Fingerprint.h>

#include "defines.h"

#include <ESP8266TimerInterrupt.h>

ESP8266Timer ITimer;

enum MODES
{
    BLINK = 0,
    FADE,
    HOLD
};

enum COLORS
{
    BLACK = 0,
    WHITE,
    RED,
    GREEN,
    BLUE
};

union color_struct {
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };
    uint8_t rgb[3];
};

struct led_status
{
    color_struct color;
    MODES mode;
    bool color_inverted;
} led {{255, 255, 255}, HOLD, false};


void operator*(color_struct &a, float &mult)
{
    a.r = (uint8_t)(a.r * mult);
    a.g = (uint8_t)(a.g * mult);
    a.b = (uint8_t)(a.b * mult);
}
void operator-=(color_struct &a, const int8_t &dimm)
{
    a.r = ((int16_t) a.r - dimm) <= 0 ? 0 : a.r - dimm;
    a.g = ((int16_t) a.g - dimm) <= 0 ? 0 : a.g - dimm;
    a.b = ((int16_t) a.b - dimm) <= 0 ? 0 : a.b - dimm;
}
bool operator==(const color_struct &a, const int8_t &b) {
    if (a.r == b && a.g == b && a.b == b)
        return true;
    return false;
}

color_struct translate_colors(COLORS color)
{
    switch (color)
    {
    case BLACK:
        return {0, 0, 0};
    case WHITE:
        return {255, 255, 255};
    case RED:
        return {255, 0, 0};
    case GREEN:
        return {0, 255, 0};
    case BLUE:
        return {0, 0, 255};
    default:
        return {0, 0, 0};
    }
}

color_struct invert_colors(color_struct color)
{
    return {255 - color.r, 255 - color.g, 255 - color.b};
}

void set_led(color_struct color)
{
    if (led.color_inverted) 
    {
        color = invert_colors(color);
    }
    PRINT("[set_led] ");
    PRINT(color.r);
    PRINT(" ");
    PRINT(color.g);
    PRINT(" ");
    PRINTLN(color.b);
    analogWrite(LED_RED,   color.r);
    analogWrite(LED_GREEN, color.g);
    analogWrite(LED_BLUE,  color.b);
}

void IRAM_ATTR rgb_led_handler()
{
    //PRINTLN("[HOHO] interrupt! ");
    static bool status = false;
    if (led.mode != BLINK)
    {
        status = true;
    }
    switch (led.mode)
    {
    case BLINK:
        if (status)
            set_led(led.color);
        else
            set_led(translate_colors(BLACK));
        status = !status;
        break;
    case FADE:
        set_led(led.color);
        led.color -= 1;
        PRINT("po ");
        PRINTLN(led.color.r);
        
        if (led.color == 0)
        {
            //TODO FADE is not working properly. Interrupt crash
            ITimer.detachInterrupt();
        }
        break;
    case HOLD:
        static bool first_time = true;
        if (!first_time){
            set_led(translate_colors(BLACK));
            ITimer.detachInterrupt();
            first_time = true;
            break;
        }
        set_led(led.color);
        first_time = false;
        break;
    default:
        break;
    }
}

void update_led_status(MODES mode, COLORS color, uint32_t time = 2000, bool invert = false)
{
    #ifdef EXTRA_INVERT
    //RGB.color = invert ? invert_colors(translate_colors(color)) : translate_colors(color);
    #endif // EXTRA_INVERT
    led.color = translate_colors(color);
    PRINT("update_led_status RGB: ");
    PRINT(led.color.r);
    PRINT(led.color.g);
    PRINTLN(led.color.b);
    led.mode = mode;
    ITimer.restartTimer();
    if (mode == FADE)
        ITimer.attachInterruptInterval(time * 1000 / 256, rgb_led_handler);
    else if (mode == BLINK) {
        ITimer.attachInterruptInterval(time * 400, rgb_led_handler);
    }
    else
        ITimer.attachInterruptInterval(time * 1000, rgb_led_handler);
}