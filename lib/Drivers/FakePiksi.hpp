#ifndef FAKE_PIKSI_HPP_
#define FAKE_PIKSI_HPP_

#include <Piksi.hpp>

namespace Devices {
class FakePiksi : public Piksi {
public:
  /** Constructor **/
  using Piksi::Piksi;

  bool setup() override;
  bool is_functional() override;
  void reset() override;
  void disable() override; // Sets Piksi's power consumption to a minimum

  bool process_buffer() override;

  void get_gps_time(gps_time_t *time) override;

  void get_pos_ecef(std::array<double, 3> *position) override;
  unsigned char get_pos_ecef_nsats() override;
  unsigned char get_pos_ecef_flags() override;

  void get_baseline_ecef(std::array<double, 3> *position) override;
  unsigned char get_baseline_ecef_nsats() override;

  void get_vel_ecef(std::array<double, 3> *velocity) override;
  unsigned char get_vel_ecef_nsats() override;

  void get_base_pos_ecef(std::array<double, 3> *position) override;
  unsigned int get_iar() override;
};
}

#endif