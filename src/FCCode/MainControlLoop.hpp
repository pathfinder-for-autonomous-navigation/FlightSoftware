#ifndef MAIN_CONTROL_LOOP_TASK_HPP_
#define MAIN_CONTROL_LOOP_TASK_HPP_

#include "ClockManager.hpp"
#include "DebugTask.hpp"
#include "DockingController.hpp"
#include "FieldCreatorTask.hpp"
#include "MissionManager.hpp"
#include <ControlTask.hpp>
#include <StateField.hpp>
#include <StateFieldRegistry.hpp>

#ifdef HOOTL
// OK
#elif FLIGHT
// OK
#else
static_assert(false, "Need to define either the HOOTL or FLIGHT flags.");
#endif

class MainControlLoop : public ControlTask<void> {
protected:
  FieldCreatorTask field_creator_task;
  ClockManager clock_manager;
  DebugTask debug_task;
  MissionManager mission_manager;

  Devices::DockingSystem docksys;
  DockingController docking_controller;

// Control cycle time offsets, in microseconds
#ifdef HOOTL
  static constexpr unsigned int debug_task_offset = 1000;
  static constexpr unsigned int mission_manager_offset = 51000;
  static constexpr unsigned int docking_controller_offset = 52000;
#else
  static constexpr unsigned int debug_task_offset = 1000;
  static constexpr unsigned int mission_manager_offset = 1010;
  static constexpr unsigned int docking_controller_offset = 2010;
#endif

public:
  /**
   * @brief Construct a new Main Control Loop Task object
   *
   * @param registry State field registry
   */
  MainControlLoop(StateFieldRegistry &registry);

  /**
   * @brief Processes state field commands present in the serial buffer.
   */
  void execute() override;
};

#endif
