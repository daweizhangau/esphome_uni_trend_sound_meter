import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, ble_client

DEPENDENCIES = ['esp32', 'ble_client']

AUTO_LOAD = ['sensor']
MULTI_CONF = True

CONF_CONTROLLER_ID = 'uni_trend_sound_meter_id'

sensor_namespace = cg.esphome_ns.namespace('uni_trend_sound_meter')

BLESensor = sensor_namespace.class_(
    'UnitTrendSoundMeter', cg.Component, cg.PollingComponent, ble_client.BLEClientNode)

CONFIG_SCHEMA = cv.All(
    sensor.sensor_schema(
        BLESensor,
        accuracy_decimals=1,
        device_class="signal_strength",
        unit_of_measurement="dBA",
        icon="mdi:microphone"
    )
    .extend(cv.polling_component_schema("1s"))
    .extend(ble_client.BLE_CLIENT_SCHEMA)
)

async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)
