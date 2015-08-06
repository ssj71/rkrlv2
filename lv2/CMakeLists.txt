#CMake file for LV2 Versions of Rakarrack Effects

cmake_minimum_required(VERSION 2.6)

project (rkrlv2)

set(LV2_INSTALL_DIR lib/lv2 ) # CACHE PATH "Specifies where the LV2 libraries should be installed")
set(CMAKE_CXX_FLAGS "-Wall -g -O2 -Wl,-no-undefined")

# check for our various libraries
find_package(PkgConfig)
pkg_check_modules(LV2 REQUIRED lv2>=1.10.0)
pkg_check_modules(SAMPLERATE REQUIRED samplerate>=0.1.5)
pkg_check_modules(FFTW  REQUIRED fftw3)

include_directories (${LV2_INCLUDE_DIRS} ${SAMPLERATE_INCLUDE_DIRS} ${FFTW_INCLUDE_DIRS} ../src)
link_directories (${LV2_LIBRARY_DIRS} ${SAMPLERATE_LIBRARY_DIRS} ${FFTW_LIBRARY_DIRS} )

# make sure corresponding ttl and source files match these prefixes
SET( PLUGS eql;comp;dist;echo;chorus;aphas;reve;eqp;wha;alienwah;cab;pan;harm_no_midi;mdel;dere;valve;dual_flange;ring;exciter;distband;arpie;expander;shuffle;synthfilter;varyband;mutromojo;echoverse;coilcrafter;shelfboost;vocoder;sustain;sequence;shifter;stomp;reverbtron;echotron;stereoharm_no_midi;compband;otrem;vibe;inf )

#run python script that generates manifest in passed in directory (bypass with switch)
add_custom_target(manifests ALL 
    COMMAND python3 manifestcrunch.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/lv2
    ) 

add_custom_target(install-separate
  ${CMAKE_COMMAND} 
  -D "BUILD_TYPE=Debug"
  -D "CMAKE_INSTALL_COMPONENT=separate"
  -P "${PROJECT_BINARY_DIR}/cmake_install.cmake"
)


# config libraries

add_library(eqllv2 SHARED
#  rkrlv2.C
  eqllv2.C
  ../src/EQ.C
  ../src/AnalogFilter.C
)
add_library(complv2 SHARED
  complv2.C
  ../src/Compressor.C
  ../src/FPreset.C
)
#add_library(distlv2 SHARED
#  distlv2.C
#  ../src/Distorsion.C
#  ../src/Waveshaper.C
#  ../src/Resample.C
#  ../src/FPreset.C
#  ../src/AnalogFilter.C
#)

#  ../src/Echo.C
#  ../src/delayline.C
#  ../src/Chorus.C
#  ../src/EffectLFO.C
#  ../src/APhaser.C
#  ../src/Harmonizer.C
#  ../src/RecognizeNote.C
#  ../src/RecChord.C
#  ../src/Sustainer.C
#  ../src/smbPitchShift.C
#  ../src/Exciter.C
#  ../src/HarmonicEnhancer.C
#  ../src/Pan.C
#  ../src/Alienwah.C
#  ../src/Reverb.C
#  ../src/Cabinet.C
#  ../src/MusicDelay.C
#  ../src/DynamicFilter.C
#  ../src/Filter.C
#  ../src/FilterParams.C
#  ../src/SVFilter.C
#  ../src/FormantFilter.C
#  ../src/NewDist.C
#  ../src/Valve.C
#  ../src/Dual_Flange.C
#  ../src/Ring.C
#  ../src/MBDist.C
#  ../src/Arpie.C
#  ../src/Expander.C
#  ../src/Shuffle.C
#  ../src/Synthfilter.C
#  ../src/MBVvol.C
#  ../src/RBFilter.C
#  ../src/RyanWah.C
#  ../src/RBEcho.C
#  ../src/CoilCrafter.C
#  ../src/ShelfBoost.C
#  ../src/Vocoder.C
#  ../src/Sequence.C
#  ../src/beattracker.C
#  ../src/Shifter.C
#  ../src/StompBox.C
#  ../src/Reverbtron.C
#  ../src/Echotron.C
#  ../src/StereoHarm.C
#  ../src/CompBand.C
#  ../src/Opticaltrem.C
#  ../src/Vibe.C
#  ../src/Infinity.C
#)

foreach(INDX RANGE 0 1 )
    list(GET PLUGS ${INDX} NAME)

    add_dependencies(install-separate ${NAME}lv2)

    set_target_properties(${NAME}lv2 PROPERTIES PREFIX "")

    set_target_properties(${NAME}lv2 PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)

    target_link_libraries(${NAME}lv2 ${LV2_LIBRARIES} ${SAMPLERATE_LIBRARIES} ${FFTW_LIBRARIES} )

    # config install 
    #install for separate bundles
    install(TARGETS ${NAME}lv2
      LIBRARY
      DESTINATION ${LV2_INSTALL_DIR}/rkr_${NAME}.lv2
      CONFIGURATIONS Debug
      COMPONENT separate
    )

    install (FILES ttl/${NAME}.ttl
      DESTINATION ${LV2_INSTALL_DIR}/rkr_${NAME}.lv2
      CONFIGURATIONS Debug
      COMPONENT separate
    )
    install (FILES ttl/manifests/${NAME}/manifest.ttl
      DESTINATION ${LV2_INSTALL_DIR}/rkr_${NAME}.lv2
      CONFIGURATIONS Debug
      COMPONENT separate
    )
    
    #install for all in 1 bundle
    install(TARGETS ${NAME}lv2
      LIBRARY
      DESTINATION ${LV2_INSTALL_DIR}/rkr.lv2
#      CONFIGURATIONS Debug
    )

    install (FILES ttl/${NAME}.ttl
      DESTINATION ${LV2_INSTALL_DIR}/rkr.lv2
#      CONFIGURATIONS Debug
    )

endforeach(INDX)

    install (FILES ttl/manifest.ttl
      DESTINATION ${LV2_INSTALL_DIR}/rkr.lv2
#      CONFIGURATIONS Debug
    )

#TODO make this only install into reverbtron and echotron bundles
#install (DIRECTORY ${CMAKE_BINARY_DIR}/lv2data/
#  DESTINATION ${LV2_INSTALL_DIR}/rkr.lv2
#)

#TODO: intall only if monolithic bundle selected?
#also install carla presets into usr share

#install (DIRECTORY ${CMAKE_BINARY_DIR}/lv2data/
#  DESTINATION share/rkr.lv2
#)
#install (DIRECTORY presets/
#  DESTINATION share/rkr.lv2
#)
