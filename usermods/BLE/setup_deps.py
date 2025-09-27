from platformio.package.meta import PackageSpec
Import('env')

env.Append(CPPDEFINES=[("USERMOD_BLE")
                     , ("CONFIG_NIMBLE_CPP_LOG_LEVEL", 5)
                     , ("CONFIG_BT_NIMBLE_LOG_LEVEL", 5)
                     , ("CONFIG_BT_NIMBLE_ROLE_CENTRAL_DISABLED ")
                     , ("CONFIG_BT_NIMBLE_ROLE_OBSERVER_DISABLED")
                     , ("CONFIG_BT_NIMBLE_ROLE_BROADCASTER_DISABLED")])
