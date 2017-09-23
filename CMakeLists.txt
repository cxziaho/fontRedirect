cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(fontRedirect)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -nostdlib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

add_executable(${PROJECT_NAME}.elf
	main.c
)

target_link_libraries(${PROJECT_NAME}.elf
	c
	gcc
	SceLibKernel_stub
	SceIofilemgr_stub  
	SceKernelThreadMgr_stub
	SceSysmem_stub 
	taihen_stub
	ScePvf_stub
)

set_target_properties(${PROJECT_NAME}.elf
	PROPERTIES LINK_FLAGS "-nostdlib"
)

add_custom_target(${PROJECT_NAME}.suprx ALL
	COMMAND vita-elf-create -e ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.yml ${PROJECT_NAME}.elf ${PROJECT_NAME}.velf
	COMMAND vita-make-fself -c ${PROJECT_NAME}.velf ${PROJECT_NAME}.suprx
)

add_dependencies(${PROJECT_NAME}.suprx ${PROJECT_NAME}.elf)
