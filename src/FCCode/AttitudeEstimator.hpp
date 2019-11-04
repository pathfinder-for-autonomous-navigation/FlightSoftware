#ifndef ATTITUDE_ESTIMATOR_HPP_
#define ATTITUDE_ESTIMATOR_HPP_

#include <ControlTask.hpp>

class AttitudeEstimator : public ControlTask<void> {
   public:
    AttitudeEstimator(StateFieldRegistry& registry);
    void execute() override;

   protected:
    /**
     * @brief Inputs collected from ADCSBoxMonitor.
     */
    std::vector<ReadableStateField<f_vector_t>> sun_vector_fp;
    std::vector<ReadableStateField<f_vector_t>> mag_vector_fp;
    std::vector<ReadableStateField<f_vector_t>> gyr_vector_fp;

    /**
     * @brief Estimation outputs.
     */
    ReadableStateField<f_vector_t> h_vector_fp;
};

#endif
