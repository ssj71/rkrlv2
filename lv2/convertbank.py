#!/usr/bin/env python3

#spencer jackson
# a rakarrack bank file to carla preset converter


import argparse
import os
import struct
import rkrremap

def makeCarlaPresetHeader(f):
    f.write("<?xml version='1.0' encoding='UTF-8'?>\n")
    f.write("<!DOCTYPE CARLA-PROJECT>\n")
    f.write("<CARLA-PROJECT VERSION='2.0'>\n")
    f.write("<!-- converted by a rkrlv2 script -->\n")
    f.write("\n")

def makePlugHeader(f,d): 
    f.write("\n")
    f.write(" <!-- " + d[0] + " -->\n")
    f.write(" <Plugin>\n")
    f.write("  <Info>\n")
    f.write("   <Type>LV2</Type>\n")
    f.write("   <Name>" + d[0] + "</Name>\n")
    f.write("   <URI>" + d[1] + "</URI>\n")
    f.write("  </Info>\n")
    f.write("\n")
    f.write("  <Data>\n")
    f.write("   <Active>Yes</Active>\n")
    f.write("   <ControlChannel>1</ControlChannel>\n")
    f.write("   <Options>0x0</Options>\n")
    f.write("\n")

def makeBypassPort(f): 
    f.write("   <Parameter>\n")
    f.write("    <Index>0</Index>\n")
    f.write("    <Name>Bypass</Name>\n")
    f.write("    <Symbol>BYPASS</Symbol>\n")
    f.write("    <Value>0</Value>\n")
    f.write("   </Parameter>\n")
    f.write("\n")

def makePort(f,d,v):
    if not d[0]:
        return
    f.write("   <Parameter>\n")
    f.write("    <Index>" + str(d[0]) + "</Index>\n")
    f.write("    <Name>" + d[1] + "</Name>\n")
    f.write("    <Symbol>" + d[2] + "</Symbol>\n")
    v += d[3]
    f.write("    <Value>" + str(v) + "</Value>\n")
    f.write("   </Parameter>\n")
    f.write("\n")

def makePlugFooter(f):
    f.write("  </Data>\n")
    f.write(" </Plugin>\n")

def makeCarlaPresetFooter(f):
    f.write("</CARLA-PROJECT>")

def makeRvbData(f,v):
    p = {
        0 : "Chamber.rvb" ,
        1 : "Concrete_Stairwell.rvb" ,
        2 : "Hall.rvb" ,
        3 : "Med_Hall.rvb" ,
        4 : "Large_Room.rvb" ,
        5 : "Large_Hall.rvb" ,
        6 : "Guitar_Ambience.rvb" ,
        7 : "Studio.rvb" ,
        8 : "Twilight.rvb" ,
        9 : "Santa_Lucia.rvb" ,
    }
    f.write("   <CustomData>\n")
    f.write("    <Type>http://lv2plug.in/ns/ext/atom#Path</Type>\n")
    f.write("    <Key>http://rakarrack.sourceforge.net/effects.html#Reverbtron:rvbfile</Key>\n")
    f.write("    <Value>/usr/share/rkr.lv2/" + p[v] + "</Value>\n")
    f.write("   </CustomData>\n")

def makeDlyData(f,v):
    p = {
        0 : "SwingPong.dly" ,
        1 : "Short_Delays.dly" ,
        2 : "Flange_plus_Echo.dly" ,
        3 : "Comb.dly" ,
        4 : "EchoFlange.dly" ,
        5 : "Filtered_Echo.dly" ,
        6 : "Notch-Wah.dly" ,
        7 : "Multi-Chorus.dly" ,
        8 : "PingPong.dly" ,
        9 : "90-Shifter.dly" ,
        10 : "Basic_LR_Delay.dly" ,
        11 : "PingPong.dly" ,
    }
    f.write("   <CustomData>\n")
    f.write("    <Type>http://lv2plug.in/ns/ext/atom#Path</Type>\n")
    f.write("    <Key>http://rakarrack.sourceforge.net/effects.html#Echotron:dlyfile</Key>\n")
    f.write("    <Value>/usr/share/rkr.lv2/" + p[v] + "</Value>\n")
    f.write("   </CustomData>\n")

