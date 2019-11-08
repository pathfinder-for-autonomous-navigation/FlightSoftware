#ifndef GNC_NAVIGATOR_HPP_
#define GNC_NAVIGATOR_HPP_

#include <ControlTask.hpp>

class GNCNavigator : public ControlTask<void> {
   public:
    /**
     * @brief Construct a new Orbit Estimator.
     * 
     * @param registry 
     */
    GNCNavigator(StateFieldRegistry& registry);

    /**
     * @brief Determine the magnitude, time, and direction of the next
     * propulsive manuever.
     */
    void execute() override;

   protected:
    //! Estimated position and velocity of this satellite in the ECI frame.
    std::shared_ptr<ReadableStateField<d_vector_t>> r1_vec_eci_fp;
    std::shared_ptr<ReadableStateField<d_vector_t>> v1_vec_eci_fp;
    //! Estimated position and velocity of other satellite in the ECI frame.
    std::shared_ptr<ReadableStateField<d_vector_t>> r2_vec_eci_fp;
    std::shared_ptr<ReadableStateField<d_vector_t>> v2_vec_eci_fp;

    //! Navigator mode. If in "auto" mode (true), it publishes recommended firings to
    //! the propulsion_cmd fields below as soon as it determines them. If in
    //! "manual" mode, the ground has all authority on propulsive manuevers.
    WritableStateField<bool> navigator_mode_f;

    //! Either a propulsion manuever computed by the rendezvous algorithm, or a
    //! propulsion manuever manually commanded by the ground.
    WritableStateField<d_vector_t> propulsion_cmd_f;
    WritableStateField<gps_time_t> propulsion_cmd_time_f;
};

#endif
