#include "helpers.hpp"

namespace DataCollection {
template <typename T, unsigned int S>
inline void add_to_buffer(circular_buffer<T, S> &buf, const T &val) {
  chSysLockFromISR();
  buf.put(val);
  chSysUnlockFromISR();
}

template <unsigned int size>
void reset_avg(std::array<float, size> *avg, unsigned int *num_measurements) {
  for (unsigned int i = 0; i < size; i++)
    (*avg)[i] = 0.0f;
  *num_measurements = 0;
}
} // namespace DataCollection