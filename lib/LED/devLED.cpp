#include <Arduino.h>
#include "common.h"
#include "device.h"

#if defined(TARGET_RX)
extern bool connectionHasModelMatch;
#endif

constexpr uint8_t LEDSEQ_WIFI_UPDATE[] = { 2, 3 };     // 20ms on, 30ms off
constexpr uint8_t LEDSEQ_BINDING[] = { 10, 10, 10, 100 };   // 2x 100ms blink, 1s pause

static bool blipLED;

static uint8_t _pin = -1;
static uint8_t _pin_inverted;
static const uint8_t *_durations;
static uint8_t _count;
static uint8_t _counter = 0;

static uint16_t updateLED()
{
    if(_counter % 2 == 1)
        digitalWrite(_pin, LOW ^ _pin_inverted);
    else
        digitalWrite(_pin, HIGH ^ _pin_inverted);
    if (_counter >= _count)
    {
        _counter = 0;
    }
    return _durations[_counter++] * 10;
}

static uint16_t flashLED(uint8_t pin, uint8_t pin_inverted, const uint8_t durations[], uint8_t count)
{
    _counter = 0;
    _pin = pin;
    _pin_inverted = pin_inverted;
    _durations = durations;
    _count = count;
    return updateLED();
}

static void initialize()
{
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, HIGH);
}

static int timeout()
{
    if (connectionState == running && blipLED)
    {
        blipLED = false;
        digitalWrite(PIN_LED, HIGH);
        return DURATION_NEVER;
    }
    return updateLED();
}

static int event()
{
    if (connectionState == running && blipLED)
    {
        digitalWrite(PIN_LED, LOW);
        return 200; // 200ms off
    }
    if (connectionState == binding)
    {
        return flashLED(PIN_LED, false, LEDSEQ_BINDING, sizeof(LEDSEQ_BINDING));
    }
    if (connectionState == wifiUpdate)
    {
        return flashLED(PIN_LED, false, LEDSEQ_WIFI_UPDATE, sizeof(LEDSEQ_WIFI_UPDATE));
    }
    return DURATION_NEVER;
}

void turnOffLED()
{
    digitalWrite(PIN_LED, LOW);
}

void blinkLED()
{
  blipLED = true;
  devicesTriggerEvent();
}

device_t LED_device = {
    .initialize = initialize,
    .start = event,
    .event = event,
    .timeout = timeout
};
