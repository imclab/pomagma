
include_directories(${CMAKE_SOURCE_DIR}/)

set(POMAGMA_CARTOGRAPHER_LIBS
	pomagma_cartographer
       	hdf5_hl hdf5
	crypto ssl
	protobuf
	)

add_executable(cartographer_carrier_test carrier_test.cpp)
target_link_libraries(cartographer_carrier_test ${POMAGMA_CARTOGRAPHER_LIBS})
add_test(NAME cartographer_carrier COMMAND cartographer_carrier_test)

add_executable(binary_relation_test binary_relation_test.cpp)
target_link_libraries(binary_relation_test ${POMAGMA_CARTOGRAPHER_LIBS})
add_test(NAME cartographer_binary_relation COMMAND binary_relation_test)

add_executable(nullary_function_test nullary_function_test.cpp)
target_link_libraries(nullary_function_test ${POMAGMA_CARTOGRAPHER_LIBS})
add_test(NAME cartographer_nullary_function COMMAND nullary_function_test)

add_executable(injective_function_test injective_function_test.cpp)
target_link_libraries(injective_function_test ${POMAGMA_CARTOGRAPHER_LIBS})
add_test(NAME cartographer_injective_function COMMAND injective_function_test)

add_executable(binary_function_test binary_function_test.cpp)
target_link_libraries(binary_function_test ${POMAGMA_CARTOGRAPHER_LIBS})
add_test(NAME cartographer_binary_function COMMAND binary_function_test)

add_executable(symmetric_function_test symmetric_function_test.cpp)
target_link_libraries(symmetric_function_test ${POMAGMA_CARTOGRAPHER_LIBS})
add_test(NAME cartographer_symmetric_function COMMAND symmetric_function_test)

add_executable(load_item_dim load_item_dim.cpp)
target_link_libraries(load_item_dim ${POMAGMA_CARTOGRAPHER_LIBS})

add_executable(copy copy_main.cpp)
target_link_libraries(copy ${POMAGMA_CARTOGRAPHER_LIBS})
#install(TARGETS copy DESTINATION ${PROJECT_SOURCE_DIR}/bin)

add_executable(aggregate aggregate_main.cpp)
target_link_libraries(aggregate ${POMAGMA_CARTOGRAPHER_LIBS})
#install(TARGETS aggregate DESTINATION ${PROJECT_SOURCE_DIR}/bin)

add_executable(trim trim_main.cpp)
target_link_libraries(trim ${POMAGMA_CARTOGRAPHER_LIBS})
#install(TARGETS trim DESTINATION ${PROJECT_SOURCE_DIR}/bin)
