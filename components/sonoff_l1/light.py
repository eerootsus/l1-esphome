import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, light, output
from esphome.const import CONF_BLUE, CONF_GREEN, CONF_RED, CONF_OUTPUT_ID

DEPENDENCIES = ["uart", "light"]

sonoff_l1_ns = cg.esphome_ns.namespace("sonoff_l1")
SonoffL1Output = sonoff_l1_ns.class_(
    "SonoffL1", cg.Component, uart.UARTDevice, light.LightOutput
)

CONFIG_SCHEMA = (
    light.RGB_LIGHT_SCHEMA.extend(
        {
            cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(SonoffL1Output),
            cv.Required(CONF_RED): cv.use_id(output.FloatOutput),
            cv.Required(CONF_GREEN): cv.use_id(output.FloatOutput),
            cv.Required(CONF_BLUE): cv.use_id(output.FloatOutput),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)
FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "sonoff_l1", baud_rate=19200, require_tx=True, require_rx=True
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await cg.register_component(var, config)
    await light.register_light(var, config)
    await uart.register_uart_device(var, config)

    red = await cg.get_variable(config[CONF_RED])
    cg.add(var.set_red(red))
    green = await cg.get_variable(config[CONF_GREEN])
    cg.add(var.set_green(green))
    blue = await cg.get_variable(config[CONF_BLUE])
    cg.add(var.set_blue(blue))
