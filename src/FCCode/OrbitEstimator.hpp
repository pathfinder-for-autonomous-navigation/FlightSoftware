#ifndef ORBIT_ESTIMATOR_HPP_
#define ORBIT_ESTIMATOR_HPP_

#include <ControlTask.hpp>

class OrbitEstimator : public ControlTask<void> {
   public:
    /**
     * @brief Construct a new Orbit Estimator.
     * 
     * @param registry 
     */
    OrbitEstimator(StateFieldRegistry& registry);

    /**
     * @brief Using raw sensor inputs, determine the current position of the
     * spacecraft in orbit.
     */
    void execute() override;

   protected:
    //! Control cycle count.
    std::shared_ptr<ReadableStateField<unsigned int>> control_cycle_count_fp;

    //! Position and velocity of this satellite, as read directly from the Piksi in the
    //! ECEF frame.
    std::shared_ptr<ReadableStateField<d_vector_t>> r1_vec_ecef_rd_fp;
    std::shared_ptr<ReadableStateField<d_vector_t>> v1_vec_ecef_rd_fp;

    //! Position and velocity of this satellite, as (either) read directly from the Piksi
    //! or updated by the ground.
    std::shared_ptr<ReadableStateField<d_vector_t>> r2_vec_ecef_rd_fp;
    std::shared_ptr<ReadableStateField<d_vector_t>> v2_vec_ecef_rd_fp;

    // TODO insert other state fields we need to know in order to determine orbital state.

    //! Estimated position and velocity of this satellite in the ECI frame.
    ReadableStateField<d_vector_t> r1_vec_eci_f;
    ReadableStateField<d_vector_t> v1_vec_eci_f;
    //! Estimated position and velocity of other satellite in the ECI frame.
    ReadableStateField<d_vector_t> r2_vec_eci_f;
    ReadableStateField<d_vector_t> v2_vec_eci_f;
};

#endif
