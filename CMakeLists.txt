# CMake definitions for SoftFM

cmake_minimum_required(VERSION 2.4)
project(SoftFM)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")

find_package(Threads)
find_package(PkgConfig)
find_package(RtAudio REQUIRED)

IF(APPLE)
  FIND_LIBRARY(CoreFoundation_LIBRARY CoreFoundation)
  FIND_LIBRARY(CoreAudio_LIBRARY CoreAudio)
  MARK_AS_ADVANCED (CoreFoundation_LIBRARY CoreAudio_LIBRARY)
  SET(EXTRA_LIBS ${CoreFoundation_LIBRARY} ${CoreAudio_LIBRARY})
ELSE(APPLE)
  find_package(ALSA REQUIRED)
ENDIF (APPLE)

# Find RTL-SDR library (for linking).
pkg_check_modules(PKG_RTLSDR librtlsdr)
find_path(RTLSDR_INCLUDE_DIR rtl-sdr.h
          HINT ${PKG_RTLSDR_INCLUDE_DIRS})
find_library(RTLSDR_LIBRARY librtlsdr.a
             HINT ${PKG_RTLSDR_LIBRARY_DIRS})

find_library(SPUCE_LIBRARY libspuce.a HINT "/usr/local/lib")
					 
FIND_PACKAGE(PythonLibs REQUIRED)
					 
# Find libusb
pkg_check_modules(PKG_LIBUSB libusb-1.0)
find_path(LIBUSB_INCLUDE_DIR libusb.h
          HINT ${PKG_LIBUSB_INCLUDE_DIRS}
          PATH_SUFFIXES libusb-1.0)
find_library(LIBUSB_LIBRARY usb-1.0
             HINT ${PKG_LIBUSB_LIBRARY_DIRS})

if(RTLSDR_INCLUDE_DIR AND RTLSDR_LIBRARY)
    message(STATUS "Found librtlsdr: ${RTLSDR_INCLUDE_DIR}, ${RTLSDR_LIBRARY}")
else()
    message(WARNING "Can not find Osmocom RTL-SDR library")
    message("Try again with environment variable PKG_CONFIG_PATH")
    message("or with -DRTLSDR_INCLUDE_DIR=/path/rtlsdr/include")
    message("        -DRTLSDR_LIBRARY=/path/rtlsdr/lib/librtlsdr.a")
endif()

set(RTLSDR_INCLUDE_DIRS ${RTLSDR_INCLUDE_DIR} ${LIBUSB_INCLUDE_DIR} ${RtAudio_INCLUDE_DIR})
set(RTLSDR_LIBRARIES    ${RTLSDR_LIBRARY} ${LIBUSB_LIBRARY} ${RtAudio_LIBRARY} ${EXTRA_LIBS})

# Compiler flags.
set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -O2 -ffast-math -ftree-vectorize ${EXTRA_FLAGS}")

add_executable(nbfm
    main.cpp
    RtlSdrSource.cpp
    fm_demod.cpp
    fm_demod_filt.cpp
    fm_discriminator.cpp
    AudioOutput.cpp
    )

	include_directories(
		${PYTHON_INCLUDE_DIRS}
		${CMAKE_CURRENT_SOURCE_DIR}
    ${RTLSDR_INCLUDE_DIRS}
    ${ALSA_INCLUDE_DIRS}
    ${EXTRA_INCLUDES} )

target_link_libraries(nbfm
    ${CMAKE_THREAD_LIBS_INIT}
    ${RTLSDR_LIBRARIES}
    ${ALSA_LIBRARIES}
		${SPUCE_LIBRARY}
    ${EXTRA_LIBS} )

install(TARGETS nbfm DESTINATION bin)

