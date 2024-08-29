
macro(install_library config targetDir file)
    get_filename_component(name "${file}" NAME)
    if(EXISTS "${file}" AND NOT EXISTS "${targetDir}/${name}")
        message(STATUS "Installing dependency '${name}' (${config})")
        configure_file("${file}" "${targetDir}/${name}" COPYONLY)
    endif()
endmacro()

macro(mingw_install_libraries targetDir)
    if(MINGW)
        get_filename_component(gccdir "${CMAKE_CXX_COMPILER}" DIRECTORY)
        foreach(lib libgcc_s_dw2-1 libgcc_s_seh-1 libwinpthread-1 libstdc++-6 libssp-0)
            install_library("${CMAKE_BUILD_TYPE}" "${targetDir}" "${gccdir}/${lib}.dll")
        endforeach()
    endif()
endmacro()

macro(qt_install_win32_plugins config targetDir libFile)
    set(suffix)
    string(TOUPPER "${config}" cfg)
    if("${cfg}" STREQUAL "DEBUG")
        set(suffix "d")
    endif()
    get_filename_component(path "${libFile}" DIRECTORY)
    get_filename_component(path "${path}" DIRECTORY)
    foreach(plugin ${ARGN})
        set(name "${plugin}${suffix}.dll")
        set(file "${path}/plugins/${name}")
        if(EXISTS "${file}" AND NOT EXISTS "${targetDir}/${name}")
            message(STATUS "Installing dependency '${name}' (${config})")
            configure_file("${file}" "${targetDir}/${name}" COPYONLY)
        endif()
    endforeach()
endmacro()

macro(qt_install_library config targetDir lib)
    string(TOUPPER "${config}" cfg)
    get_target_property(file "${lib}" LOCATION_${cfg})
    get_filename_component(name "${file}" NAME)
    if(EXISTS "${file}" AND NOT EXISTS "${targetDir}/${name}")
        message(STATUS "Installing dependency '${name}' (${config})")
        configure_file("${file}" "${targetDir}/${name}" COPYONLY)
    endif()
    if(WIN32 AND "${lib}" STREQUAL "Qt6::Gui")
        qt_install_win32_plugins("${config}" "${targetDir}" "${file}"
            imageformats/qico
            imageformats/qjpeg
            platforms/qwindows
            )
    endif()
endmacro()

macro(qt_install_libraries target outputDir)
    foreach(name ${ARGN})
        set(lib "Qt6::${name}")
        if(MSVC OR MINGW)
            if(MSVC)
                foreach(config ${CMAKE_CONFIGURATION_TYPES})
                    qt_install_library("${config}" "${outputDir}/${config}" "${lib}")
                endforeach()
            elseif(MINGW)
                qt_install_library("${CMAKE_BUILD_TYPE}" "${outputDir}" "${lib}")
            endif()
        endif()
        target_link_libraries("${target}" PRIVATE "${lib}")
    endforeach()
endmacro()
