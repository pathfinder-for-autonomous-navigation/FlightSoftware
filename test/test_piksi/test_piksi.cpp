#include <core_pins.h>
#include <unity.h>
#include <usb_serial.h>
#include <array>
#include <fsw/FCCode/Drivers/Piksi.hpp>

#define PIKSI_READ_ALLOTED 650
#define SAFETY 250
#define RUNS 200
//good readings rely on small control cycles
#define CONTROL_CYCLE 170

Devices::Piksi piksi("piksi", Serial4);
int setup_start_time;
int second_time;
int preread_time;

std::array<double, 3> pos;
std::array<double, 3> vel;
std::array<double, 3> baseline_pos;
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
float ssqrt(std::array<double, 3> in) {
    return (float)sqrt(((float)in[0]) * ((float)in[0]) + ((float)in[1]) * ((float)in[1]) +
                       ((float)in[2]) * ((float)in[2]));
}
bool verify_out() {
    // Verify that last call to process_buffer was successful
    bool ret = (out == 1);

    if (!ret) Serial.printf("Process Out: %d\n", out);

    return ret;
}
bool verify_same_time(){
    bool ret = time.tow == pos_tow && time.tow ==vel_tow && time.tow ==baseline_tow;
    if(!ret){
        Serial.printf("GPS tow: %u\n", time.tow);
        Serial.printf("POS PAST: %u, CURR: %u\n", pos_past, pos_tow);
        Serial.printf("VEL PAST: %u, CURR: %u\n", vel_past, vel_tow);
        Serial.printf("BASE PAST: %u, CURR: %u\n", baseline_past, baseline_tow);


    }
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
void test_time(){

    Serial.printf("GPS week: %d\n", time.wn);
    Serial.printf("GPS OLD tow: %u\n", prev_tow);
    Serial.printf("GPS tow: %u\n", time.tow);
    TEST_ASSERT_TRUE(time.tow > prev_tow);
    
}
bool verify_pos() {
    // Check magnitude of position
    // float pos_mag = sqrt((float)pos[0] * (float)pos[0] + (float)pos[1] * (float)pos[1] +
    //                      (float)pos[2] * (float)pos[2]);
    float pos_mag = ssqrt(pos);
    bool ret = (piksi.get_pos_ecef_nsats() > 3);
    ret = ret && comp(6.37E6, pos_mag, 5.0E3);

    // Check flag
    // 1 means float rtk
    ret = ret && (piksi.get_pos_ecef_flags() == 1);

    ret = ret && pos_tow > pos_past;

    if (!ret) {
        Serial.printf("GPS position: %g,%g,%g\n", pos[0], pos[1], pos[2]);
        Serial.printf("position_mag: %g\n", pos_mag);
        Serial.printf("pos_ecef_flags: %u\n", piksi.get_pos_ecef_flags());

        Serial.printf("Nsat Pos: %d\n", piksi.get_pos_ecef_nsats());

        Serial.printf("PAST: %u, CURR: %u\n", pos_past, pos_tow);
    }

    return ret;
}
void test_pos(){
    // Check magnitude of position
    // float pos_mag = sqrt((float)pos[0] * (float)pos[0] + (float)pos[1] * (float)pos[1] +
    //                      (float)pos[2] * (float)pos[2]);
    float pos_mag = ssqrt(pos);
    TEST_ASSERT_TRUE(piksi.get_pos_ecef_nsats() > 3);
    TEST_ASSERT_TRUE(comp(6.37E6, pos_mag, 5.0E3));

    // Check flag
    // 1 means float rtk
    // TEST_ASSERT_TRUE(piksi.get_pos_ecef_flags() == 1);

    Serial.printf("GPS position: %g,%g,%g\n", pos[0], pos[1], pos[2]);
    Serial.printf("position_mag: %g\n", pos_mag);
    Serial.printf("pos_ecef_flags: %u\n", piksi.get_pos_ecef_flags());

    Serial.printf("Nsat Pos: %d\n", piksi.get_pos_ecef_nsats());

    Serial.printf("PAST: %u, CURR: %u\n", pos_past, pos_tow);
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
        Serial.printf("Vel: %g,%g,%g\n", vel[0], vel[1], vel[2]);
        Serial.printf("velocity_mag: %g\n", vel_mag);
        Serial.printf("vel_ecef_flags: %u\n", piksi.get_vel_ecef_flags());

        Serial.printf("Nsat Vel: %d\n", piksi.get_vel_ecef_nsats());
        Serial.printf("VEL PAST: %u, CURR: %u\n", vel_past, vel_tow);
    }

    return ret;
}
void test_vel() {
    // Check mangitude of velocity
    float vel_mag = ssqrt(vel);

    // units of mm/s btw
    TEST_ASSERT_TRUE(piksi.get_vel_ecef_nsats() > 3);
    TEST_ASSERT_TRUE(comp(3.9E3, vel_mag, 7.0E2));

    //ret = ret && (piksi.get_vel_ecef_flags() == 0);
    TEST_ASSERT_TRUE(vel_tow > vel_past);

    Serial.printf("Vel: %g,%g,%g\n", vel[0], vel[1], vel[2]);
    Serial.printf("velocity_mag: %g\n", vel_mag);
    Serial.printf("vel_ecef_flags: %u\n", piksi.get_vel_ecef_flags());

    Serial.printf("Nsat Vel: %d\n", piksi.get_vel_ecef_nsats());
    Serial.printf("PAST: %u, CURR: %u\n", vel_past, vel_tow);
}
bool verify_baseline() {
    // float baseline_mag =
    //     sqrt(baseline_pos[0] * baseline_pos[0] + baseline_pos[1] * baseline_pos[1] +
    //          baseline_pos[2] * baseline_pos[2]);
    float baseline_mag = ssqrt(baseline_pos);

    bool ret = comp(1.0E5, baseline_mag, 2E3);
    ret = ret && piksi.get_baseline_ecef_nsats() > 3;

    // 1 means fixed RTK
    ret = ret && piksi.get_baseline_ecef_flags() == 1;
    ret = ret && baseline_tow > baseline_past;

    if (!ret) {
        Serial.printf("GPS baseline position: %g,%g,%g", baseline_pos[0], baseline_pos[1],
                      baseline_pos[2]);
        Serial.printf("baseline_mag: %g\n", baseline_mag);
        Serial.printf("baseline_ecef_flags: %u\n", piksi.get_baseline_ecef_flags());
        Serial.printf("Nsat baseline: %d\n", piksi.get_baseline_ecef_nsats());
        Serial.printf("BASE PAST: %u, CURR: %u\n", baseline_past, baseline_tow);
    }
    return ret;
    
}
void test_baseline() {
    // float baseline_mag =
    //     sqrt(baseline_pos[0] * baseline_pos[0] + baseline_pos[1] * baseline_pos[1] +
    //          baseline_pos[2] * baseline_pos[2]);
    float baseline_mag = ssqrt(baseline_pos);

    TEST_ASSERT_TRUE(comp(1.0E5, baseline_mag, 2E3));
    TEST_ASSERT_TRUE(piksi.get_baseline_ecef_nsats() > 3);

    // 1 means fixed RTK
    //ret = ret && piksi.get_baseline_ecef_flags() == 1;
    TEST_ASSERT_EQUAL(piksi.get_baseline_ecef_flags(), 1);
    TEST_ASSERT_TRUE(baseline_tow > baseline_past);

    Serial.printf("GPS baseline position: %g,%g,%g", baseline_pos[0], baseline_pos[1],
                    baseline_pos[2]);
    Serial.printf("baseline_mag: %g\n", baseline_mag);
    Serial.printf("baseline_ecef_flags: %u\n", piksi.get_baseline_ecef_flags());
    Serial.printf("Nsat basline: %d\n", piksi.get_baseline_ecef_nsats());
    Serial.printf("PAST: %u, CURR: %u\n", baseline_past, baseline_tow);
}
bool verify_iar() {
    // experimentally during sim, iar gshould == 0
    bool ret = iar == 0;

    if (!ret) {
        Serial.printf("IAR: %u\n", iar);
    }

    return ret;
}
void test_iar() {
    // experimentally during sim, iar should == 0
    TEST_ASSERT_EQUAL(iar, 0);

    Serial.printf("IAR: %u\n", iar);
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
/**
 * @brief This method actually calls the getters of Piksi,
 * This will only make sense if read_buffer was called just before.
 * Otherwise, it will pull OLD data.
 * 
 * Currently in use by the test script below
 * 
 * @return true If data is good and new
 * @return false If otherwise
 */
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
    
    bool ret = verify_same_time() & verify_time() & verify_baseline() & verify_pos() & verify_vel();// & verify_iar();
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
void test_validate_buffer() {
    

    prev_tow = time.tow;

    piksi.get_gps_time(&time);

    TEST_ASSERT_TRUE(verify_time());

    pos_past = pos_tow;
    vel_past = vel_tow;
    baseline_past = baseline_tow;

    piksi.get_pos_ecef(&pos_tow, &pos);
    piksi.get_vel_ecef(&vel_tow, &vel);
    piksi.get_baseline_ecef(&baseline_tow, &baseline_pos);

    iar = piksi.get_iar();

    TEST_ASSERT_TRUE(verify_baseline());
    TEST_ASSERT_TRUE(verify_pos());
    TEST_ASSERT_TRUE(verify_vel());
    TEST_ASSERT_TRUE(verify_iar());

    //bool ret = verify_time() & verify_baseline() & verify_pos() & verify_vel() & verify_iar();
    //return ret;
}

void get_data(){
    prev_tow = time.tow;

    piksi.get_gps_time(&time);

    pos_past = pos_tow;
    vel_past = vel_tow;
    baseline_past = baseline_tow;

    piksi.get_pos_ecef(&pos_tow, &pos);
    piksi.get_vel_ecef(&vel_tow, &vel);
    piksi.get_baseline_ecef(&baseline_tow, &baseline_pos);

    iar = piksi.get_iar();
}

void print_time(){
    Serial.printf("GPS week: %d\n", time.wn);
    Serial.printf("GPS OLD tow: %u\n", prev_tow);
    Serial.printf("GPS tow: %u\n", time.tow);
}
void print_pos(){
    float pos_mag = ssqrt(pos);

    Serial.printf("GPS position: %g,%g,%g\n", pos[0], pos[1], pos[2]);
    Serial.printf("position_mag: %g\n", pos_mag);
    Serial.printf("pos_ecef_flags: %u\n", piksi.get_pos_ecef_flags());
    Serial.printf("Nsat Pos: %d\n", piksi.get_pos_ecef_nsats());
    Serial.printf("PAST: %u, CURR: %u\n", pos_past, pos_tow);
}
void print_vel(){
    float vel_mag = ssqrt(vel);

    Serial.printf("Vel: %g,%g,%g\n", vel[0], vel[1], vel[2]);
    Serial.printf("velocity_mag: %g\n", vel_mag);
    Serial.printf("vel_ecef_flags: %u\n", piksi.get_vel_ecef_flags());

    Serial.printf("Nsat Vel: %d\n", piksi.get_vel_ecef_nsats());
    Serial.printf("PAST: %u, CURR: %u\n", vel_past, vel_tow);
}
void print_baseline(){
    float baseline_mag = ssqrt(baseline_pos);

    Serial.printf("GPS baseline position: %g,%g,%g", baseline_pos[0], baseline_pos[1],
                    baseline_pos[2]);
    Serial.printf("baseline_mag: %g\n", baseline_mag);
    Serial.printf("baseline_ecef_flags: %u\n", piksi.get_baseline_ecef_flags());
    Serial.printf("Nsat basline: %d\n", piksi.get_baseline_ecef_nsats());
    Serial.printf("PAST: %u, CURR: %u\n", baseline_past, baseline_tow);
}
void print_all(){
    
    print_time();
    print_pos();
    print_vel();
    print_baseline();
}

int main(void) {

    delay(5000);
    Serial.begin(9600);
    while (!Serial)
        ;
    UNITY_BEGIN();

    piksi.setup();

    Serial.println("***************************************************************");

    
    int start_millis = millis();

    //this initial loop is just for debugging, verify that there are indeed bytes coming over the line
    while(millis() - start_millis < 200){

        Serial.printf("TIME: %d ", millis() - start_millis);
        Serial.printf("BYTES: %d\n", piksi.bytes_available());
        delay(20);

    }
    Serial.println("FINISH WHILE LOOP");

    //bool keeprun = true;

    // while(keeprun){
    //     int exec_ret = piksi.read_buffer_exp();
    //     Serial.printf("RES: %d\n", exec_ret);

    //     if(exec_ret == 0){
    //         RUN_TEST(test_get_data);
    //         keeprun = false;
    //     }

    //     delay(120);
    // }

    int prevtime = micros();
    int posttime = micros();
    int exec_pass_count = 0;
    int timing_pass_count = 0;
    int msg_len_fail_count = 0;
    int data_fail_count = 0;
    int bad_buffer = 0;
    int read_sum = 0;
    int max_exec_time = 0;

    std::array<int, 6> read_ct{};

    for (int i = 0; i < RUNS; i++) {
        //this syncs up command to execute exactly every 120 ms 
        //(barring a failure on the previous loop)
        //Serial.printf("delay: %d\n", CONTROL_CYCLE - (micros() - prevtime) / 1000);

        delay(CONTROL_CYCLE - (micros() - prevtime) / 1000);

        //SPP only
        //firmware 1: 159 bytes

        //SAT addition
        //firmware 3: 159 bytes

        //RTK Float
        //firmware 7: 159 bytes


        Serial.printf("BYTES AVAIL: %u\n",piksi.bytes_available());
        
        prevtime = micros();
        int res = piksi.read_all();
        posttime = micros();

        if(posttime - prevtime > max_exec_time){
            max_exec_time = posttime - prevtime;
        }
        read_sum += res;
        read_ct[res] += 1;


        Serial.printf("RES: %d\n", res);
        // if (res == 1) {
        //     if(verify_all())
        //         exec_pass_count += 1;
        //     else
        //         data_fail_count += 1;
            
        // }

        // else if(res == 2){
        //     msg_len_fail_count += 1;
        // }
        // else if(res == 0)
        //     bad_buffer += 1;

        get_data();
        print_all();

        Serial.printf("EXEC TIME: %d micros\n", posttime-prevtime);
        if (posttime - prevtime < PIKSI_READ_ALLOTED) {
            timing_pass_count++;
        } else {
            Serial.printf("EXCEED: %d\n", posttime - prevtime);
        }
    }
    Serial.println("\n********************************************");

    Serial.printf("MSG LEN FAIL COUNT: %d\n", msg_len_fail_count);
    Serial.printf("EXEC PASS COUNT: %d\n", exec_pass_count);
    Serial.printf("VERIFY FAIL COUNT: %d\n", data_fail_count);
    Serial.printf("BAD BUFFER COUNT: %d\n", bad_buffer);
    Serial.printf("TIMING PASS COUNT: %d\n", timing_pass_count);

    Serial.printf("MAX EXEC TIME: %d\n", max_exec_time);
    Serial.printf("READSUM AVG: %f\n", (float)read_sum/(float)RUNS);

    for(int i = 0; i<6;i++)
    Serial.printf("OUT %d: %d\n",i,read_ct[i]);


    /*

    Looking for these things in the output (assumed to test for 200 control cycles):

    EXEC TIME: Is the time each ReadPiksiControlTask would take
        looking for it to be under 250 micros.
        Nominally it is around 213 for the 299 byte packets, and 239 micros for the 333 byte packets.

    MSG LEN FAIL COUNT: Number of times data came through to the buffer 
        WHILE a the process buffer loop was executing
        this is rare, but not non zero, expect to see a 0 1 or 2 at max. 
        if it occurs, it is handled gracefully, you need not worry.

    EXEC PASS COUNT: Number of times it successfully read data from the buffer
        Expect about half of all test control tasks to pass

    VERIFY FAIL COUNT: Number of times buffer was properly processed, 
        BUT the data was not within expected bounds or flags

    BAD BUFFER COUNT: Number of times there was not a clean entire packet in the buffer
        Expect about half of all test control tasks to fail this

    TIMING PASS COUNT: Number of executions that succesfully executed within the time limit.

    EXCEED: This line will only print if a simulated control cycle exceeds the defined bounds.
    */

    RUN_TEST(test_sats);
    UNITY_END();
    return 0;
}
