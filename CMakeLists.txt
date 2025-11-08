cmake_minimum_required(VERSION "3.19.2")

add_subdirectory("contrib/AGE")

project("chip8")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

if(APPLE)
	set(CMAKE_MACOSX_BUNDLE TRUE)
    set(CMAKE_INSTALL_RPATH "@executable_path/../Frameworks")
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	add_executable(chip8 MACOSX_BUNDLE ${MY_SOURCES})
else()
	add_executable(chip8 ${MY_SOURCES})
endif()
#set_target_properties(chip8 PROPERTIES INSTALL_PATH "@executable_path/../Frameworks")

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
elseif(APPLE)
	file(GLOB EXTRA_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/AGE/lib/mac/*.dylib")
	foreach(DLL ${EXTRA_DLLS})
		add_custom_command(TARGET chip8 POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E make_directory
				$<TARGET_FILE_DIR:chip8>/../Frameworks
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				"${DLL}"
				$<TARGET_FILE_DIR:chip8>/../Frameworks
		)
	endforeach()
endif()

install(TARGETS chip8
	RUNTIME DESTINATION .
	BUNDLE DESTINATION .
)

install(FILES misc/put_roms_here
	DESTINATION media/roms)
	

