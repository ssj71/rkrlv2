#CMake file for LV2 Versions of Rakarrack Effects

cmake_minimum_required(VERSION 2.6)

project (rkrlv2)

set(LV2_INSTALL_DIR lib/lv2/rkr.lv2 ) # CACHE PATH "Specifies where the LV2 libraries should be installed")

IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86|X86|amd64|AMD64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3 -msse -msse2 -mfpmath=sse -ffast-math")
ENDIF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86|X86|amd64|AMD64")

# check for our various libraries
find_package(PkgConfig)
pkg_check_modules(LV2 REQUIRED lv2>=1.10.0)
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
  ../src/DynamicFilter.C
  ../src/Filter.C
  ../src/FilterParams.C
  ../src/SVFilter.C
  ../src/FormantFilter.C
  ../src/NewDist.C
  ../src/Valve.C
  ../src/Dual_Flange.C
  ../src/Ring.C
  ../src/MBDist.C
  ../src/Arpie.C
  ../src/Expander.C
  ../src/Shuffle.C
  ../src/Synthfilter.C
  ../src/MBVvol.C
  ../src/RBFilter.C
  ../src/RyanWah.C
  ../src/RBEcho.C
  ../src/CoilCrafter.C
  ../src/ShelfBoost.C
  ../src/Vocoder.C
  ../src/Sequence.C
  ../src/beattracker.C
  ../src/Shifter.C
  ../src/StompBox.C
  ../src/Reverbtron.C
  ../src/Echotron.C
  ../src/StereoHarm.C
  ../src/CompBand.C
  ../src/Opticaltrem.C
  ../src/Vibe.C
  ../src/Infinity.C
  ../src/Phaser.C
  ../src/Gate.C
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
install (DIRECTORY ${CMAKE_BINARY_DIR}/lv2data/
  DESTINATION ${LV2_INSTALL_DIR}
)

#also install carla presets into usr share
install (DIRECTORY ${CMAKE_BINARY_DIR}/lv2data/
  DESTINATION share/rkr.lv2
)
install (DIRECTORY presets/
  DESTINATION share/rkr.lv2
)
