
#include "../state/state_holder.hpp"
#include "../state/state_history_holder.hpp"
#include "../comms/downlink_serializer.hpp"
#include "../state/state_history_rates.hpp"
#include "adcs_threads.h"
#include <Arduino.h>

// Concurrency Variables
static mutex_t cnt_lock;
static condition_variable_t producers_done;
static condition_variable_t consumer_done;

thread_t *adcs_threads::consumer_thd;
thread_t *adcs_threads::prod_threads[PRODUCER_COUNT];
// Producer/Consumer Variables
size_t producer_cnt = 0;
bool prod_ready[PRODUCER_COUNT];

THD_WORKING_AREA(adcs_threads::waConsumerThd, 16384);
THD_WORKING_AREA(adcs_threads::waProd[PRODUCER_COUNT], 128);

void blink()
{
  // digitalWrite(13, HIGH);
  // delay(500);
  // digitalWrite(13, LOW);
  // delay(500);
}

template <size_t data_size, size_t history_size, typename T>
struct Data
{
  using data_t = const std::array<T, data_size> &;
  using state_history_t = circular_buffer<std::array<T, data_size>, history_size> &;

  Data(const data_t &data_p, state_history_t &state_history_p,
       unsigned int state_history_rate, int id)
      : data_p(data_p),
        state_history_p(state_history_p),
        state_history_rate(state_history_rate), id(id) {}

  data_t data_p;
  state_history_t state_history_p;
  unsigned int state_history_rate;
  size_t cur_index;
  int id;
}; // 20 bytes

/*
 * Each Producer thread is passed a Data object containing references to their data_buf and
 * state_history. They retrieve data some number of times before acquiring the
 * mutex for producer_cnt and then incrementing it. The last producer signals
 * the consumer.
 */
template <size_t data_size, size_t history_size, typename T>
void adcs_threads::producer(void *vargp)
{
  Data<data_size, history_size, T> *data = (Data<data_size, history_size, T> *)vargp;
  while (true)
  {
    // Serial.printf("producer start (%d)\n", data->id);
    while (data->cur_index < history_size)
    {
      // Copy the contents of data->data_buf into state_history
      // debug_printf("Producer waiting: [%d]\n", data->id);
      data->state_history_p.put(data->data_p);
      data->cur_index += 1;

    /// chThdSleepMilliseconds(MS2ST(data->state_history_rate));
    }
    // lock and then increment producer count
    // for some reason producer is unable to obtained the lock here.
    chMtxLock(&cnt_lock);

    // debug_printf("Producer acquire lock: [%d]\n", data->id);
    producer_cnt++;
    // Signal consumers to start making packets if this is the last producer
    if (producer_cnt >= PRODUCER_COUNT)
    {
      chCondSignal(&producers_done);
      // debug_printf("Producer signal (%d)\n", data->id);
    }
    // Producers wait on consumer_done, take cnt_lock and then immediately
    // unlock and give to the next producer
    prod_ready[data->id] = false;
    // debug_printf("Producer start waiting [%d]\n", data->id);
    while (prod_ready[data->id] == false)
      chCondWait(&consumer_done);

    // debug_printf("Producer acquired lock again: [%d]\n", data->id);
    chMtxUnlock(&cnt_lock);

    // Reset state_history to avoid stack space issues
    data->state_history_p.reset();
    data->cur_index = 0;
    // blink();

  }
}

static std::bitset<Comms::PACKET_SIZE_BITS> packets[Comms::NUM_PACKETS];
/*
 * Consumer waits for the last producer to signal that it has finished writing
 * to its own state history.
 */
