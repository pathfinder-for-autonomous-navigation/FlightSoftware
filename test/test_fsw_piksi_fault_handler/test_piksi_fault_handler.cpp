#include <unity.h>
#include "../StateFieldRegistryMock.hpp"
#include <fsw/FCCode/PiksiFaultHandler.hpp>
#include <fsw/FCCode/piksi_mode_t.enum>

unsigned int& cc_count = TimedControlTaskBase::control_cycle_count;

class TestFixture {
  public:
    StateFieldRegistryMock registry;

    std::shared_ptr<ReadableStateField<unsigned int>> piksi_state_fp;
    std::shared_ptr<WritableStateField<unsigned char>> mission_state_fp;
    std::shared_ptr<InternalStateField<unsigned int>> last_fix_time_ccno_fp;
    std::shared_ptr<InternalStateField<unsigned int>> enter_close_appr_time_fp;

    std::unique_ptr<PiksiFaultHandler> pfh;

    TestFixture() {
        piksi_state_fp = registry.create_internal_field<unsigned char>("piksi.state");
        mission_state_fp = registry.create_internal_field<unsigned int>("pan.state");
        last_fix_time_ccno_fp = registry.create_writable_field<bool>("piksi.last_fix_time_ccno");
        enter_close_appr_time_fp = registry.create_writable_field<bool>("piksi.last_fix_time_ccno");

        // Set initial conditions
        last_checkin_cycle_fp->set(0);
        radio_power_cycle_fp->set(false);
        cc_count = 0;

        pfh = std::make_unique<PiksiFaultHandler>(registry);
    }

}
