cmake_minimum_required(VERSION 3.12)

project(
	zeiterfassung
	VERSION 1.0
	LANGUAGES CXX
	DESCRIPTION "Zeiterfassungstool mit StateMachine"
)


add_executable( ${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/src/zeiterfassung.cpp" ) 
