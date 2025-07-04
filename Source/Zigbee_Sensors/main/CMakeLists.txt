idf_component_register(
    
    SRCS                "main.c"
                        "userInterface/userInterface.c"
                        "userInterface/buttonController.c"
                        "userInterface/ledController.c"

                        "network/zigbeeManager.c"
                        "network/basicCluster.c"
                        "network/tempMeasCluster.c"
                        "network/humidityMeasCluster.c"
                        "network/identifyCluster.c"

                        "sensors/aht10.c"
                        "sensors/sensorController.c"

    INCLUDE_DIRS        ""
                        "userInterface"
                        "network"
                        "sensors"

    PRIV_REQUIRES       spi_flash    
                        nvs_flash
                        driver               
)
