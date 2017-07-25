#ifndef RKRLV2_H
#define RKRLV2_H

#define EQLV2_URI "http://rakarrack.sourceforge.net/effects.html#eql"
#define COMPLV2_URI "http://rakarrack.sourceforge.net/effects.html#comp"
#define DISTLV2_URI "http://rakarrack.sourceforge.net/effects.html#dist"
#define ECHOLV2_URI "http://rakarrack.sourceforge.net/effects.html#eco"
#define CHORUSLV2_URI "http://rakarrack.sourceforge.net/effects.html#chor"
#define APHASELV2_URI "http://rakarrack.sourceforge.net/effects.html#aphas"
#define PANLV2_URI "http://rakarrack.sourceforge.net/effects.html#pan"
#define ALIENLV2_URI "http://rakarrack.sourceforge.net/effects.html#awha"
#define HARMNOMIDLV2_URI "http://rakarrack.sourceforge.net/effects.html#har_no_mid"
#define EXCITERLV2_URI "http://rakarrack.sourceforge.net/effects.html#Exciter"
#define REVELV2_URI "http://rakarrack.sourceforge.net/effects.html#reve"
#define EQPLV2_URI "http://rakarrack.sourceforge.net/effects.html#eqp"
#define CABLV2_URI "http://rakarrack.sourceforge.net/effects.html#cabe"
#define MDELLV2_URI "http://rakarrack.sourceforge.net/effects.html#delm"
#define WAHLV2_URI "http://rakarrack.sourceforge.net/effects.html#wha"
#define DERELV2_URI "http://rakarrack.sourceforge.net/effects.html#dere"
#define VALVELV2_URI "http://rakarrack.sourceforge.net/effects.html#Valve"
#define DFLANGELV2_URI "http://rakarrack.sourceforge.net/effects.html#Dual_Flange"
#define RINGLV2_URI "http://rakarrack.sourceforge.net/effects.html#Ring"
#define MBDISTLV2_URI "http://rakarrack.sourceforge.net/effects.html#DistBand"
#define ARPIELV2_URI "http://rakarrack.sourceforge.net/effects.html#Arpie"
#define EXPANDLV2_URI "http://rakarrack.sourceforge.net/effects.html#Expander"
#define SHUFFLELV2_URI "http://rakarrack.sourceforge.net/effects.html#Shuffle"
#define SYNTHLV2_URI "http://rakarrack.sourceforge.net/effects.html#Synthfilter"
#define MBVOLLV2_URI "http://rakarrack.sourceforge.net/effects.html#VaryBand"
#define MUTROLV2_URI "http://rakarrack.sourceforge.net/effects.html#MuTroMojo"
#define ECHOVERSELV2_URI "http://rakarrack.sourceforge.net/effects.html#Echoverse"
#define COILLV2_URI "http://rakarrack.sourceforge.net/effects.html#CoilCrafter"
#define SHELFLV2_URI "http://rakarrack.sourceforge.net/effects.html#ShelfBoost"
#define VOCODERLV2_URI "http://rakarrack.sourceforge.net/effects.html#Vocoder"
#define SUSTAINLV2_URI "http://rakarrack.sourceforge.net/effects.html#Sustainer"
#define SEQUENCELV2_URI "http://rakarrack.sourceforge.net/effects.html#Sequence"
#define SHIFTERLV2_URI "http://rakarrack.sourceforge.net/effects.html#Shifter"
#define STOMPLV2_URI "http://rakarrack.sourceforge.net/effects.html#StompBox"
#define STOMPFUZZLV2_URI "http://rakarrack.sourceforge.net/effects.html#StompBox_fuzz"
#define REVTRONLV2_URI "http://rakarrack.sourceforge.net/effects.html#Reverbtron"
#define ECHOTRONLV2_URI "http://rakarrack.sourceforge.net/effects.html#Echotron"
#define SHARMNOMIDLV2_URI "http://rakarrack.sourceforge.net/effects.html#StereoHarm_no_mid"
#define HARMLV2_URI "http://rakarrack.sourceforge.net/effects.html#har"
#define MBCOMPLV2_URI "http://rakarrack.sourceforge.net/effects.html#CompBand"
#define OPTTREMLV2_URI "http://rakarrack.sourceforge.net/effects.html#Otrem"
#define VIBELV2_URI "http://rakarrack.sourceforge.net/effects.html#Vibe"
#define INFLV2_URI "http://rakarrack.sourceforge.net/effects.html#Infinity"
#define PHASELV2_URI "http://rakarrack.sourceforge.net/effects.html#phas"
#define GATELV2_URI "http://rakarrack.sourceforge.net/effects.html#gate"

#define RVBFILE_URI "http://rakarrack.sourceforge.net/effects.html#Reverbtron:rvbfile"
#define DLYFILE_URI "http://rakarrack.sourceforge.net/effects.html#Echotron:dlyfile"

enum RKRLV2_ports_
{
    INL =0,
    INR,
    OUTL,
    OUTR,
    BYPASS,
    PARAM0,
    PARAM1,
    PARAM2,
    PARAM3,
    PARAM4,
    PARAM5,
    PARAM6,
    PARAM7,
    PARAM8,
    PARAM9,
    PARAM10,
    PARAM11,
    PARAM12,
    PARAM13,
    PARAM14,
    PARAM15,
    PARAM16,
    PARAM17,
    PARAM18,
    DBG,
    EXTRA
};


//just make sure these line up with the order in manifest.ttl
enum RKRLV2_effects_
{
	IEQ =0,
	ICOMP,
	IDIST,
	IECHO,
	ICHORUS,
	IAPHASE,
	IHARM_NM,
	IEXCITER,
	IPAN,
	IAWAH,
	IREV,//10
	IEQP,
	ICAB,
	IMDEL,
	IWAH,
	IDERE,
	IVALVE,
	IDFLANGE,
	IRING,
	IMBDIST,
	IARPIE,//20
	IEXPAND,
	ISHUFF,
	ISYNTH,
	IMBVOL,
	IMUTRO,
	IECHOVERSE,
	ICOIL,
	ISHELF,
	IVOC,
	ISUS,//30
	ISEQ,
	ISHIFT,
	ISTOMP,
	IFUZZ,
	IREVTRON,
	IECHOTRON,
	ISHARM_NM,
	IMBCOMP,
	IOPTTREM,
	IVIBE,//40
	IINF,
	IPHASE,
        IGATE
};

#endif
