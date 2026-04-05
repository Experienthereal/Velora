cmake_minimum_required(VERSION 3.16)
project(velora VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─── Velora Compiler (vlc) ───
add_executable(velora
    compiler/main.cpp
    compiler/lexer.cpp
    compiler/parser.cpp
    compiler/analyzer.cpp
    compiler/codegen.cpp
)

target_include_directories(velora PRIVATE compiler)

if(MSVC)
    target_compile_options(velora PRIVATE /O2 /W3)
else()
    target_compile_options(velora PRIVATE -O2 -Wall -Wextra)
endif()

# ─── Veloragame Runtime Library ───
add_library(veloragame STATIC
    veloragame/veloragame.c
)

target_include_directories(veloragame PUBLIC
    veloragame/
    stdlib/
)

if(WIN32)
    target_link_libraries(veloragame PRIVATE
        gdi32
        user32
        kernel32
        winmm
    )
    target_compile_definitions(veloragame PRIVATE WIN32_LEAN_AND_MEAN)
endif()

# ─── Install ───
install(TARGETS velora RUNTIME DESTINATION bin)
install(TARGETS veloragame ARCHIVE DESTINATION lib)
install(FILES
    veloragame/veloragame.h
    veloragame/vg_all.h
    veloragame/vg_physics.h
    veloragame/vg_ui.h
    veloragame/vg_sound.h
    veloragame/vg_scene.h
    DESTINATION include/veloragame
)
install(FILES
    stdlib/vl_math.h
    stdlib/vl_io.h
    stdlib/vl_fs.h
    stdlib/vl_time.h
    stdlib/vl_log.h
    DESTINATION include/velora/stdlib
)

# ─── Print build summary ───
message(STATUS "")
message(STATUS "  Velora ${PROJECT_VERSION}")
message(STATUS "  Platform : ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Build    : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler : ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "")
