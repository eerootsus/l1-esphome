#include "sonoff_l1.h"

namespace esphome {
namespace sonoff_l1 {

static const char *const TAG = "sonoff_l1";

// This assumes some data is already available
bool SonoffL1Output::read_command_(uint8_t *cmd, size_t &len) {
  // Read a minimal packet
  if (this->read_array(cmd, 6)) {
    ESP_LOGV(TAG, "[%04d] Reading from strip:", this->write_count_);
    ESP_LOGV(TAG, "[%04d] %s", this->write_count_, format_hex_pretty(cmd, 6).c_str());
  }
  return true;
}

bool SonoffL1Output::write_command_(uint8_t *cmd, const size_t len) {
    ESP_LOGV(TAG, "[%04d] Writing to the strip:", this->write_count_);
    ESP_LOGV(TAG, "[%04d] %s", this->write_count_, format_hex_pretty(cmd, len).c_str());
    //this->write_array(cmd, len);
    this->write_count_++;
    return true;
}

bool SonoffL1Output::control_strip_(const bool binary, const uint8_t brightness) {
  ESP_LOGI(TAG, "[%04d] Setting strip state to %s, raw brightness=%d", this->write_count_, ONOFF(binary), brightness);
  return this->write_command_(cmd, sizeof(cmd));
}

void SonoffL1Output::process_command_(const uint8_t *cmd, const size_t len) {
  if (cmd[2] == 0x01 && cmd[3] == 0x04 && cmd[4] == 0x00 && cmd[5] == 0x0A) {
    uint8_t ack_buffer[7] = {0xAA, 0x55, cmd[2], cmd[3], 0x00, 0x00, 0x00};
    ESP_LOGI(TAG, "[%04d] RF sets strip state to %s, raw brightness=%d", this->write_count_, ONOFF(cmd[6]), cmd[7]);
    const uint8_t new_brightness = cmd[7];
    const bool new_state = cmd[6];

    // Got light change state command. In all cases we revert the command immediately
    // since we want to rely on ESP controlled transitions
    if (new_state != this->last_binary_ || new_brightness != this->last_brightness_) {
      this->control_strip_(this->last_binary_, this->last_brightness_);
    }

    this->publish_state_(new_state, new_brightness);
  } else {
    ESP_LOGW(TAG, "[%04d] Unexpected command received", this->write_count_);
  }
}

void SonoffL1Output::publish_state_(const bool is_on, const uint8_t brightness) {
  if (light_state_) {
    ESP_LOGV(TAG, "Publishing new state: %s, brightness=%d", ONOFF(is_on), brightness);
    auto call = light_state_->make_call();
    call.set_state(is_on);
    if (brightness != 0) {
      // Brightness equal to 0 has a special meaning.
      // D1 uses 0 as "previously set brightness".
      // Usually zero brightness comes inside light ON command triggered by RF remote.
      // Since we unconditionally override commands coming from RF remote in process_command_(),
      // here we mimic the original behavior but with LightCall functionality
      call.set_brightness((float) brightness / 100.0f);
    }
    call.perform();
  }
}

// Set the device's traits
light::LightTraits SonoffL1Output::get_traits() {
  auto traits = light::LightTraits();
  traits.set_supported_color_modes({light::ColorMode::BRIGHTNESS});
  return traits;
}

void SonoffL1Output::write_state(light::LightState *state) {
  bool binary;
  float brightness;

  // Fill our variables with the device's current state
  state->current_values_as_binary(&binary);
  state->current_values_as_brightness(&brightness);

  // Convert ESPHome's brightness (0-1) to the device's internal brightness (0-100)
  const uint8_t calculated_brightness = (uint8_t) roundf(brightness * 100);

  if (calculated_brightness == 0) {
    // if(binary) ESP_LOGD(TAG, "current_values_as_binary() returns true for zero brightness");
    binary = false;
  }

  // If a new value, write to the strip
  if (binary != this->last_binary_ || calculated_brightness != this->last_brightness_) {
    if (this->control_strip_(binary, calculated_brightness)) {
      this->last_brightness_ = calculated_brightness;
      this->last_binary_ = binary;
    } else {
      // Return to original value if failed to write to the strip
      ESP_LOGW(TAG, "Failed to update the strip, publishing the previous state");
      this->publish_state_(this->last_binary_, this->last_brightness_);
    }
  }
}

void SonoffL1Output::dump_config() {
  ESP_LOGCONFIG(TAG, "Sonoff L1 LED strip: '%s'", this->light_state_ ? this->light_state_->get_name().c_str() : "");
}

void SonoffL1Output::loop() {
  uint8_t buffer[32] = {0};
  size_t len = sizeof(buffer);
  int count = static_cast<int>(len);
  ESP_LOGV(TAG, "Trying to read %04d bytes from UART", count);
  while (this->available() && count--) {
    this->read_byte(&buffer[len - count - 1]);
  }

  ESP_LOGV(TAG, "Got from strip: %s", format_hex_pretty(buffer, len).c_str());
}

}  // namespace sonoff_l1
}  // namespace esphome