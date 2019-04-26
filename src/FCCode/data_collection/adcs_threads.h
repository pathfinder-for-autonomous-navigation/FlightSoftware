namespace adcs_threads
{
#include "circular_buffer.hpp"
#define PRODUCER_COUNT 3 // must set this
void consumer(void *vargp);
void init();
template <size_t data_size, size_t history_size, typename T>
void producer(void *vargp);
template <size_t data_size, size_t history_size, typename T>
thread_t *deployProducers(std::array<T, data_size> data,
                          circular_buffer<std::array<T, data_size>, history_size> history,
                          size_t rate,
                          size_t i);
extern thread_t *consumer_thd;
extern thread_t *prod_threads[PRODUCER_COUNT];
extern THD_WORKING_AREA(waConsumerThd, 16384);
extern THD_WORKING_AREA(waProd[PRODUCER_COUNT], 128);

} // namespace adcs_threads