def efxorder2structindex(x):
    return{
        0 : 7, # eq1
        1 : 9, # comp
        2 : 6, # dist
        3 : 5, # overdrive
        4 : 1, # echo
        5 : 2, # chorus
        6 : 4, # phaser
        7 : 3, # flanger
        8 : 0, # reverb
        9 : 8, # paramEq
        10 : 11, # Wha
        11 : 12, # alien
        12 : 13, # cab
        13 : 14, # pan
        14 : 15, # harm
        15 : 16, # music del
        16 : 17, # gate
        17 : 18, # derelict
        18 : 19, # aphasor
        19 : 20, # valve
        20 : 21, # dflange
        21 : 22, # ring
        22 : 23, # exciter
        23 : 24, # mbdist
        24 : 25, # arp
        25 : 26, # expander
        26 : 27, # shuffle
        27 : 28, # synth
        28 : 29, # mbvol
        29 : 30, # convol
        30 : 31, # looper
        31 : 32, # mutro
        32 : 33, # echoverse
        33 : 34, # coil
        34 : 35, # shelf
        35 : 36, # vocoder
        36 : 37, # sust
        37 : 38, # seq
        38 : 39, # shifter
        39 : 40, # stomp
        40 : 41, # reverbtron
        41 : 42, # echotron
        42 : 43, # stereoharm
        43 : 44, # compband
        44 : 44, # otrem
        45 : 46, # vibe
        46 : 47, # inf
    }[x]

def readBankFile(filename):

    #file is just a list of these structures
    #struct Preset_Bank_Struct {
    #    char Preset_Name[64];
    #    char Author[64];
    #    char Classe[36];
    #    char Type[4];
    #    char ConvoFiname[128];
    #    char cInput_Gain[64];
    #    char cMaster_Volume[64];
    #    char cBalance[64];
    #    float Input_Gain;
    #    float Master_Volume;
    #    float Balance;
    #    int Bypass;
    #    char RevFiname[128];
    #    char EchoFiname[128];
    #    int lv[70][20];
    #    int XUserMIDI[128][20];
    #    int XMIDIrangeMin[128];
    #    int XMIDIrangeMax[128];
    #} Bank[62];


    rkb_fmt = "64s64s36s4s128s64s64s64s3fi128s128s1400i2560i128i128i"
    struct_size = struct.calcsize(rkb_fmt)

    f = open(filename,"rb")
    for i in range(62):
        buf = f.read(struct_size)
        b = struct.unpack(rkb_fmt,buf);
        if b[0].decode("utf-8")[0] == '\00':
            continue
        print(b[0].decode("utf-8"))
    #    print("char Preset_Name[64]; ", b[0].decode("utf-8"))
    #    print("char Author[64]; ", b[1].decode("utf-8"))
    #    print("char Classe[36]; ", b[2].decode("utf-8"))
    #    print("char Type[4]; ", b[3].decode("utf-8"))
    #    print("char ConvoFiname[128]; ", b[4].decode("utf-8"))
    #    print("char cInput_Gain[64]; ", b[5].decode("utf-8"))
    #    print("char cMaster_Volume[64]; ", b[6].decode("utf-8"))
    #    print("char cBalance[64]; ", b[7].decode("utf-8"))
    #    print("float Input_Gain; ", b[8])
    #    print("float Master_Volume; ", b[9])
    #    print("float Balance; ", b[10])
    #    print("int Bypass; ", b[11])
    #    print("char RevFiname[128]; ", b[12].decode("utf-8"))
    #    print("char EchoFiname[128]; ", b[13].decode("utf-8"))
    #    print("int lv[70][20]; ")
    #    for j in range(70):
    #        print(j, b[14+20*j:34+20*j])

    #    print "int XUserMIDI[128][20]; ", b[4]
    #    print "int XMIDIrangeMin[128]; ", b[4]
    #    print "int XMIDIrangeMax[128]; ", b[4]


        #open file
        ofname = b[0].decode("utf-8")
        ofname = "presets/" + ofname.split('\0',1)[0] + ".carxp"
        of = open(ofname,'w')

        #print header
        makeCarlaPresetHeader(of) 

        order = b[214:224]
        for j in order:
            k = 14+efxorder2structindex(j)*20
            params = b[k:k+21]
            if (not params[19] or j==30): #skip bypassed or looper TODO: add gate
                continue
            if (j==16):
                print(" WARNING: gate not done")
                continue
            if (j == 29):
                print(" WARNING: convo not done")
            if (j == 30):
                print(" WARNING: looper not done")
                continue
            #print(j, rkrremap.remap(j,-1))
            makePlugHeader(of,rkrremap.remap(j,-1))
            makeBypassPort(of)
            k = 0 
            data = rkrremap.remap(j,k)
            while data[0] != -1:
                if data[0] == 0:
                   if data[1] == "SPECIAL":
                      sd = rkrremap.remap_special(j,k,params[k])
                      #print("  ", j, k, params[k], sd)
                      for n in range(sd[0]):
                        makePort(of,rkrremap.remap(j,sd[2*n+1]),sd[2*n+2])
                   #else skipped
                else:
                    makePort(of,data,params[k])
                k += 1
                data = rkrremap.remap(j,k)
            if (j == 40):
                makeRvbData(of,params[8])
            if (j == 41):
                makeDlyData(of,params[8])
            makePlugFooter(of)
        makeCarlaPresetFooter(of)
        of.close() 
    f.close()

# -----------------------------------------------------------------------------------------------
    
readBankFile("../data/Default.rkrb")
readBankFile("../data/Extra.rkrb")
readBankFile("../data/Extra1.rkrb")
exit()
