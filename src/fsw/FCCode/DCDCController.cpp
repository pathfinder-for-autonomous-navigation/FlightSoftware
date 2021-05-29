#include "DCDCController.hpp"

DCDCController::DCDCController(StateFieldRegistry &registry,
    Devices::DCDC &_dcdc)
    : TimedControlTask<void>(registry, "dcdc_ct"), dcdc(_dcdc),
      ADCSMotorDCDC_cmd_f("dcdc.ADCSMotor_cmd", Serializer<bool>()),
      SpikeDockDCDC_cmd_f("dcdc.SpikeDock_cmd", Serializer<bool>()),
      disable_cmd_f("dcdc.disable_cmd", Serializer<bool>()),
      reset_cmd_f("dcdc.reset_cmd", Serializer<bool>()),
      ADCSMotorDCDC_f("dcdc.ADCSMotor", Serializer<bool>()),
      SpikeDockDCDC_f("dcdc.SpikeDock", Serializer<bool>())
{
  add_writable_field(ADCSMotorDCDC_cmd_f);
  add_writable_field(SpikeDockDCDC_cmd_f);
  add_writable_field(disable_cmd_f);
  add_writable_field(reset_cmd_f);
  add_readable_field(ADCSMotorDCDC_f);
  add_readable_field(SpikeDockDCDC_f);

  // Set default values
  ADCSMotorDCDC_cmd_f.set(dcdc.adcs_enabled());
  SpikeDockDCDC_cmd_f.set(dcdc.sph_enabled());
  disable_cmd_f.set(false);
  reset_cmd_f.set(false);
  ADCSMotorDCDC_f.set(dcdc.adcs_enabled());
  SpikeDockDCDC_f.set(dcdc.sph_enabled());
}

void DCDCController::execute() {

    if (ADCSMotorDCDC_cmd_f.get() && !dcdc.adcs_enabled()) {
        dcdc.enable_adcs();
    }
    else if (!ADCSMotorDCDC_cmd_f.get() && dcdc.adcs_enabled()) {
        dcdc.disable_adcs();
    }

    if (SpikeDockDCDC_cmd_f.get() && !dcdc.sph_enabled()) {
        dcdc.enable_sph();
    }
    else if (!SpikeDockDCDC_cmd_f.get() && dcdc.sph_enabled()) {
        dcdc.disable_sph();
    }

    if (disable_cmd_f.get()) {
        if (dcdc.adcs_enabled() || dcdc.sph_enabled()) {
            dcdc.disable();
            // Set commands to false to prevent unwanted writes on the next cycle
            ADCSMotorDCDC_cmd_f.set(false);
            SpikeDockDCDC_cmd_f.set(false);
        }
        // Whether or not action was taken, we don't want the disable command to
        // persist across control cycles
        disable_cmd_f.set(false);
    }

    if (reset_cmd_f.get()) {
        if (dcdc.adcs_enabled() || dcdc.sph_enabled()) {
            dcdc.disable();
            // Set commands to false to prevent unwanted writes on the next cycle
            ADCSMotorDCDC_cmd_f.set(false);
            SpikeDockDCDC_cmd_f.set(false);
        }
        else {
            dcdc.enable_adcs();
            dcdc.enable_sph();
            // Set commands to true to prevent unwanted writes on the next cycle
            ADCSMotorDCDC_cmd_f.set(true);
            SpikeDockDCDC_cmd_f.set(true);
            reset_cmd_f.set(false);
        }
    }

    ADCSMotorDCDC_f.set(dcdc.adcs_enabled());
    SpikeDockDCDC_f.set(dcdc.sph_enabled());

}
