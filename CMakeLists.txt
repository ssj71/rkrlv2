#CMake file for LV2 Versions of Rakarrack Effects

cmake_minimum_required(VERSION 2.6)

#rename these files so they are easier to use for plugins
SET( RVB x;Chamber;Concrete_Stairwell;Hall;Med_Hall;Large_Room;Large_Hall;Guitar_Ambience;Studio;Twilight;Santa_Lucia )
SET( DLY x;SwingPong;Short_Delays;Flange_plus_Echo;Comb;EchoFlange;Filtered_Echo;Notch-Wah;Multi-Chorus;PingPong;90-Shifter;Basic_LR_Delay )

foreach(INDX RANGE 1 10)
    list(GET RVB ${INDX} NEWNAME)
    configure_file(data/${INDX}.rvb lv2data/${NEWNAME}.rvb COPYONLY)
    list(GET DLY ${INDX} NEWNAME)
    configure_file(data/${INDX}.dly lv2data/${NEWNAME}.dly COPYONLY)
endforeach(INDX)
# 1 more dly file 
list(GET DLY 11 NEWNAME)
configure_file(data/11.dly lv2data/${NEWNAME}.dly COPYONLY)


add_subdirectory(lv2)
