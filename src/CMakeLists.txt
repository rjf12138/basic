project(basic)

target_include_directories(basic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_sources(basic PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src/./wejson.cc
		${CMAKE_CURRENT_SOURCE_DIR}/src/./debug.cc
		${CMAKE_CURRENT_SOURCE_DIR}/src/./logger.cc
		${CMAKE_CURRENT_SOURCE_DIR}/src/./byte_buffer.cc)
