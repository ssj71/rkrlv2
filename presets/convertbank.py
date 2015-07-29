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
struct_size = struct.calcsize(fmt)

f = file
for i in range(len(f), struct_size):
    b = struct.unpack(rkb_fmt,f[i:i+struct_size]);
    print "char Preset_Name[64]; ", b[0]
    print "char Author[64]; ", b[1]
    print "char Classe[36]; ", b[2]
    print "char Type[4]; ", b[3]
    print "char ConvoFiname[128]; ", b[4]
    print "char cInput_Gain[64]; ", b[5]
    print "char cMaster_Volume[64]; ", b[6]
    print "char cBalance[64]; ", b[7]
    print "float Input_Gain; ", b[8]
    print "float Master_Volume; ", b[9]
    print "float Balance; ", b[10]
    print "int Bypass; ", b[11]
    print "char RevFiname[128]; ", b[4]
    print "char EchoFiname[128]; ", b[4]
    print "int lv[70][20]; ", b[4]
    print "int XUserMIDI[128][20]; ", b[4]
    print "int XMIDIrangeMin[128]; ", b[4]
    print "int XMIDIrangeMax[128]; ", b[4]
    dat1= list(struct.unpack(fmt_s, f[i]))
    dat2= list(struct.unpack(fmt_spec, bytes[i-1024:i]))
    dat1.append(dat2)
    data.append(dat1)
    """
