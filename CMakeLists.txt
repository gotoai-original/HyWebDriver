if (NOT HY_WEBDRIVER_DIR)
    set(HY_WEBDRIVER_DIR "${CMAKE_SOURCE_DIR}/Source/HyWebDriver")
    # message("set HY_WEBDRIVER_DIR!")
endif ()

set(DERIVED_SOURCES_HYWEBDRIVER_DIR "${CMAKE_BINARY_DIR}/DerivedSources/HyWebDriver")
file(MAKE_DIRECTORY ${DERIVED_SOURCES_HYWEBDRIVER_DIR})

set(HyWebDriver_INCLUDE_DIRECTORIES
    "${HY_WEBDRIVER_DIR}"
    "${HY_WEBDRIVER_DIR}/miso"
    "${DERIVED_SOURCES_HYWEBDRIVER_DIR}"
    "${DERIVED_SOURCES_WEBKIT2GTK_DIR}"
    "${DERIVED_SOURCES_JAVASCRIPCOREGTK_DIR}"
    "${FORWARDING_HEADERS_DIR}"
    "${FORWARDING_HEADERS_WEBKIT2GTK_DIR}"
    "${FORWARDING_HEADERS_DIR}/JavaScriptCore/glib"
    "${CMAKE_SOURCE_DIR}/Source"
)

set(HyWebDriver_SOURCES
    main.cpp
    HyWebDriver.cpp
    miso/miso_spinlock.c
    miso/miso_timestamp.c
    miso/miso_common.c
    miso/miso_log.c
    miso/miso_thread.c
)

set(HyWebDriver_LIBRARIES
    WTF
    ${JavaScriptCore_LIBRARY_NAME}
    WebKit
    ${GTK3_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${LIBSOUP_LIBRARIES}
    ${GSTREAMER_LIBRARIES}
)

set(HyWebDriver_SYSTEM_INCLUDE_DIRECTORIES
    ${GTK3_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${LIBSOUP_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
)

include_directories(${HyWebDriver_INCLUDE_DIRECTORIES})
include_directories(SYSTEM ${HyWebDriver_SYSTEM_INCLUDE_DIRECTORIES})
add_executable(HyWebDriver ${HyWebDriver_SOURCES})
target_link_libraries(HyWebDriver ${HyWebDriver_LIBRARIES})

WEBKIT_ADD_TARGET_CXX_FLAGS(HyWebDriver -Wno-unused-parameter)
install(TARGETS HyWebDriver DESTINATION "${LIBEXEC_INSTALL_DIR}")

