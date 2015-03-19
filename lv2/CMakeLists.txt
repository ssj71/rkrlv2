#CMake file for LV2 Versions of Rakarrack Effects

cmake_minimum_required(VERSION 2.6)

project (rkrlv2)

set(LV2_INSTALL_DIR /usr/lib/lv2/rkr.lv2 ) # CACHE PATH "Specifies where the LV2 libraries should be installed")
set(CMAKE_CXX_FLAGS "-Wall -g")

# check for our various libraries
find_package(PkgConfig)
pkg_check_modules(LV2 REQUIRED lv2>=.0.1.0)
pkg_check_modules(SAMPLERATE REQUIRED samplerate>=0.1.5)
pkg_check_modules(FFTW  REQUIRED fftw3)

include_directories (${LV2_INCLUDE_DIRS} ${SAMPLERATE_INCLUDE_DIRS} ${FFTW_INCLUDE_DIRS} ../src)
link_directories (${LV2_LIBRARY_DIRS} ${SAMPLERATE_LIBRARY_DIRS} ${FFTW_LIBRARY_DIRS} )


# config libraries

add_library(rkrlv2 SHARED
  rkrlv2.C
  ../src/EQ.C
  ../src/AnalogFilter.C
  ../src/Compressor.C
  ../src/Distorsion.C
  ../src/Waveshaper.C
  ../src/Resample.C
  ../src/FPreset.C
  ../src/Echo.C
  ../src/delayline.C
  ../src/Chorus.C
  ../src/EffectLFO.C
  ../src/APhaser.C
  ../src/Harmonizer.C
  ../src/RecognizeNote.C
  ../src/RecChord.C
  ../src/Sustainer.C
  ../src/smbPitchShift.C
  ../src/Exciter.C
  ../src/HarmonicEnhancer.C
  ../src/Pan.C
  ../src/Alienwah.C
  ../src/Reverb.C
  ../src/Cabinet.C
  ../src/MusicDelay.C
)

set_target_properties(rkrlv2 PROPERTIES PREFIX "")

set_target_properties(rkrlv2 PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)

target_link_libraries(rkrlv2 ${LV2_LIBRARIES} ${SAMPLERATE_LIBRARIES} ${FFTW_LIBRARIES} )

# config install
install(TARGETS rkrlv2
  LIBRARY
  DESTINATION ${LV2_INSTALL_DIR}
)

install (DIRECTORY ttl/
  DESTINATION ${LV2_INSTALL_DIR}
)
