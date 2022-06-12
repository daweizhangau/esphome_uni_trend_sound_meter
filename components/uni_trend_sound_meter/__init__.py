import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import cover, ble_client
from esphome.const import CONF_ID

DEPENDENCIES = ['esp32', 'ble_client']

AUTO_LOAD = ['cover']
MULTI_CONF = True

CONF_CONTROLLER_ID = 'unit_trend_sound_meter_id'
CONF_ONLY_UP_DOWN_COMMAND = 'only_up_down_command'

sensor_namespace = cg.esphome_ns.namespace('uni_trend_sound_meter')

ControllerComponent = sensor_namespace.class_(
    'UnitTrendSoundMeter', cg.Component, cover.Cover, ble_client.BLEClientNode)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(ControllerComponent),
    cv.Optional(CONF_ONLY_UP_DOWN_COMMAND, False): cv.boolean,
}).extend(ble_client.BLE_CLIENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    # await ble_client.register_ble_node(var, config)
