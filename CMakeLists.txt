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
    function(copy_per_config_dlls TARGET DLL_MAPPINGS)
      foreach(MAPPING ${DLL_MAPPINGS})
        string(REPLACE "|" ";" PARTS "${MAPPING}")
        list(GET PARTS 0 RELEASE_NAME)
        list(GET PARTS 1 DEBUG_NAME)

        set(DLL_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/out/build/x64-Release/contrib/AGE/bin/${RELEASE_NAME}")
        set(DLL_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/out/build/x64-Debug/contrib/AGE/bin/${DEBUG_NAME}")

        add_custom_command(TARGET ${TARGET} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<$<CONFIG:Debug>:${DLL_DEBUG}>"
            "$<$<CONFIG:Release>:${DLL_RELEASE}>"
            "$<TARGET_FILE_DIR:${TARGET}>"
        )
      endforeach()
    endfunction()

    set(DLL_MAPPINGS
      "SDL2.dll|SDL2d.dll"
      "SDL2_ttf.dll|SDL2_ttfd.dll"
      "SDL2_image.dll|SDL2_imaged.dll"
      "SDL2_mixer.dll|SDL2_mixerd.dll"
    )

    copy_per_config_dlls(chip8 "${DLL_MAPPINGS}")
endif()

install(TARGETS chip8
	RUNTIME DESTINATION .
)

install(FILES misc/put_roms_here
	DESTINATION media/roms)

