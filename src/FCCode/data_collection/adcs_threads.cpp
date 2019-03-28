#include <AttitudeEstimator.hpp>
#include "../state/state_holder.hpp"
#include "../state/state_history_holder.hpp"
#include <circular_buffer.hpp>
#include "helpers.hpp"
#include "data_collection.hpp"
#include <ChRt.h>
#include <rt/chvt.h>
#include <Arduino.h>
#include <string>

#define PRODUCER_COUNT 3 // must set this

std::bitset<10> *Packet; // placeholder for packet
constexpr unsigned int COLLECTION_PERIOD = 30;

constexpr unsigned int attitude_cmd_history_rate = COLLECTION_PERIOD / 10;
std::array<float, 4> cmd_attitude;
circular_buffer<std::array<float, 4>, 10> attitude_cmd_history;

constexpr unsigned int magnetometer_history_rate = COLLECTION_PERIOD / 10;
std::array<float, 3> mag_data;
circular_buffer<std::array<float, 3>, 10> magnetometer_history;

constexpr unsigned int gyro_history_rate = 500;
std::array<double, 3> gyro_data;
circular_buffer<std::array<double, 3>, 10> gyro_history;

static mutex_t cnt_lock;
static condition_variable_t producers_done;
static condition_variable_t consumer_done;
// the amount of producers that are done
size_t producer_cnt = 0;
bool prod_ready[PRODUCER_COUNT];
struct prod_arg_t
{
  size_t data_size;
  size_t history_size;
  unsigned int rate;
  void *data;
};
static THD_WORKING_AREA(waConsumerThd, 10000);
static THD_WORKING_AREA(waMagnetometer, 10000);
static THD_WORKING_AREA(waAttitude, 10000);
static THD_WORKING_AREA(waGyroscope, 10000);
template <size_t data_size, size_t history_size, typename T>
class Data
{
public:
  using data_t = std::array<T, data_size>;
  using state_history_t = circular_buffer<std::array<T, data_size>, history_size>;

public:
  Data(const data_t &data_p, state_history_t &state_history_p,
       unsigned int state_history_rate, int id)
      : data_p(data_p),
        state_history_p(state_history_p),
        state_history_rate(state_history_rate), id(id) {}

  const data_t &data_p;
  state_history_t &state_history_p;
  unsigned int state_history_rate;
  size_t cur_index;
  int id;
};

/*
 * Each Producer thread is passed a struct with references to their data_buf and
 * state_history. They retrieve data some number of times before acquiring the
 * mutex for producer_cnt and then incrementing it. The last producer signals
 * the consumer.
 */
template <size_t data_size, size_t history_size, typename T>
void producer(void *vargp)
{
  debug_println("Producer start!");
  Data<data_size, history_size, T> *data = (Data<data_size, history_size, T> *)vargp;
  char buf[10];
  while (true)
  {
    debug_println("Producer begin loop");
    itoa(data->id, buf, 10);
    debug_println(buf);
    while (data->cur_index < history_size)
    {
      // Copy the contents of data->data_buf into state_history
      data->state_history_p.put(data->data_p);
      data->cur_index += 1;

      //debug_println(buf);
      chThdSleepMilliseconds(MS2ST(data->state_history_rate));
    }
    debug_println("Producer finish loop");
    // lock and then increment producer count
    // for some reason producer is unable to obtained the lock here.
    chMtxLock(&cnt_lock);
    debug_println("Producer acquired lock");
    producer_cnt++;
    // Signal consumers to start making packets if this is the last producer
    if (producer_cnt == PRODUCER_COUNT)
    {
      chCondSignal(&producers_done);
      debug_println("producer signale");
    }
    // Producers wait on consumer_done, take cnt_lock and then immediately
    // unlock and give to the next producer
    prod_ready[data->id] = false;
    debug_println("Producer stop");
    debug_println(buf);
    while (prod_ready[data->id] == false)
      chCondWait(&consumer_done);
    chMtxUnlock(&cnt_lock);
    debug_println("Producer release lock");
    data->cur_index = 0;
    debug_println("Producer reset");
  }
}

/*
 * Consumer waits for the last producer to signal that it has finished writing
 * to its own state history.
 */
