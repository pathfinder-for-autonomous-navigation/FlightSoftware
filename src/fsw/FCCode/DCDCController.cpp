#include "DCDCController.hpp"

DCDCController::DCDCController(StateFieldRegistry &registry, unsigned int offset,
    Devices::DCDC &_dcdc)
    : TimedControlTask<void>(registry, "dcdc_ct", offset), dcdc(_dcdc),
      adcs_dcdc_cmd_f("dcdc.adcs_cmd", Serializer<bool>()),
      sph_dcdc_cmd_f("dcdc.sph_cmd", Serializer<bool>()),
      disable_cmd_f("dcdc.disable_cmd", Serializer<bool>()),
      reset_cmd_f("dcdc.reset_cmd", Serializer<bool>())
{
  add_writable_field(adcs_dcdc_cmd_f);
  add_writable_field(sph_dcdc_cmd_f);
  add_writable_field(disable_cmd_f);
  add_writable_field(reset_cmd_f);

  // Set default values
  adcs_dcdc_cmd_f.set(dcdc.adcs_enabled());
  sph_dcdc_cmd_f.set(dcdc.sph_enabled());
  disable_cmd_f.set(false);
  reset_cmd_f.set(false);
}

void DCDCController::execute() {

    if (adcs_dcdc_cmd_f.get() && !dcdc.adcs_enabled()) {
        dcdc.enable_adcs();
    }
    else if (!adcs_dcdc_cmd_f.get() && dcdc.adcs_enabled()) {
        dcdc.disable_adcs();
    }

    if (sph_dcdc_cmd_f.get() && !dcdc.sph_enabled()) {
        dcdc.enable_sph();
    }
    else if (!sph_dcdc_cmd_f.get() && dcdc.sph_enabled()) {
        dcdc.disable_sph();
    }

    if (disable_cmd_f.get() && (dcdc.adcs_enabled() || dcdc.sph_enabled())) {
        dcdc.disable();
        disable_cmd_f.set(false);
    }

    if (reset_cmd_f.get()) {
        dcdc.reset();
        reset_cmd_f.set(false);
    }

}
