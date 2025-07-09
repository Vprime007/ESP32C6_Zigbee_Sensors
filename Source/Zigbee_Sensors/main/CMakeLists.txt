idf_component_register(
    
    SRCS                "main.c"
                        "userInterface/userInterface.c"
                        "userInterface/buttonController.c"
                        "userInterface/led/ledController.c"
                        "userInterface/led/ledDriver.c"
                        "userInterface/led/ledDriver_cfg.c"
                        "userInterface/sequencer/sequencer.c"
                        "userInterface/sequencer/sequencer_cfg.c"

                        "network/zigbeeManager.c"
                        "network/basicCluster.c"
                        "network/tempMeasCluster.c"
                        "network/humidityMeasCluster.c"
                        "network/identifyCluster.c"

                        "sensors/aht10.c"
                        "sensors/sensorController.c"

    INCLUDE_DIRS        "."
                        "userInterface"
                        "userInterface/led"
                        "userInterface/sequencer"
                        "network"
                        "sensors"

    PRIV_REQUIRES       spi_flash    
                        nvs_flash
                        driver               
)
