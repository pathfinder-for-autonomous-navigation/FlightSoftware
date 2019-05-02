/**
 * adcs_threads.cpp 
 * 
 * This system uses the producer-consumer model with multiple producers to store
 * state data. A [PRODUCER_COUNT] number of "producer" threads are spawned. Each
 * producer thread is passed a Data struct, which contains references to a 
 * data and history buffer. The producers copy data from their data buffers to their
 * history buffers. When the history buffer is full, the producer will increment
 * [producer_cnt] to indicate that they are finished and then wait. 
 * The last producer signals the consumer thread with [producers_done]
 * that all the producers are finished. 
 * Upon receiving  [producers_done], the consumer thread will serialize the data
 * from each producer thread's history buffer. When it is finish, it will reset
 * the history buffers and broadcast [consumer_done] to all the producers. The
 * producers stop waiting and will start collecting data again. 
 * 
 * 
 * !!!!!!!!!!!!!!!!! WARNING !!!!!!!!!!!!!!!!!!!!!
 * [PRODUCER_COUNT] must be set in [adcs_threads.h]
 * downlink_serialize in the consumer thread needs to be uncommented
 * uncomment the sleep statement in producer thread
 * The code between the "auto generated section" comments are auto-generated. 
 *    Should check that these are valid
 * */
#include "../state/state_holder.hpp"
#include "../state/state_history_holder.hpp"
#include "../comms/downlink_serializer.hpp"
#include "../state/state_history_rates.hpp"
#include "adcs_threads.h"

// Concurrency Variables
static mutex_t cnt_lock;                    // controls access to [producer_cnt]
static condition_variable_t producers_done; // signals to consumer that producers are done
static condition_variable_t consumer_done;  // broadcast to producers that consumer is done

// Producer/Consumer Variables
size_t producer_cnt = 0;
bool prod_ready[PRODUCER_COUNT];

// Threads
thread_t *adcs_threads::consumer_thd;
thread_t *adcs_threads::prod_threads[PRODUCER_COUNT];

THD_WORKING_AREA(adcs_threads::waConsumerThd, 16384);
THD_WORKING_AREA(adcs_threads::waProd[PRODUCER_COUNT], 64);

// Data struct
template <typename T, size_t sample_size>
struct Data
{
  using state_history_t = circular_buffer<T, sample_size> &;

  Data(const T &data_p, state_history_t &state_history_p,
       unsigned int state_history_rate, int id)
      : data_p(data_p),
        state_history_p(state_history_p),
        state_history_rate(state_history_rate), id(id) {}

  const T& data_p;                          // data buffer
  state_history_t state_history_p;          // history buffer
  unsigned int state_history_rate;          // the amount of milliseconds the thread should wait
                                            //  before copying the next packet of data into history
  size_t cur_index;
  int id;
}; // 20 bytes

// Packets that contain the serialized data
static std::bitset<Comms::PACKET_SIZE_BITS> packets[Comms::NUM_PACKETS]; 

/*
 * Producer function that each producer thread runs. 
 *  - Copies the data buffer into the history buffer a [sample_size]
 *    number of times and then atomically increments [producer_cnt]
 *
 *  - T               the type of the data buffer
 *  - sample_size     the number of slots in the history buffer
 *  - vargp           pointer to a Data struct
 */
template <typename T, size_t sample_size>
void adcs_threads::producer(void *vargp)
{
  Data<T, sample_size> *data = (Data <T, sample_size> *)vargp;
  
  while (true)
  {
    while (data->cur_index < sample_size)
    {
      data->state_history_p.put(data->data_p);
      data->cur_index += 1;
      // chThdSleepMilliseconds(MS2ST(data->state_history_rate));
    }
    chMtxLock(&cnt_lock);
    producer_cnt++;

    if (producer_cnt >= PRODUCER_COUNT)
      chCondSignal(&producers_done);
    
    prod_ready[data->id] = false;
    while (prod_ready[data->id] == false)
      chCondWait(&consumer_done);

    chMtxUnlock(&cnt_lock);

    data->state_history_p.reset();
    data->cur_index = 0;

  }
}

/*
 * Consumer function. 
 *  - Waits for [producers_done] signal
 *  - Upon receiving signal, calls [serialize_downlink] to serialize the producer history buffers
 *  - Resets producer_cnt and prod_ready then broadcasts [consumer_done]
 */
void adcs_threads::consumer(void *vargp)
{
  unsigned int downlink_no = 0;
  while (true)
  {
    chMtxLock(&cnt_lock);

    while (producer_cnt != PRODUCER_COUNT)
      chCondWait(&producers_done);
    
    // Reset producer variables
    producer_cnt = 0; 
    for (size_t i = 0; i < PRODUCER_COUNT; i++)
      prod_ready[i] = true;

    // Comms::serialize_downlink(packets, downlink_no);
#ifdef DEBUG
    Serial.printf("Done serializing downlink_no packet: (%d)\n", downlink_no);
    Serial.printf("\n--------------\nConsumer Thd: %d\n Magno: %d\n Gyro: %d \n Attitude: %d\n---------------\n",
                  chUnusedThreadStack(waConsumerThd, sizeof(waConsumerThd)),
                  chUnusedThreadStack(waProd[0], sizeof(waProd[0])),
                  chUnusedThreadStack(waProd[1], sizeof(waProd[1])),
                  chUnusedThreadStack(waProd[2], sizeof(waProd[2])));
#endif

    ++downlink_no;
    chCondBroadcast(&consumer_done);
    chMtxUnlock(&cnt_lock);
  }
}

