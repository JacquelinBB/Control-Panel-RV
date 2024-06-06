cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(PROJECT_PARTITION_TABLE_CSV "partitions.csv")
set(EXTRA COMPONENT_DIRS "components")
project(Control-Panel_RV)