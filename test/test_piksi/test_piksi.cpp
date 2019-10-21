#include <core_pins.h>
#include <unity.h>
#include <usb_serial.h>
#include <array>
#include "../lib/Drivers/Piksi.hpp"

Devices::Piksi piksi("piksi", Serial4);
int setup_start_time;
int second_time;
int preread_time;

// assume piksi already setup
void test_piksi_manyreading_fast() {
    std::array<double, 3> pos = {0};
    std::array<double, 3> vel = {0};
    // Serial.printf("Preread val: %d\n", pos[0]);
    preread_time = millis();
    Serial.println("EVERYTHING: Attempting to get solution...");

    // while (!piksi.process_buffer() || pos[0] == 0) {
    // bool keeprun = true;

    int out = -5;
    msg_gps_time_t prevtime;
    piksi.get_gps_time(&prevtime);

    unsigned int currenttow = prevtime.tow;
    unsigned int temptow;

    Serial.printf("TOW INIT: %u\n", prevtime.tow);
    Serial.printf("BYTES AVAIL: %u\n", piksi.bytes_available());
    // while
    while (currenttow == prevtime.tow) {
        currenttow = UINT_MAX;

        out = piksi.process_buffer();
        piksi.get_pos_ecef(&temptow, &pos);
        // sets current tow to be the minimum of all measurements
        currenttow = std::min(currenttow, temptow);

        piksi.get_vel_ecef(&temptow, &vel);
        currenttow = std::min(currenttow, temptow);

        // set below .5 ms for nyquist
        delayMicroseconds(100);
    }
    Serial.printf("PROCESS BUFF OUT: %hi\n", out);

    Serial.printf("Read time: %d ms\n", millis() - preread_time);
    Serial.printf("GPS position: %lf,%lf,%lf\n", pos[0], pos[1], pos[2]);
    double pos_mag = sqrt(pos[0] * pos[0] + pos[1] * pos[1] + pos[2] * pos[2]);
    TEST_ASSERT_DOUBLE_WITHIN(1E3, 6.37E6, pos_mag);  // We're somewhere on Earth.
    // TEST_ASSERT_GREATER_THAN(4, piksi.get_pos_ecef_nsats()); // We need at least 4 satellites to
    // get position.

    Serial.printf("Vel position: %lf,%lf,%lf\n", vel[0], vel[1], vel[2]);
    double vel_mag = sqrt(vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2]);
    TEST_ASSERT_DOUBLE_WITHIN(1E3, 3.9E3, vel_mag);  // We're fast?
    // TEST_ASSERT_GREATER_THAN(4, piksi.get_pos_ecef_nsats()); // We need at least 4 satellites to
    // get position.
}

// assume piksi already setup
void test_piksi_manyreading() {
    std::array<double, 3> pos = {0};
    std::array<double, 3> vel = {0};
    msg_gps_time_t time;

    // Serial.printf("Preread val: %d\n", pos[0]);
    preread_time = millis();
    Serial.println("EVERYTHING: Attempting to get solution...");
    Serial.printf("BYTES AVAIL: %u\n", piksi.bytes_available());

    int out = -5;
    //tune parameters?
    if (piksi.bytes_available() >= 299 && piksi.bytes_available()<599) {
        while (piksi.bytes_available()) {
            piksi.process_buffer();
            delayMicroseconds(100);
        }
        piksi.get_pos_ecef(&pos);
        piksi.get_vel_ecef(&vel);
        piksi.get_gps_time(&time);

        Serial.printf("PROCESS BUFF OUT: %hi\n", out);

        Serial.printf("GPS time: %u\n", time.tow);
        Serial.printf("GPS position: %lf,%lf,%lf\n", pos[0], pos[1], pos[2]);
        double pos_mag = sqrt(pos[0] * pos[0] + pos[1] * pos[1] + pos[2] * pos[2]);
        TEST_ASSERT_DOUBLE_WITHIN(1E3, 6.37E6, pos_mag);  // We're somewhere on Earth.
        // TEST_ASSERT_GREATER_THAN(4, piksi.get_pos_ecef_nsats()); // We need at least 4 satellites
        // to get position.

        Serial.printf("Vel position: %lf,%lf,%lf\n", vel[0], vel[1], vel[2]);
        double vel_mag = sqrt(vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2]);
        TEST_ASSERT_DOUBLE_WITHIN(1E3, 3.9E3, vel_mag);  // We're fast?
        // TEST_ASSERT_GREATER_THAN(4, piksi.get_pos_ecef_nsats()); // We need at least 4 satellites
        // to get position.
    }

    else {
        // if no data in buffer throw error
        Serial.println("NOT A CLEAN READ");

        // getrid of extra bytes:
        // while(piksi.bytes_available()){
        //     piksi.process_buffer();
        // }
        

        TEST_ASSERT_TRUE(true);
    }
    if(piksi.bytes_available()){
        Serial.println("KILLING EXTRA BYTES");
    }
    while (piksi.bytes_available()) {
            piksi.clear_bytes();
    }
    Serial.printf("Read time: %d ms\n", millis() - preread_time);
    Serial.println();

}

