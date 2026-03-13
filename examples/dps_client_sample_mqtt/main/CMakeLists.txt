idf_component_register(
    SRCS
        "dps_client_sample_mqtt.c"     # Application logic
        "custom_hsm_optiga.c"          # Custom HSM adapter (OPTIGA ↔ DPS SDK)

    INCLUDE_DIRS
        "."                            # Makes custom_hsm_optiga.h visible
)

component_compile_definitions(SET_TRUSTED_CERT_IN_SAMPLES)
component_compile_definitions(LOAD_TA_FROM_OPTIGA)
