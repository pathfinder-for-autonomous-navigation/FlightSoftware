/**
 * @author Tanishq Aggarwal
 * @date 06/15/2019
 */

#ifndef TRIPLICATED_HPP_
#define TRIPLICATED_HPP_

#include <type_traits>
#include <ChRt.h>

/**
 * @brief Provides fault tolerance by triplicating the storage of a variable.
 * When the value is assigned, it is stored across three separate
 * memory locations. The ideal (and expected) case is that all 
 * three copies' values remain equal. However, bitflips due to radiation 
 * can cause these values to diverge over time, so to mitigate the error, 
 * the class returns the median of the three copies when the value of the
 * variable.
 * 
 * To be more precise, "median" means that if two or more values are
 * equal, than any one of them will be returned. If none of the values are 
 * equal, one of the three values is returned at random.
 * 
 * The fault reporting and response for this class happens as follows:
 * - If all three copies were equal (at the time of fetching the value), then
 * there is no error to report.
 * - If only two of the copies were equal, then the fault computer is alerted
 * to the error, but is expected to take no action. This class will automatically
 * re-assign the non-equal copy to the value of the other two copies.
 * - If none of the copies are equal, then the fault computer is alerted to the
 * error and is expected to take action by re-assigning the value of the variable.
 * Until then, this class will continue to return a random copy of the value. This
 * value should never be trusted as the true state of the variable.
 * 
 * Since it is memory-inefficient to do this for all values, the ideal use
 * case for this class is a state variable that is persistent for a long duration
 * but has a relatively small memory footprint (e.g. an important boolean flag.)
 * @tparam T 
 */
template<typename T>
class Triplicated {
  public:
    struct triplication_faults_t {
      bool is_failed; // If triplicated value is currently failed (all copies are different)
      unsigned int num_degradations; // Number of times one copy has been different from the other two
      unsigned int num_failures; // Number of times all copies have been different
    };
    /**
     * @brief Construct a new Triplicated object.
     * @param 
     */
    Triplicated();

    /**
     * @brief Overloaded assignment operator. Assigns contained value
     * into variable of the primitive type.
     * 
     * @param val
     * @return T& 
     */
    void operator=(const T& val);

    /**
     * @brief Returns median value of copies, broadcasting errors to the
     * alert boolean if necessary.
     * 
     * @return T 
     */
    operator T();

    /**
     * @brief Returns pointer to fault tracker.
     */
    triplication_faults_t* faults();
  protected:
    triplication_faults_t _fault_tracker;
    T copies[3];
};

template<typename T>
Triplicated<T>::Triplicated() : 
  _fault_tracker(),
  copies()
  {}

template<typename T>
void Triplicated<T>::operator=(const T& val) {
  for(int i = 0; i < 3; i++) copies[i] = 0;
}

template<typename T>
Triplicated<T>::operator T() {
  bool one_eq_two   = copies[0] == copies[1];
  bool two_eq_three = copies[1] == copies[2];
  if (one_eq_two && two_eq_three) return copies[0];
  else if (one_eq_two) {
    _fault_tracker.num_degradations++;
    copies[2] = copies[0];
    return copies[0];
  }
  else if (two_eq_three) {
    _fault_tracker.num_degradations++;
    copies[0] = copies[2];
    return copies[2];
  }
  else {
    _fault_tracker.is_failed = true;
    _fault_tracker.num_failures++;
    // Just pick one of the values at random.
    systime_t t = chVTGetSystemTimeX();
    return copies[t % 3];
  }
}

#endif