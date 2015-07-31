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
        print(j, b[14+20*j:34+20*j])

#    print "int XUserMIDI[128][20]; ", b[4]
#    print "int XMIDIrangeMin[128]; ", b[4]
#    print "int XMIDIrangeMax[128]; ", b[4]
f.close()
exit()

#print header
order = b[214:224]
for j in order
    k = 14+efxorder2lvindex(j)*20
    params = b[k:k+21]
    if params[19]:
        continue
    print(found)

def efxorder2lvindex(x)
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

# Bank[i].lv[0][19] = Reverb_Bypass; 
# Bank[i].lv[1][19] = Echo_Bypass;
# Bank[i].lv[2][19] = Chorus_Bypass;
# Bank[i].lv[3][19] = Flanger_Bypass;
# Bank[i].lv[4][19] = Phaser_Bypass;
# Bank[i].lv[5][19] = Overdrive_Bypass;
# Bank[i].lv[6][19] = Distorsion_Bypass;
# Bank[i].lv[7][19] = EQ1_Bypass; 0
# Bank[i].lv[8][19] = EQ2_Bypass;
# Bank[i].lv[9][19] = Compressor_Bypass;

# Bank[i].lv[10][19] = effect Order;

# Bank[i].lv[11][19] = WhaWha_Bypass;
# Bank[i].lv[12][19] = Alienwah_Bypass;
# Bank[i].lv[13][19] = Cabinet_Bypass;
# Bank[i].lv[14][19] = Pan_Bypass;
# Bank[i].lv[15][19] = Harmonizer_Bypass;
# Bank[i].lv[16][19] = MusDelay_Bypass;
# Bank[i].lv[17][19] = Gate_Bypass;
# Bank[i].lv[18][19] = NewDist_Bypass;
# Bank[i].lv[19][19] = APhaser_Bypass;
# Bank[i].lv[20][19] = Valve_Bypass;
# Bank[i].lv[21][19] = DFlange_Bypass;
# Bank[i].lv[22][19] = Ring_Bypass;
# Bank[i].lv[23][19] = Exciter_Bypass;
# Bank[i].lv[24][19] = MBDist_Bypass;
# Bank[i].lv[25][19] = Arpie_Bypass;
# Bank[i].lv[26][19] = Expander_Bypass;
# Bank[i].lv[27][19] = Shuffle_Bypass;
# Bank[i].lv[28][19] = Synthfilter_Bypass;
# Bank[i].lv[29][19] = MBVvol_Bypass;
# Bank[i].lv[30][19] = Convol_Bypass;
# Bank[i].lv[31][19] = Looper_Bypass;
# Bank[i].lv[32][19] = RyanWah_Bypass;
# Bank[i].lv[33][19] = RBEcho_Bypass;
# Bank[i].lv[34][19] = CoilCrafter_Bypass;
# Bank[i].lv[35][19] = ShelfBoost_Bypass;
# Bank[i].lv[36][19] = Vocoder_Bypass;
# Bank[i].lv[37][19] = Sustainer_Bypass;
# Bank[i].lv[38][19] = Sequence_Bypass;
# Bank[i].lv[39][19] = Shifter_Bypass;
# Bank[i].lv[40][19] = StompBox_Bypass;
# Bank[i].lv[41][19] = Reverbtron_Bypass;
# Bank[i].lv[42][19] = Echotron_Bypass;
# Bank[i].lv[43][19] = StereoHarm_Bypass;
# Bank[i].lv[44][19] = CompBand_Bypass;
# Bank[i].lv[45][19] = Opticaltrem_Bypass;
# Bank[i].lv[46][19] = Vibe_Bypass;
# Bank[i].lv[47][19] = Infinity_Bypass;

