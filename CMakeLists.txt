cmake_minimum_required(VERSION "3.19.2")

add_subdirectory("contrib/AGE")

project("chip8")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

add_executable(chip8 ${MY_SOURCES})

set_target_properties(chip8 PROPERTIES INSTALL_PATH "@executable_path/bin")

target_include_directories(chip8 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(chip8 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/contrib/AGE/include")
target_link_libraries(chip8 PUBLIC AGE)

if(WIN32)
	file(GLOB EXTRA_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/AGE/lib/win/*.dll")
	foreach(DLL ${EXTRA_DLLS})
		add_custom_command(TARGET chip8 POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				"${DLL}"
				$<TARGET_FILE_DIR:chip8>
		)
	endforeach()
endif()

install(TARGETS chip8
	RUNTIME DESTINATION .
)

install(FILES misc/put_roms_here
	DESTINATION media/roms)
	

