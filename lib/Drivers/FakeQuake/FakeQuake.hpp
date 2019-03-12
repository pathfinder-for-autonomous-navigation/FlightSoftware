#ifndef FAKE_QUAKE_HPP_
#define FAKE_QUAKE_HPP_

#include <QLocate/QLocate.hpp>

namespace Devices {
  class FakeQuake : public QLocate {
    /*! Sets the QLocate serial port and serial timeout value. Do not initialize
    *  the serial port with begin(), it will be done in the constructor. */
    using QLocate::QLocate;

    bool is_functional() override;
    void reset() override;
    void disable() override;

    int sbdwb(char const *c, int len) override;

    int run_sbdix() override;

    int end_sbdix() override;

    int sbdrb() override;
  };
}

#endif