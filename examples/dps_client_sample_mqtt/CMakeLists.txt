cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)


idf_build_set_property(COMPILE_OPTIONS "-Wno-format-nonliteral;-Wno-format-security;-Wformat=0;-Wno-unused-variable" APPEND)

set (EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/../../components/optiga" "${CMAKE_CURRENT_LIST_DIR}/../../components/esp-azure/port")
project(dps_client_sample_mqtt)
