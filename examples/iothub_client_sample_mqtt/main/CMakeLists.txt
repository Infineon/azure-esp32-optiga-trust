idf_component_register(
	SRCS "iothub_client_sample_mqtt.c"
	"azure_main.c"
	INCLUDE_DIRS "."
)

#component_compile_definitions(SET_TRUSTED_CERT_IN_SAMPLES)
#component_compile_definitions(LOAD_TA_FROM_OPTIGA)