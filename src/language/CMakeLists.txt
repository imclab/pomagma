
include_directories(${CMAKE_SOURCE_DIR}/)

add_library(pomagma_language SHARED
	language.cpp
	language.pb.cc
	)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_LIST_DIR}/language.pb.cc
	COMMAND protoc ${CMAKE_CURRENT_LIST_DIR}/language.proto
		--proto_path=${CMAKE_CURRENT_LIST_DIR}
		--cpp_out=${CMAKE_CURRENT_LIST_DIR}
	DEPENDS ${CMAKE_CURRENT_LIST_DIR}/language.proto
	)