void test_sats() {
    preread_time = millis();

    TEST_ASSERT_GREATER_THAN(
        4, piksi.get_pos_ecef_nsats());  // We need at least 4 satellites to get position.
    Serial.printf("Num Sats Read Time: %d ms\n", millis() - preread_time);
}

void test_piksi_functional() {
    setup_start_time = millis();
    piksi.setup();
    Serial.println("Attempting to get solution...");

    std::array<double, 3> pos = {0};
    // while (!piksi.process_buffer() || pos[0] == 0) {
    while (piksi.process_buffer() == SBP_OK || pos[0] == 0) {
        delayMicroseconds(1000);
        piksi.get_pos_ecef(&pos);
    };

    Serial.printf("Setup time: %d ms\n", millis() - setup_start_time);
    Serial.printf("GPS position: %lf,%lf,%lf\n", pos[0], pos[1], pos[2]);
    double pos_mag = sqrt(pos[0] * pos[0] + pos[1] * pos[1] + pos[2] * pos[2]);
    TEST_ASSERT_DOUBLE_WITHIN(1E3, 6.37E6, pos_mag);  // We're somewhere on Earth.
    // TEST_ASSERT_GREATER_THAN(4, piksi.get_pos_ecef_nsats()); // We need at least 4 satellites to
    // get position.

    // second_time = millis();

    // pos = {0,0,0};
    // //while (!piksi.process_buffer() || pos[0] == 0) {
    // while (piksi.process_buffer() == SBP_OK || pos[0] ==0 ) {
    //     delayMicroseconds(1000);
    //     piksi.get_pos_ecef(&pos);
    // };

    // Serial.printf("Second time: %d ms\n", millis() - second_time);
    // Serial.printf("GPS position: %lf,%lf,%lf\n", pos[0], pos[1], pos[2]);
    // pos_mag = sqrt(pos[0]*pos[0] + pos[1]*pos[1] + pos[2]*pos[2]);
    // TEST_ASSERT_DOUBLE_WITHIN(1E3, 6.37E6, pos_mag); // We're somewhere on Earth.
    // TEST_ASSERT_GREATER_THAN(4, piksi.get_pos_ecef_nsats()); // We need at least 4 satellites to
    // get position.
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
    int weird_delay = 100;
    // ensure that atleast one message comes in;
    //this one should error out, no bytes in
    RUN_TEST(test_piksi_manyreading);
    
    Serial.println("***************************************************************");

    //mimic exact 100 ms control cycle
    int prevtime = millis();
    for(int i = 0;i<20;i++){
        //Serial.println(100 - (millis()-prevtime));
        delay(100 - (millis()-prevtime));
        prevtime = millis();
        RUN_TEST(test_piksi_manyreading);
        
    }

    delay(weird_delay);
    RUN_TEST(test_piksi_manyreading);
    delay(weird_delay);
    RUN_TEST(test_piksi_manyreading);
    delay(weird_delay);
    RUN_TEST(test_piksi_manyreading);
    delay(weird_delay);
    RUN_TEST(test_piksi_manyreading);
    delay(weird_delay);
    RUN_TEST(test_piksi_manyreading);
    delay(weird_delay);
    RUN_TEST(test_piksi_manyreading);
    delay(weird_delay);
    RUN_TEST(test_piksi_manyreading);
    delay(weird_delay);
    RUN_TEST(test_piksi_manyreading);

    Serial.println("***************************************************************");

    delay(weird_delay);
    delay(weird_delay);
    RUN_TEST(test_piksi_manyreading);
    delay(weird_delay);
    delay(weird_delay);
    RUN_TEST(test_piksi_manyreading);
    delay(weird_delay);
    delay(weird_delay);
    RUN_TEST(test_piksi_manyreading);

    // delay(weird_delay);
    // RUN_TEST(test_piksi_fastread_pos);
    // delay(weird_delay);
    // RUN_TEST(test_piksi_fastread_pos);
    // delay(weird_delay);
    // RUN_TEST(test_piksi_fastread_pos);

    // Serial.println("****************************************************");
    // // set wd = 0 for max fast calls;
    // // tho response time is limited by internall condition that
    // // tow != prevtow
    // int wd = 0;

    // delay(wd);
    // RUN_TEST(test_piksi_fast_vel);
    // delay(wd);
    // RUN_TEST(test_piksi_fast_vel);
    // delay(wd);
    // RUN_TEST(test_piksi_fast_vel);
    // delay(weird_delay);
    // RUN_TEST(test_sats);

    // delay(weird_delay);
    RUN_TEST(test_sats);
    UNITY_END();
    return 0;
}
