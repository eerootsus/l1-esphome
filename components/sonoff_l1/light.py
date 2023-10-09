import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, light
from esphome.const import CONF_OUTPUT_ID

DEPENDENCIES = ["uart", "light"]

sonoff_l1_ns = cg.esphome_ns.namespace("sonoff_l1")
SonoffL1Output = sonoff_l1_ns.class_(
    "SonoffL1", cg.Component, uart.UARTDevice, light.LightOutput
)

CONFIG_SCHEMA = (
    light.RGB_LIGHT_SCHEMA.extend(
        {
            cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(SonoffL1Output),
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