void consumer(void *vargp)
{
  debug_println("[Consumer] Start");
  while (true)
  {
    chMtxLock(&cnt_lock);
    debug_println("[Consumer] start wait");
    // wait for producers to be ready
    while (producer_cnt != PRODUCER_COUNT)
    {
      chCondWait(&producers_done);
    }
    debug_println("[Consumer] acquired lock again");
    for (size_t i = 0; i < PRODUCER_COUNT; i++)
      prod_ready[i] = true;

    //serialize_downlink();
    producer_cnt = 0; // reset producer count
    chCondBroadcast(&consumer_done);
    debug_println("[Consumer] broadcast");
    chMtxUnlock(&cnt_lock);
  }
}

// template <size_t data_size, size_t history_size, typename T>
// void deployProducers(std::array<T, data_size> data,
//                      circular_buffer<std::array<float, data_size>, history_size> history,
//                      size_t rate,
//                      size_t i)
// {
//   auto d = Data<data_size, history_size, T>(data, history, rate, i);
//   // THD_WORKING_AREA(waProducerThds[i], 256);
//   THD_WORKING_AREA(waProducerThds[i], 256);
//   chThdCreateStatic(waProducerThds[i], sizeof(waProducerThds[i]),
//                     NORMALPRIO + 1, producer<data_size, history_size, T>, (void *)&d);
// }

void init()
{
  debug_println("Initializing...");
  chCondObjectInit(&producers_done);
  chCondObjectInit(&consumer_done);
  chMtxObjectInit(&cnt_lock);

  producer_cnt = 0;
  size_t i = 0;
  auto datt = Data<4, 10, float>(cmd_attitude, attitude_cmd_history, attitude_cmd_history_rate, 0);
  thread_t *p = chThdCreateStatic(waAttitude, sizeof(waAttitude),
                                  NORMALPRIO + 1, producer<4, 10, float>, (void *)&datt);
  auto dmag = Data<3, 10, float>(mag_data, magnetometer_history, magnetometer_history_rate, 1);
  chThdCreateStatic(waMagnetometer, sizeof(waMagnetometer),
                    NORMALPRIO + 1, producer<3, 10, float>, (void *)&dmag);
  auto dgyro = Data<3, 10, double>(gyro_data, gyro_history, gyro_history_rate, 2);
  chThdCreateStatic(waGyroscope, sizeof(waGyroscope),
                    NORMALPRIO + 1, producer<3, 10, double>, (void *)&dgyro);
  chThdCreateStatic(waConsumerThd, sizeof(waConsumerThd), NORMALPRIO + 1, consumer, NULL);
  // deployProducers<3, 10, float>(mag_data, magnetometer_history, magnetometer_history_rate, i++);
  //deployProducers<3, 10, double>(gyro_data, gyro_history, gyro_history_rate, i++);
  //deployProducers<4, 10, double>(cmd_attitude, attitude_cmd_history, attitude_cmd_history_rate, i++);
  chThdWait(p);
  debug_println("Initializing Finished");
}

// // int main()
// {
//   init();
//   // create consumer thread
//   chThdCreateStatic(waConsumerThd, sizeof(waConsumerThd), NORMALPRIO + 1, consumer, NULL);

//   create producer threads auto d1 = Data<3, 10, float>(mag_data, magnetometer_history, magnetometer_history_rate, i);
//   pthread_create(&producer_tid[i++], NULL, producer<3, 10, float>, (void *)&d1);
//   auto d2 = Data<3, 10, double>(gyro_data, gyro_history, gyro_history_rate, i);
//   pthread_create(&producer_tid[i++], NULL, producer<3, 10, double>, (void *)&d2);
//   int i = 0;
//   auto d2 = Data<3, 10, double>(gyro_data, gyro_history, gyro_history_rate, i);
//   chdThdCreateStatic(waProducersThds[i], sizeof(waProducersThds[i++]),
//                      NORMALPRIO + 1, producer<4, 10, float>, (void *)&d2);
//   auto d3 = Data<4, 10, float>(cmd_attitude, attitude_cmd_history, attitude_cmd_history_rate, i);
//   chdThdCreateStatic(waProducersThds[i], sizeof(waProducersThds[i++]),
//                      NORMALPRIO + 1, producer<4, 10, float>, (void *)&d3);
//   return 0;
// }
