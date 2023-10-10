#pragma once

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

  #define SONOFF_L1_MODE_COLORFUL           1  // [Color key] Colorful (static color)
  #define SONOFF_L1_MODE_COLORFUL_GRADIENT  2  // [SMOOTH] Colorful Gradient
  #define SONOFF_L1_MODE_COLORFUL_BREATH    3  // [FADE] Colorful Breath
  #define SONOFF_L1_MODE_DIY_GRADIENT       4  // DIY Gradient (fade in and out) [Speed 1- 100, color]
  #define SONOFF_L1_MODE_DIY_PULSE          5  // DIY Pulse  (faster fade in and out) [Speed 1- 100, color]
  #define SONOFF_L1_MODE_DIY_BREATH         6  // DIY Breath (toggle on/off) [Speed 1- 100, color]
  #define SONOFF_L1_MODE_DIY_STROBE         7  // DIY Strobe (faster toggle on/off) [Speed 1- 100, color]
  #define SONOFF_L1_MODE_RGB_GRADIENT       8  // RGB Gradient
  #define SONOFF_L1_MODE_RGB_PULSE          9  // [STROBE] RGB Pulse
  #define SONOFF_L1_MODE_RGB_BREATH        10  // RGB Breath
  #define SONOFF_L1_MODE_RGB_STROBE        11  // [FLASH] RGB strobe
  #define SONOFF_L1_MODE_SYNC_TO_MUSIC     12  // Sync to music [Speed 1- 100, sensitivity 1 - 10]

 public:

  int mode = SONOFF_L1_MODE_COLORFUL;

  // LightOutput methods
  light::LightTraits get_traits() override;
  void setup_state(light::LightState *state) override { this->light_state_ = state; }
  void write_state(light::LightState *state) override;

  // Component methods
  void setup() override{};
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return esphome::setup_priority::DATA; }

 protected:
  bool last_binary_{false};
  uint8_t last_brightness_{0};
  int write_count_{0};
  int read_count_{0};
  light::LightState *light_state_{nullptr};
};

}  // namespace sonoff_l1
}  // namespace esphome