void adcs_threads::consumer(void *vargp)
{
  unsigned int downlink_no = 0;
  while (true)
  {
    // digitalWrite(13, LOW);
    // delay(1000);
    chMtxLock(&cnt_lock);
    // wait for producers to be ready
    //  debug_printf("\nConsumer Waiting\n");
    while (producer_cnt != PRODUCER_COUNT)
    {
      chCondWait(&producers_done);
    }
    producer_cnt = 0; // reset producer count
    for (size_t i = 0; i < PRODUCER_COUNT; i++)
      prod_ready[i] = true;

   // Comms::serialize_downlink(packets, downlink_no);
    Serial.printf("Done serializing downlink_no packet: (%d)\n", downlink_no);
    Serial.printf("\n--------------\nConsumer Thd: %d\n Magno: %d\n Gyro: %d \n Attitude: %d\n---------------\n",
                  chUnusedThreadStack(waConsumerThd, sizeof(waConsumerThd)),
                  chUnusedThreadStack(waProd[0], sizeof(waProd[0])),
                  chUnusedThreadStack(waProd[1], sizeof(waProd[1])),
                  chUnusedThreadStack(waProd[2], sizeof(waProd[2])));
    ++downlink_no;
    chCondBroadcast(&consumer_done);
    chMtxUnlock(&cnt_lock);
  }
}

template <size_t data_size, size_t history_size, typename T>
thread_t *adcs_threads::deployProducers(std::array<T, data_size> data,
                                        circular_buffer<std::array<T, data_size>, history_size> history,
                                        size_t rate,
                                        size_t i, Data<data_size, history_size, T>& d)
{
  return chThdCreateStatic(adcs_threads::waProd[i], sizeof(adcs_threads::waProd[i]),
                           NORMALPRIO + 1, adcs_threads::producer<data_size, history_size, T>, (void *)&d);
}

void adcs_threads::init()
{
  pinMode(13, OUTPUT);
  debug_println("Initializing...");
  chCondObjectInit(&producers_done);
  chCondObjectInit(&consumer_done);
  chMtxObjectInit(&cnt_lock);

  producer_cnt = 0;
  // producer thread 1
  size_t i = 0;
  auto datt = Data<4, 10, float>(State::ADCS::cmd_attitude, StateHistory::ADCS::attitude_cmd_history, StateHistory::Rates::ADCS::attitude_cmd_history, i);
  adcs_threads::prod_threads[i] = chThdCreateStatic(waProd[i], sizeof(waProd[i]), NORMALPRIO + 1, adcs_threads::producer<4, 10, float>, (void *)&datt);
i++;
  // producer thread 2
  auto dmag = Data<3, 10, float>(State::ADCS::mag_data, StateHistory::ADCS::magnetometer_history, StateHistory::Rates::ADCS::magnetometer_history,i);
  adcs_threads::prod_threads[i] = chThdCreateStatic(waProd[i], sizeof(waProd[i]), NORMALPRIO + 1, adcs_threads::producer<3, 10, float>, (void *)&dmag);
i++;
  //producer thread 3
  auto dgyro = Data<3, 10, float>(State::ADCS::gyro_data, StateHistory::ADCS::gyro_history, StateHistory::Rates::ADCS::gyro_history, i);
  adcs_threads::prod_threads[i] = chThdCreateStatic(waProd[i], sizeof(waProd[i]), NORMALPRIO + 1, adcs_threads::producer<3, 10, float>, (void *)&dgyro);

  // adcs_threads::prod_threads[0] = deployProducers<4, 10, float>(cmd_attitude, attitude_cmd_history, attitude_cmd_history_rate, 0);
//   adcs_threads::prod_threads[1] = adcs_threads::deployProducers<3, 10, float>(State::ADCS::mag_data, StateHistory::ADCS::magnetometer_history, StateHistory::Rates::ADCS::magnetometer_history, 1);

//  adcs_threads::prod_threads[2] = adcs_threads::deployProducers<3, 10, float>(State::ADCS::gyro_data, StateHistory::ADCS::gyro_history, StateHistory::Rates::ADCS::gyro_history, 2);

  adcs_threads::consumer_thd = chThdCreateStatic(waConsumerThd, sizeof(waConsumerThd), NORMALPRIO + 1, adcs_threads::consumer, NULL);
  chThdWait(consumer_thd);
  debug_println("Initializing Finished");
}