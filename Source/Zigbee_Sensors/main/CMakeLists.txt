idf_component_register(
    
    SRCS                "main.c"
                        "userInterface/userInterface.c"
                        "userInterface/buttonController.c"
                        "userInterface/ledController.c"

    INCLUDE_DIRS        ""
                        "userInterface"
                       
    PRIV_REQUIRES spi_flash                   
)
