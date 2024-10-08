cmake_minimum_required(VERSION 3.10)
project(Studio)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")
set_property(GLOBAL PROPERTY AUTOGEN_TARGETS_FOLDER "CMake")
set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

if(MSVC)
    add_definitions(/GS- /Gy /GR
        -D_CRT_SECURE_NO_DEPRECATE=1
        -D_CRT_SECURE_NO_WARNINGS=1
        -D_CRT_NONSTDC_NO_DEPRECATE=1
        -D_SCL_SECURE_NO_WARNINGS=1
        )
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /GL")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /GL")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /GL")
    string(REGEX REPLACE "/INCREMENTAL(:NO)?" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
    string(REGEX REPLACE "/INCREMENTAL(:NO)?" "" CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
    string(REGEX REPLACE "/INCREMENTAL(:NO)?" "" CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /INCREMENTAL:NO /LTCG /OPT:REF /OPT:ICF=20")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /INCREMENTAL:NO /LTCG /OPT:REF /OPT:ICF=20")
elseif(WASM_TARGET)
    add_definitions(-fno-ident -fno-plt -fvisibility=hidden)
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-ident -fno-plt -fvisibility=hidden")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-ident -fno-plt -fvisibility=hidden")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(COMPONENTS_QT Core Gui Svg Widgets Network WebSockets Protobuf)
include(cmake/cute.cmake)
include(cmake/libraries.cmake)
include(cmake/util.cmake)

file(GLOB_RECURSE proto
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    CONFIGURE_DEPENDS
    "src/Network/Protocol/*.proto"
    )

file(GLOB_RECURSE src
     RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
     CONFIGURE_DEPENDS
    "res/*"
    "src/*.cpp"
    "src/*.h"
    "src/*.ui"
    )

foreach(file ${src} ${proto})
    get_filename_component(dir "${file}" DIRECTORY)
    string(REGEX REPLACE "^src" "Source Files" group "${dir}")
    string(REGEX REPLACE "^res" "Resource Files" group "${group}")
    string(REPLACE "/" "\\" group "${group}")
    source_group("${group}" FILES "${file}")
endforeach()

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "Studio")

if(WASM_TARGET)
    cute_add_executable(Studio)
    target_compile_definitions(Studio PRIVATE WASM_TARGET)
else()
    add_executable(Studio WIN32 MACOSX_BUNDLE)
    set_target_properties(Studio PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${build_dir}")
    qt_install_libraries(Studio "${build_dir}" ${COMPONENTS_QT})
    mingw_install_libraries("${build_dir}")
endif()

target_sources(Studio PRIVATE ${src} ${proto})
target_include_directories(Studio PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

qt_add_protobuf(Studio
    GENERATE_PACKAGE_SUBFOLDERS
    OUTPUT_HEADERS headers
    PROTO_FILES ${proto}
    )
set(unicxxfile "${CMAKE_CURRENT_BINARY_DIR}/protobuf_generated.cpp")
set(unicxx "")
foreach(hdr ${headers})
    get_filename_component(dir "${hdr}" DIRECTORY)
    get_filename_component(name "${hdr}" NAME_WE)
    set(cxx_file "${dir}/${name}.qpb.cpp")
    set(reg_file "${dir}/${name}_protobuftyperegistrations.cpp")
    set_source_files_properties("${cxx_file}" "${reg_file}" PROPERTIES HEADER_FILE_ONLY TRUE)
    set(unicxx "${unicxx}#include \"${cxx_file}\"\n#include \"${reg_file}\"\n")
endforeach()
maybe_write_file("${unicxxfile}" "${unicxx}")
source_group("Generated Files" FILES "${unicxxfile}")
target_sources(Studio PRIVATE "${unicxxfile}")
