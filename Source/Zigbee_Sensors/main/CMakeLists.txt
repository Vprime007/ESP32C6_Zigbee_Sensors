idf_component_register(
    
    SRCS                "main.c"
                        "userInterface/userInterface.c"
                        "userInterface/buttonController.c"
                        "userInterface/ledController.c"

                        "network/zigbeeManager.c"
                        "network/basicCluster.c"
                        "network/tempMeasCluster.c"
                        "network/humidityMeasCluster.c"

    INCLUDE_DIRS        ""
                        "userInterface"
                        "network"

    PRIV_REQUIRES       spi_flash    
                        nvs_flash
                        driver               
)
