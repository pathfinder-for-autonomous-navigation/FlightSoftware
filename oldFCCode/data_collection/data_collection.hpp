/** @file data_collection.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Specifies functions for initializing data history collection timers.
 */

#ifndef DATA_COLLECTION_HPP_
#define DATA_COLLECTION_HPP_

namespace DataCollection {
/** @brief Initialize ADCS history data collection callbacks. */
void initialize_adcs_history_timers();
/** @brief Initialize propulsion history data collection callbacks. */
void initialize_propulsion_history_timers();
/** @brief Initialize Piksi history data collection callbacks. */
void initialize_piksi_history_timers();
}  // namespace DataCollection

#endif