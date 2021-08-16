#ifndef ESTIMATORS_HPP_
#define ESTIMATORS_HPP_

#include "TimedControlTask.hpp"
#undef abs

#include "Estimators/AttitudeEstimator.hpp"
#include "Estimators/OrbitEstimator.hpp"
#include "Estimators/RelativeOrbitEstimator.hpp"
#include "Estimators/TimeEstimator.hpp"

/** @author Kyle Krol
 *
 *  @brief Wraps all estimator control tasks as a single timed control task.
 */
class Estimators : public TimedControlTask<void>
{
  public:
    Estimators() = delete;
    Estimators(Estimators const &) = delete;
    Estimators(Estimators &&) = delete;
    Estimators &operator=(Estimators const &) = delete;
    Estimators &operator=(Estimators &&) = delete;

    ~Estimators() = default;

    Estimators(StateFieldRegistry &registry);

    void init();
    void execute() override;

  private:
    TimeEstimator time_estimator;
    OrbitEstimator orbit_estimator;
    RelativeOrbitEstimator relative_orbit_estimator;
    AttitudeEstimator attitude_estimator;
};

#endif
