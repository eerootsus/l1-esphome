#pragma once

#include <vector>
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/light/light_state.h"
#include "esphome/components/light/light_traits.h"

namespace esphome {
namespace sonoff_l1 {
class SonoffL1Output : public light::LightOutput, public uart::UARTDevice, public Component {
 public:
  // LightOutput methods
  light::LightTraits get_traits() override;
  void setup_state(light::LightState *state) override;
  void write_state(light::LightState *state) override;

  // Component methods
  void setup() override {};
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return esphome::setup_priority::BUS; }

 protected:
  std::vector<uint8_t> bytes_{};
  light::LightState *light_state_{nullptr};
  light::LightState *next_light_state_{nullptr};
  uint64_t last_sequence_{0};

  std::string uart_bytes_to_string(std::vector<uint8_t> bytes);
  void send_at_command(const char *str);
  void send_next_state();
};

}  // namespace sonoff_l1
}  // namespace esphome
