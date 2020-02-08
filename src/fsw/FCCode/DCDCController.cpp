#include "DCDCController.hpp"

DCDCController::DCDCController(StateFieldRegistry &registry, unsigned int offset,
    Devices::DCDC &_dcdc)
    : TimedControlTask<void>(registry, "dcdc_ct", offset), dcdc(_dcdc),
      ADCSMotorDCDC_f("dcdc.ADCSMotor", Serializer<bool>()),
      SpikeDockDCDC_f("dcdc.SpikeDock", Serializer<bool>()),
      disable_cmd_f("dcdc.disable_cmd", Serializer<bool>()),
      reset_cmd_f("dcdc.reset_cmd", Serializer<bool>())
{
  add_writable_field(ADCSMotorDCDC_f);
  add_writable_field(SpikeDockDCDC_f);
  add_writable_field(disable_cmd_f);
  add_writable_field(reset_cmd_f);

  // Set default values
  ADCSMotorDCDC_f.set(dcdc.adcs_enabled());
  SpikeDockDCDC_f.set(dcdc.sph_enabled());
  disable_cmd_f.set(false);
  reset_cmd_f.set(false);
}

void DCDCController::execute() {

    if (ADCSMotorDCDC_f.get() && !dcdc.adcs_enabled()) {
        dcdc.enable_adcs();
    }
    else if (!ADCSMotorDCDC_f.get() && dcdc.adcs_enabled()) {
        dcdc.disable_adcs();
    }

    if (SpikeDockDCDC_f.get() && !dcdc.sph_enabled()) {
        dcdc.enable_sph();
    }
    else if (!SpikeDockDCDC_f.get() && dcdc.sph_enabled()) {
        dcdc.disable_sph();
    }

    if (disable_cmd_f.get() && (dcdc.adcs_enabled() || dcdc.sph_enabled())) {
        dcdc.disable();
        ADCSMotorDCDC_f.set(false);
        SpikeDockDCDC_f.set(false);
        disable_cmd_f.set(false);
    }

    if (reset_cmd_f.get()) {
        if (dcdc.adcs_enabled() || dcdc.sph_enabled()) {
            dcdc.disable();
            ADCSMotorDCDC_f.set(false);
            SpikeDockDCDC_f.set(false);
        }
        else if (!dcdc.adcs_enabled() && !dcdc.sph_enabled()) {
            dcdc.enable_adcs();
            ADCSMotorDCDC_f.set(true);
            dcdc.enable_sph();
            SpikeDockDCDC_f.set(true);
            reset_cmd_f.set(false);
        }
    }

}
