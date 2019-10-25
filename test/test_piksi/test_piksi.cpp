#include <core_pins.h>
#include <unity.h>
#include <usb_serial.h>
#include <array>
#include "../lib/Drivers/Piksi.hpp"

Devices::Piksi piksi("piksi", Serial4);
int setup_start_time;
int second_time;
int preread_time;

std::array<int, 3> pos;
std::array<int, 3> vel;
std::array<int, 3> baseline_pos;
msg_gps_time_t time;
unsigned int prev_tow = 0;
int out;

template <typename T>
bool comp(T a, T b, float margin) {
    if (abs(a - b) > margin) return false;
    return true;
}
float ssqrt(std::array<int, 3> in) {
    return (float)sqrt(((float)in[0]) * ((float)in[0]) + ((float)in[1]) * ((float)in[1]) +
                ((float)in[2]) * ((float)in[2]));
}
bool verify_out() {
    // Verify that last call to process_buffer was successful
    bool ret = (out == 1);

    if (!ret) Serial.printf("Process Out: %d\n", out);

    return ret;
}
bool verify_time() {
    // Verify that time has increased
    bool ret = time.tow > prev_tow;

    if (!ret) {
        // Serial.printf("GPS week: %d\n", time.wn);
        // Serial.printf("GPS OLD tow: %u\n", prev_tow);
        // Serial.printf("GPS tow: %u\n", time.tow);
    }

    return ret;
}
bool verify_pos() {
    // Check magnitude of position
    // float pos_mag = sqrt((float)pos[0] * (float)pos[0] + (float)pos[1] * (float)pos[1] +
    //                      (float)pos[2] * (float)pos[2]);
    float pos_mag = ssqrt(pos);
    bool ret = (piksi.get_pos_ecef_nsats() > 3);
    ret = ret && comp(6.37E6, pos_mag, 1.0E3);

    // Check flag
    ret = ret && (piksi.get_pos_ecef_flags() == 1);

    if (!ret) {
        Serial.printf("GPS position: %d,%d,%d\n", pos[0], pos[1], pos[2]);
        Serial.printf("pos_ecef_flags: %d\n", piksi.get_pos_ecef_flags());

        Serial.printf("Nsat Pos: %d\n", piksi.get_pos_ecef_nsats());
    }

    return ret;
}
bool verify_vel() {
    // Check mangitude of velocity
    float vel_mag = ssqrt(vel);

    // units of mm/s btw
    bool ret = (piksi.get_vel_ecef_nsats() > 3);
    ret = ret && comp(3.9E3, vel_mag, 5.0E2);

    ret = ret && (piksi.get_vel_ecef_flags() == 0);

    if (!ret) {
        Serial.printf("Vel: %d,%d,%d\n", vel[0], vel[1], vel[2]);
        Serial.printf("veloicty_mag: %g\n", vel_mag);
        Serial.printf("vel_ecef_flags: %d\n", piksi.get_vel_ecef_flags());

        Serial.printf("Nsat Vel: %d\n", piksi.get_vel_ecef_nsats());
    }

    return ret;
}
bool verify_baseline() {
    // float baseline_mag =
    //     sqrt(baseline_pos[0] * baseline_pos[0] + baseline_pos[1] * baseline_pos[1] +
    //          baseline_pos[2] * baseline_pos[2]);
    float baseline_mag = ssqrt(baseline_pos);

    bool ret = comp(1.0E5, baseline_mag, 2E3);
    if (!ret) {
        Serial.printf("GPS baseline position: %d,%d,%d\n", baseline_pos[0], baseline_pos[1],
                      baseline_pos[2]);
        Serial.printf("baseline_mag: %g\n", baseline_mag);
    }
    return ret;
}

// assume piksi already setup
bool execute_piksi_all() {
    // Serial.printf("Preread val: %d\n", pos[0]);
    preread_time = micros();
    // Serial.println("EVERYTHING: Attempting to get solution...");
    // Serial.printf("BYTES AVAIL: %u\n", piksi.bytes_available());
    bool ret = true;
    // tune parameters?
    //
    // CANNOT DO THIS, MESSAGES WILL VARY IN LENGTH
    // COULD BE A CASE WHERE ONCE IN SPACE, MESSAGE LENGTH ALWAYS NOT 299
    // if (piksi.bytes_available() == 299) {
    if (piksi.bytes_available() >= 200 && piksi.bytes_available() < 599) {
        while (piksi.bytes_available()) {
            out = piksi.process_buffer();
            delayMicroseconds(100);
        }
        prev_tow = time.tow;
        piksi.get_gps_time(&time);

        piksi.get_pos_ecef(&pos);
        piksi.get_vel_ecef(&vel);

        piksi.get_baseline_ecef(&baseline_pos);
        // Serial.printf("PROCESS BUFF OUT: %hi\n", out);

        // Serial.printf("RET: %d\n",ret);

        ret = verify_out() & verify_time() & verify_baseline() & verify_pos() & verify_vel();
    }

    else {
        // if no data in buffer throw error
        Serial.println("NOT A CLEAN READ ***");
        while (piksi.bytes_available()) {
            piksi.clear_bytes();
        }
        ret = false;
    }

    Serial.printf("Read time: %d ms\n", micros() - preread_time);
    // Serial.println();

    return ret;
}

void test_sats() {
    preread_time = millis();

    TEST_ASSERT_GREATER_THAN(
        4, piksi.get_pos_ecef_nsats());  // We need at least 4 satellites to get position.
    Serial.printf("Num Sats Read Time: %d ms\n", millis() - preread_time);
}

int main(void) {
    /*okay this stuff is really whack
    it seems like a normal packet is 299 bytes long
    when you get like 330 some bytes it's correlated with the
    gps time spiking up 200ms
    so i think whats happening is the call of the test happened at the exact
    same time as data coming over the line

    the entire payload could also be variable in length
    */

    delay(5000);
    Serial.begin(9600);
    while (!Serial)
        ;
    UNITY_BEGIN();
    // RUN_TEST(test_piksi_functional);
    piksi.setup();
    // int weird_delay = 100;
    // ensure that atleast one message comes in;
    // this one should error out, no bytes in

    Serial.println("***************************************************************");

    // mimic exact 120 ms control cycle
    int prevtime = micros();
    int posttime = micros();
    int exec_pass_count = 0;
    int timing_pass_count = 0;
    int PIKSI_READ_ALLOTED = 7200;
    for (int i = 0; i < 100; i++) {
        // Serial.println(100 - (millis()-prevtime));
        delay(120 - (micros() - prevtime) / 1000);
        prevtime = micros();

        // count += piksi_fast_read();
        exec_pass_count += execute_piksi_all();
        posttime = micros();
        if (posttime - prevtime < PIKSI_READ_ALLOTED) timing_pass_count++;
        // RUN_TEST(test_piksi_all);
    }

    Serial.printf("EXEC PASS COUNT OF 100: %d\n", exec_pass_count);
    Serial.printf("TIMING PASS COUNT OF 100: %d\n", timing_pass_count);
    RUN_TEST(test_sats);
    UNITY_END();
    return 0;
}
