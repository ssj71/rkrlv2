#!/usr/bin/env python3

#spencer jackson
# a rakarrack bank file to carla preset converter


import argparse
import os
import struct


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


# -----------------------------------------------------------------------------------------------

rkb_fmt = "64s64s36s4s128s64s64s64s3fi128s128s1400i2560i128i128i"
struct_size = struct.calcsize(rkb_fmt)

f = open("../data/Default.rkrb","rb")
#f = open("../data/Extra.rkrb","rb")
for i in range(62):
    buf = f.read(struct_size)
    b = struct.unpack(rkb_fmt,buf);
    if b[0].decode("utf-8")[0] == '\00':
        continue
    print("char Preset_Name[64]; ", b[0].decode("utf-8"))
    print("char Author[64]; ", b[1].decode("utf-8"))
    print("char Classe[36]; ", b[2].decode("utf-8"))
    print("char Type[4]; ", b[3].decode("utf-8"))
    print("char ConvoFiname[128]; ", b[4].decode("utf-8"))
    print("char cInput_Gain[64]; ", b[5].decode("utf-8"))
    print("char cMaster_Volume[64]; ", b[6].decode("utf-8"))
    print("char cBalance[64]; ", b[7].decode("utf-8"))
    print("float Input_Gain; ", b[8])
    print("float Master_Volume; ", b[9])
    print("float Balance; ", b[10])
    print("int Bypass; ", b[11])
    print("char RevFiname[128]; ", b[12].decode("utf-8"))
    print("char EchoFiname[128]; ", b[13].decode("utf-8"))
    print("int lv[70][20]; ")
    for j in range(70):
        print(j, b[14+20*j:33+20*j])

#    print "int XUserMIDI[128][20]; ", b[4]
#    print "int XMIDIrangeMin[128]; ", b[4]
#    print "int XMIDIrangeMax[128]; ", b[4]
f.close()
