#include <core_pins.h>
#include <unity.h>
#include <usb_serial.h>
#include <array>
#include "../lib/Drivers/Piksi.hpp"

#define PIKSI_READ_ALLOTED 1000
#define SAFETY 250
#define CONTROL_CYCLE 120

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
unsigned int iar;

unsigned int pos_past;
unsigned int pos_tow;

unsigned int vel_past;
unsigned int vel_tow;

unsigned int baseline_past;
unsigned int baseline_tow;

bool bad_bytes;

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
        Serial.printf("GPS week: %d\n", time.wn);
        Serial.printf("GPS OLD tow: %u\n", prev_tow);
        Serial.printf("GPS tow: %u\n", time.tow);
    }

    return ret;
}
bool verify_pos() {
    // Check magnitude of position
    // float pos_mag = sqrt((float)pos[0] * (float)pos[0] + (float)pos[1] * (float)pos[1] +
    //                      (float)pos[2] * (float)pos[2]);
    float pos_mag = ssqrt(pos);
    bool ret = (piksi.get_pos_ecef_nsats() > 3);
    ret = ret && comp(6.37E6, pos_mag, 5.0E3);

    // Check flag
    // ret = ret && (piksi.get_pos_ecef_flags() == 1);

    ret = ret && pos_tow > pos_past;

    if (!ret) {
        Serial.printf("GPS position: %d,%d,%d\n", pos[0], pos[1], pos[2]);
        Serial.printf("position_mag: %g\n", pos_mag);
        Serial.printf("pos_ecef_flags: %d\n", piksi.get_pos_ecef_flags());

        Serial.printf("Nsat Pos: %d\n", piksi.get_pos_ecef_nsats());

        Serial.printf("PAST: %u, CURR: %u\n", pos_past, pos_tow);
    }

    return ret;
}
bool verify_vel() {
    // Check mangitude of velocity
    float vel_mag = ssqrt(vel);

    // units of mm/s btw
    bool ret = (piksi.get_vel_ecef_nsats() > 3);
    ret = ret && comp(3.9E3, vel_mag, 7.0E2);

    ret = ret && (piksi.get_vel_ecef_flags() == 0);
    ret = ret && vel_tow > vel_past;

    if (!ret) {
        Serial.printf("Vel: %d,%d,%d\n", vel[0], vel[1], vel[2]);
        Serial.printf("velocity_mag: %g\n", vel_mag);
        Serial.printf("vel_ecef_flags: %d\n", piksi.get_vel_ecef_flags());

        Serial.printf("Nsat Vel: %d\n", piksi.get_vel_ecef_nsats());
        Serial.printf("PAST: %u, CURR: %u\n", vel_past, vel_tow);
    }

    return ret;
}
bool verify_baseline() {
    // float baseline_mag =
    //     sqrt(baseline_pos[0] * baseline_pos[0] + baseline_pos[1] * baseline_pos[1] +
    //          baseline_pos[2] * baseline_pos[2]);
    float baseline_mag = ssqrt(baseline_pos);

    bool ret = comp(1.0E5, baseline_mag, 2E3);
    ret = ret && piksi.get_baseline_ecef_nsats() > 3;
    ret = ret && piksi.get_baseline_ecef_flags() == 1;
    ret = ret && baseline_tow > baseline_past;

    if (!ret) {
        Serial.printf("GPS baseline position: %d,%d,%d\n", baseline_pos[0], baseline_pos[1],
                      baseline_pos[2]);
        Serial.printf("baseline_mag: %g\n", baseline_mag);
        Serial.printf("baseline_ecef_flags: %d\n", piksi.get_baseline_ecef_flags());
        Serial.printf("Nsat basline: %d\n", piksi.get_baseline_ecef_nsats());
        Serial.printf("PAST: %u, CURR: %u\n", baseline_past, baseline_tow);
    }
    return ret;
}
bool verify_iar() {
    // experimentally during sim, iar should == 0
    bool ret = iar == 0;

    if (!ret) {
        Serial.printf("IAR: %u\n", iar);
    }

    return ret;
}
// assume piksi already setup
int execute_piksi_all() {
    // Serial.printf("Preread val: %d\n", pos[0]);
    // preread_time = micros();
    // Serial.println("EVERYTHING: Attempting to get solution...");
    // Serial.printf("BYTES AVAIL: %u\n", piksi.bytes_available());
    //bool ret = false;
    //int pbuff_count = 0;

    //unsigned char msg_len = 0;
    // tune parameters?
    //
    // CANNOT DO THIS, MESSAGES WILL VARY IN LENGTH
    // COULD BE A CASE WHERE ONCE IN SPACE, MESSAGE LENGTH ALWAYS NOT 299
    // if (piksi.bytes_available() == 299) {
    int initial_bytes = piksi.bytes_available();
    // if (initial_bytes >= 200 && initial_bytes < 599) {

    if (initial_bytes == 299 || initial_bytes == 333) {
        // Serial.printf("BYTES: %d\n", initial_bytes);
        //Serial.printf("BYTES: %d ", initial_bytes);

        // abnormal byte number:

        // if (!(piksi.bytes_available() >= 180 && piksi.bytes_available() < 599))
        //     Serial.printf("BYTES: %d\n", piksi.bytes_available());

        // time to read one burst is like 275 ms

        //int pre_loop = piksi.bytes_available();
        int msg_len_sum = 0;

        // loop can finish before new bytes added?
        //while (piksi.bytes_available() && (micros() - preread_time < (PIKSI_READ_ALLOTED - SAFETY))) {
        while (piksi.bytes_available()){

            msg_len_sum += piksi.process_buffer_msg_len();
             
            // if (micros() - preread_time > (PIKSI_READ_ALLOTED - SAFETY)) {
            //     bad_bytes = true;
            //     // print this if we're probably being interrupted by a piksi scream
            //     Serial.print("BAD FEELING ");
            // }
        }
        
        if (initial_bytes == 299 && msg_len_sum != 227){
            Serial.print("Process wrong # ");
            return -1;
        }
        else if (initial_bytes == 333 && msg_len_sum != 245){
            Serial.print("Process wrong # ");
            return -1;
        }
        else
        {
            return 1;
        }
        
            
        // ret = verify_out() & verify_time() & verify_baseline() & verify_pos() & verify_vel() &
        //         verify_iar();
        
        // Serial.printf("TOW: %u pos: %u ", time.tow, pos_tow);
        // sum should be 227 or 245
        // Serial.printf("SUM: %d ",msg_len_sum);
        

    }

    else {
        // if no data in buffer throw error
        //Serial.print("Clearing bytes ");
        while (piksi.bytes_available()) {
            piksi.clear_bytes();
            //delayMicroseconds(10);
        }
        return 0;
    }
}
bool verify_all(){
    prev_tow = time.tow;

    piksi.get_gps_time(&time);

    pos_past = pos_tow;
    vel_past = vel_tow;
    baseline_past = baseline_tow;

    piksi.get_pos_ecef(&pos_tow, &pos);
    piksi.get_vel_ecef(&vel_tow, &vel);
    piksi.get_baseline_ecef(&baseline_tow, &baseline_pos);

    iar = piksi.get_iar();

    bool ret = verify_time() & verify_baseline() & verify_pos() & verify_vel() & verify_iar();
    return ret;
}
void test_get_data(){
    prev_tow = time.tow;

    piksi.get_gps_time(&time);

    pos_past = pos_tow;
    vel_past = vel_tow;
    baseline_past = baseline_tow;

    piksi.get_pos_ecef(&pos_tow, &pos);
    piksi.get_vel_ecef(&vel_tow, &vel);
    piksi.get_baseline_ecef(&baseline_tow, &baseline_pos);

    iar = piksi.get_iar();

    bool ret = verify_time() & verify_baseline() & verify_pos() & verify_vel() & verify_iar();
    TEST_ASSERT_TRUE(ret);
}
void test_sats() {
    preread_time = millis();

    TEST_ASSERT_GREATER_THAN(
        4, piksi.get_pos_ecef_nsats());  // We need at least 4 satellites to get position.
    Serial.printf("Num Sats Read Time: %d micros\n", millis() - preread_time);
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
    // int start_time = micros();
    int prevtime = micros();
    int posttime = micros();
    int exec_pass_count = 0;
    int timing_pass_count = 0;
    int msg_len_fail_count = 0;

    // while(micros()-start_time < 1000*500){

    //     Serial.printf("TIME: %d ", micros() - start_time);
    //     Serial.printf("BYTES: %d\n", piksi.bytes_available());
    //     delayMicroseconds(1000);

    //     //83135
    //     //57233
    // }

    for (int i = 0; i < 200; i++) {
        // Serial.println(100 - (millis()-prevtime));
        delay(CONTROL_CYCLE - (micros() - prevtime) / 1000);
        prevtime = micros();

        // count += piksi_fast_read();
        int res = piksi.read_buffer();
        if (res == 1 && verify_all()) {
            exec_pass_count += 1;
            // Serial.print("PASS \n");
        }
        else if(res == -1){
            msg_len_fail_count += 1;
        }

        posttime = micros();
        Serial.printf("EXEC TIME: %d micros\n", posttime-prevtime);
        if (posttime - prevtime < PIKSI_READ_ALLOTED) {
            timing_pass_count++;
        } else {
            Serial.printf("EXCEED: %d\n", posttime - prevtime);
        }
        // RUN_TEST(test_piksi_all);
    }

    Serial.printf("MSG LEN FAIL COUNT: %d\n", msg_len_fail_count);

    Serial.printf("EXEC PASS COUNT: %d\n", exec_pass_count);
    Serial.printf("TIMING PASS COUNT: %d\n", timing_pass_count);
    RUN_TEST(test_sats);
    UNITY_END();
    return 0;
}
