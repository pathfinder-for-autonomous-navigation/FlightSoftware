#include "Estimators.hpp"

Estimators::Estimators(StateFieldRegistry &registry, unsigned int offset)
    : TimedControlTask<void>(registry, "estimators", offset),
      time_estimator(registry),
      orbit_estimator(registry),
      relative_orbit_estimator(registry),
      attitude_estimator(registry)
{ }

void Estimators::execute()
{
    time_estimator.execute();
    orbit_estimator.execute();
    relative_orbit_estimator.execute();
    attitude_estimator.execute();
}
