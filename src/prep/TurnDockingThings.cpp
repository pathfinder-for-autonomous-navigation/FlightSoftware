
#include <Arduino.h>
#include <i2c_t3.h>

// Spike and Hold DCDC pin
constexpr unsigned char DCDC = 25;

// Motor controller pins
constexpr unsigned char DIRECTION_PIN = 16;
constexpr unsigned char SLEEP_PIN = 17;
constexpr unsigned char STEP_PIN = 39;

// Direction and number of steps for the motor to go through
constexpr auto DIRECTION = LOW;
constexpr auto STEPS = 100;

// Delay between steps in us
constexpr unsigned int DELAY = 4000;

// Ping the GOMSpace to prevent it from turning us off
void ping_gomspace()
{
    unsigned char buf[3];

    Wire.beginTransmission(0x02);
    Wire.write(0x01);
    Wire.write(0x09);

    if (Wire.endTransmission(I2C_NOSTOP, 100000))
        Serial.println("GS Transmission Failed!");

    if (Wire.requestFrom(0x02, 3, I2C_STOP, 100000) < 3)
        Serial.println("GS Request Failed!");

    Wire.read(buf, 3);
}

void setup()
{
    Serial.begin(9600);
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM);

    delay(5000);

    ping_gomspace();

    Serial.println("Enabling pins as outputs");

    // Set pin modes
    pinMode(DCDC, OUTPUT);
    pinMode(DIRECTION_PIN, OUTPUT);
    pinMode(SLEEP_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);

    Serial.println("Booting the DCDC");

    // Turn on the DCDC
    digitalWrite(DCDC, HIGH); delay(1000);

    ping_gomspace();

    Serial.print("Waking up the docking system");

    // Wake up the docking system
    digitalWrite(SLEEP_PIN, HIGH); delay(10);
    digitalWrite(DIRECTION_PIN, DIRECTION);

    for (auto i = 0; i < STEPS; i++)
    {
        digitalWrite(STEP_PIN, HIGH); delayMicroseconds(DELAY);
        digitalWrite(STEP_PIN, LOW); delay(170);

        if (i % 10 == 0) Serial.println("Stepping a multipl of ten...");

        ping_gomspace();
    }

    Serial.begin("Shutting down...");

    // Sleep the motor
    digitalWrite(SLEEP_PIN, HIGH); delay(100);

    // Turn off the DCDC
    digitalWrite(DCDC, LOW);
}

void loop()
{
    ping_gomspace(); delay(1000);
}
