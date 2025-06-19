idf_component_register(
    
    SRCS                "main.c"
                        "userInterface/userInterface.c"
                        "userInterface/buttonController.c"
                        "userInterface/ledController.c"

                        "network/networkManager.c"

    INCLUDE_DIRS        ""
                        "userInterface"
                        "network"
                       
    PRIV_REQUIRES spi_flash                   
)
