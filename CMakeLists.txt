cmake_minimum_required(VERSION 3.10)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR "In-source builds are not allowed, please conduct builds from within a separate build directory")
endif()

project(Fractal VERSION 0.1.0)

add_executable(Fractal
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Game.cpp
)

target_include_directories(Fractal
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/include
)

#set(FRACTAL_REQUIRED_VLK_CORE_VERSION 0.0.0)
#set(FRACTAL_REQUIRED_VLK_COMMON_VERSION 0.0.0)
set(FRACTAL_REQUIRED_VLFW_VERSION 0.2.0)

#if(NOT TARGET ValkyrieEngineCore)
#	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/ValkyrieEngineCore)
#elseif(NOT ValkyrieEngineCore_VERSION VERSION_EQUAL FRACTAL_REQUIRED_VLK_CORE_VERSION)
#	message(FATAL_ERROR "Version mismatch: ValkyrieEngineCore. Found: ${ValkyrieEngineCore_VERSION}, must be at least: ${FRACTAL_REQUIRED_VLK_CORE_VERSION}")
#endif()

#if(NOT TARGET ValkyrieEngineCommon)
#	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/ValkyrieEngineCommon)
#elseif(NOT ValkyrieEngineCommon_VERSION VERSION_EQUAL FRACTAL_REQUIRED_VLK_COMMON_VERSION)
#	message(FATAL_ERROR "Version mismatch: ValkyrieEngineCommon. Found: ${ValkyrieEngineCommon_VERSION}, must be at least: ${FRACTAL_REQUIRED_VLK_COMMON_VERSION}")
#endif()

if (NOT TARGET VLFW)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/VLFW)
elseif(NOT VLFW_VERSION VERSION_EQUAL FRACTAL_REQUIRED_VLFW_VERSION)
	message(FATAL_ERROR "Version mismatch: VLFW. Found: ${VLFW_VERSION}, must be: ${FRACTAL_REQUIRED_VLFW_VERSION}")
endif()

if (NOT TARGET Vulkan::Vulkan)
	find_package(Vulkan)
endif()

if (NOT TARGET glad)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/glad)
endif()

target_link_libraries(Fractal
	PRIVATE
		VLFW
		Vulkan::Vulkan
		glad
)

add_custom_command(
    TARGET Fractal POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf
        ${CMAKE_BINARY_DIR}/res/
)

add_custom_command(
    TARGET Fractal POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/res/
    ${CMAKE_BINARY_DIR}/res/
)