// template <typename T, size_t sample_size>
// thread_t *adcs_threads::deployProducers(T data,
//                                         circular_buffer<T, sample_size> history,
//                                         size_t rate,
//                                         size_t i, Data<T, sample_size> &d)
// {
//   return chThdCreateStatic(adcs_threads::waProd[i], sizeof(adcs_threads::waProd[i]),
//                            NORMALPRIO + 1, adcs_threads::producer<T, sample_size>, (void *)&d);
// }

/**
 * Initializes the producer threads and the consumer threads
 *  !! This function does not return !!
 * */
void adcs_threads::init()
{

  debug_println("Initializing...");
  chCondObjectInit(&producers_done);
  chCondObjectInit(&consumer_done);
  chMtxObjectInit(&cnt_lock);

  producer_cnt = 0;

/* Auto generated section begin ----------------------------------------------- */
 // -------------------------------------------------------------------------------------
 // -------------------------------------------------------------------------------------
 // Does loop index refer to the inner loop index? 
 // -------------------------------------------------------------------------------------
  // ------------------------------------------------------------------------------------
// #define GEN_PROD_THREADS
// {% for namespace in namespaces -%}
//     {% for field in namespace.fields -%}

//       auto datt_{{loop.index}} = Data<{{field.type}}, {{field.samples}}>(State::{{namespace.name}}::{{field.source}}, 
//         StateHistory::{{namespace.name}}::{{field.name}}, 
//         StateHistory::Rates::{{namespace.name}}::{{field.name}}, 
//         {{loop.index}});

//       adcs_threads::prod_threads[{{loop.index}}] = 
//         chThdCreateStatic(waProd[{{loop.index}}], sizeof(waProd[{{loop.index}}]), NORMALPRIO + 1, 
//         adcs_threads::producer<{{field.type}}, {{field.samples}}>, 
//         (void *)&datt_{{loop.index}});

//     {% endfor %}
// {% endfor -%}

/* Auto generate section end -------------------------------------------------- */


// Test data ------------------------------------------------------------------
// ----------------------------------------------------------------------------
// Auto-generated producer threads should look like these threads: 

#ifndef GEN_PROD_THREADS  // if there is no generated producer threads, run these
  debug_println("Warning: No producer threads have been generated");
  size_t i = 0;

  // producer thread 1
  auto datt = Data<std::array<float, 4>,10>(State::ADCS::cmd_attitude, 
    StateHistory::ADCS::attitude_cmd_history, 
    StateHistory::Rates::ADCS::attitude_cmd_history, 
    i);
  adcs_threads::prod_threads[i] = 
    chThdCreateStatic(waProd[i], sizeof(waProd[i]), NORMALPRIO + 1, 
    adcs_threads::producer<std::array<float, 4>, 10>, 
    (void *)&datt);
  i++;

  // producer thread 2
  auto dmag = Data<std::array<float, 3>, 10>(State::ADCS::mag_data, StateHistory::ADCS::magnetometer_history, StateHistory::Rates::ADCS::magnetometer_history, i);
  adcs_threads::prod_threads[i] = chThdCreateStatic(waProd[i], sizeof(waProd[i]), NORMALPRIO + 1, adcs_threads::producer<std::array<float, 3>,10>, (void *)&dmag);
  i++;

  //producer thread 3
  auto dgyro = Data<std::array<float, 3>, 10>(State::ADCS::gyro_data, StateHistory::ADCS::gyro_history, StateHistory::Rates::ADCS::gyro_history, i);
  adcs_threads::prod_threads[i] = chThdCreateStatic(waProd[i], sizeof(waProd[i]), NORMALPRIO + 1, adcs_threads::producer<std::array<float, 3>, 10>, (void *)&dgyro);
#endif

// End test data --------------------------------------------------------------
// ----------------------------------------------------------------------------

  // consumer thread
  adcs_threads::consumer_thd = chThdCreateStatic(waConsumerThd, sizeof(waConsumerThd), NORMALPRIO + 1, adcs_threads::consumer, NULL);
  chThdWait(consumer_thd);
  debug_println("Initializing Finished");

  // adcs_threads::prod_threads[0] = deployProducers<4, 10, float>(cmd_attitude, attitude_cmd_history, attitude_cmd_history_rate, 0);
  // adcs_threads::prod_threads[1] = adcs_threads::deployProducers<3, 10, float>(State::ADCS::mag_data, StateHistory::ADCS::magnetometer_history, StateHistory::Rates::ADCS::magnetometer_history, 1);
  // adcs_threads::prod_threads[2] = adcs_threads::deployProducers<3, 10, float>(State::ADCS::gyro_data, StateHistory::ADCS::gyro_history, StateHistory::Rates::ADCS::gyro_history, 2);
}