file(GLOB sdk_avian_src *.cpp *.hpp *.c *.h internal/*.hpp internal/*.cpp internal/*.h)

add_library(sdk_avian_obj OBJECT ${sdk_avian_src})
target_compile_definitions(sdk_avian_obj PRIVATE radar_sdk_EXPORTS=1)
target_link_libraries(sdk_avian_obj PUBLIC sdk_base_obj sdk_radar_device_common_obj)
target_link_libraries(sdk_avian_obj PRIVATE strata_static lib_avian_static)
#target_include_directories(sdk_avian_obj PUBLIC ifxAvian) # XXX

add_library(sdk_avian SHARED ../dummy.c)
target_link_libraries(sdk_avian PUBLIC sdk_avian_obj sdk_base_obj sdk_radar_device_common_obj)

add_library(sdk_avian_static STATIC ../dummy.c)
target_link_libraries(sdk_avian_static PUBLIC sdk_avian_obj sdk_base_obj sdk_radar_device_common_obj)
