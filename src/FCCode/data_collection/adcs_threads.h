// adcs_threads.h
// 
// Need to set [PRODUCER_COUNT]!!!
// 

namespace adcs_threads
{
#include "circular_buffer.hpp"

// SET THIS --------------------------
#define PRODUCER_COUNT 3 
// ------------------------------------

void consumer(void *vargp);
void init();
template <typename T, size_t sample_size>
void producer(void *vargp);
// template <typename T, size_t sample_size>
// thread_t *deployProducers(T data,
//                           circular_buffer<T, sample_size> history,
//                           size_t rate,
//                           size_t i);
extern thread_t *consumer_thd;
extern thread_t *prod_threads[PRODUCER_COUNT];
extern THD_WORKING_AREA(waConsumerThd, 16384);
extern THD_WORKING_AREA(waProd[PRODUCER_COUNT], 64);

} 
