cmake_minimum_required(VERSION 3.10)

project(GLAD VERSION 1.0)

add_library(glad
	${CMAKE_CURRENT_SOURCE_DIR}/src/glad.c
)

target_include_directories(glad
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
)
