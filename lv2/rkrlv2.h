//Spencer Jackson
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
	IINF
};

#include<lv2.h>
#include<lv2/lv2plug.in/ns/ext/urid/urid.h>
#include<lv2/lv2plug.in/ns/ext/midi/midi.h>
#include<lv2/lv2plug.in/ns/ext/atom/util.h>
#include<lv2/lv2plug.in/ns/ext/atom/forge.h>
#include<lv2/lv2plug.in/ns/ext/time/time.h>
#include<lv2/lv2plug.in/ns/ext/buf-size/buf-size.h>
#include<lv2/lv2plug.in/ns/ext/options/options.h>
#include<lv2/lv2plug.in/ns/ext/atom/atom.h>
#include<lv2/lv2plug.in/ns/ext/patch/patch.h>
#include<lv2/lv2plug.in/ns/ext/worker/worker.h>
#include<lv2/lv2plug.in/ns/ext/state/state.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include"rkrlv2.h"

#include"EQ.h"
#include"Compressor.h"
#include"Distorsion.h"
#include"Echo.h"
#include"Chorus.h"
#include"APhaser.h"
#include"Harmonizer.h"
#include"RecChord.h"
#include"RecognizeNote.h"
#include"Exciter.h"
#include"Pan.h"
#include"Alienwah.h"
#include"Reverb.h"
#include"Cabinet.h"
#include"MusicDelay.h"
#include"DynamicFilter.h"
#include"NewDist.h"
#include"Valve.h"
#include"Dual_Flange.h"
#include"Ring.h"
#include"MBDist.h"
#include"Arpie.h"
#include"Expander.h"
#include"Shuffle.h"
#include"Synthfilter.h"
#include"MBVvol.h"
#include"RyanWah.h"
#include"RBEcho.h"
#include"CoilCrafter.h"
#include"ShelfBoost.h"
#include"Vocoder.h"
#include"Sequence.h"
#include"Shifter.h"
#include"StompBox.h"
#include"Reverbtron.h"
#include"Echotron.h"
#include"StereoHarm.h"
#include"CompBand.h"
#include"Opticaltrem.h"
#include"Vibe.h"
#include"Infinity.h"

//this is the default hopefully hosts don't use periods of more than this, or they will communicate the max bufsize
#define INTERMEDIATE_BUFSIZE 1024


typedef struct _RKRLV2
{
    uint8_t nparams;
    uint8_t effectindex;//index of effect
    uint16_t period_max;
    uint8_t loading_file;//flag to indicate that file load work is underway
    uint8_t file_changed;
    uint8_t prev_bypass;
    RvbFile* rvbfile;//file for reverbtron
    DlyFile* dlyfile;//file for echotron

    //ports
    float *input_l_p;
    float *input_r_p;
    float *output_l_p;
    float *output_r_p;
    float *bypass_p;
    const LV2_Atom_Sequence* atom_in_p;
    LV2_Atom_Sequence* atom_out_p;
    float *param_p[20];
    float *dbg_p;

    //various "advanced" lv2 stuffs
    LV2_Worker_Schedule* scheduler;
    LV2_Atom_Forge	forge;
    LV2_Atom_Forge_Frame atom_frame;
    LV2_URID_Map *urid_map;

    struct urids
    {
        LV2_URID    midi_MidiEvent;
        LV2_URID    atom_Float;
        LV2_URID    atom_Int;
        LV2_URID    atom_Object;
        LV2_URID    atom_Path;
        LV2_URID    atom_URID;
        LV2_URID    bufsz_max;
        LV2_URID    patch_Set;
        LV2_URID    patch_Get;
        LV2_URID	patch_property;
        LV2_URID 	patch_value;
        LV2_URID 	filetype_rvb;
        LV2_URID 	filetype_dly;

    } URIDs;

    //effect modules
    EQ* eq;             //0, 11
    Compressor* comp;   //1
    Distorsion* dist;   //2
    Echo* echo;         //3
    Chorus* chorus;     //4
    Analog_Phaser* aphase;//5
    Harmonizer* harm;	//6
    RecChord* chordID;
    Recognize* noteID;
    Exciter* exciter;	//7
    Pan* pan;			//8
    Alienwah* alien;    //9
    Reverb* reve;       //10
    Cabinet* cab;       //12
    MusicDelay* mdel; 	//13
    DynamicFilter* wah; //14
    NewDist* dere; 		//15
    Valve* valve;		//16
    Dflange* dflange;   //17
    Ring* ring;			//18
    MBDist* mbdist;		//19
    Arpie* arp;			//20
    Expander* expand;	//21
    Shuffle* shuf;		//22
    Synthfilter* synth; //23
    MBVvol* mbvol;		//24
    RyanWah* mutro; 	//25
    RBEcho* echoverse;	//26
    CoilCrafter* coil;	//27
    ShelfBoost* shelf;	//28
    Vocoder* voc;		//29
    Sustainer* sus;		//30
    Sequence* seq;		//31
    Shifter* shift;		//32
    StompBox* stomp;	//33,34
    Reverbtron* revtron;//35
    Echotron* echotron; //36
    StereoHarm* sharm;  //37
    CompBand* mbcomp; 	//38
    Opticaltrem* otrem; //39
    Vibe* vibe;			//40
    Infinity* inf;		//41
} RKRLV2;

enum other_ports
{
    //be sure to account for index of array vs lv2 port index
    HARMONIZER_MIDI,//unassigned right now
    VOCODER_AUX_IN = 7,
    VOCODER_VU_LEVEL = 8
};

// A few helper functions taken from the RKR object
static void
wetdry_mix (RKRLV2* plug, float mix, uint32_t period)
{
    unsigned int i;
    float v1, v2;

    if (mix < 0.5f)
    {
        v1 = 1.0f;
        v2 = mix * 2.0f;
    }
    else
    {
        v1 = (1.0f - mix) * 2.0f;
        v2 = 1.0f;
    };


//   Reverb and musical delay are exceptions for some reason?
//    if ((NumEffect == 8) || (NumEffect == 15))
//        v2 *= v2;

    for (i = 0; i < period; i++)
    {
        plug->output_l_p[i] = plug->input_r_p[i] * v2 + plug->output_l_p[i] * v1;
        plug->output_r_p[i] = plug->input_r_p[i] * v2 + plug->output_r_p[i] * v1;
    };

}

static void
xfade_in (RKRLV2* plug, uint32_t period)
{
	unsigned int i;
	float v = 0;
	float step = 1/(float)period;
	//just a linear fade since it's (hopefully) correlated
    for (i = 0; i < period; i++)
    {
    	plug->output_l_p[i] = (v)*plug->output_l_p[i] + (1-v)*plug->input_l_p[i];
    	plug->output_r_p[i] = (v)*plug->output_r_p[i] + (1-v)*plug->input_r_p[i];
    	v+=step;
    }
}

static void
xfade_out (RKRLV2* plug, uint32_t period)
{
	unsigned int i;
	float v = 0;
	float step = 1/(float)period;
	//just a linear fade since it's (hopefully) correlated
    for (i = 0; i < period; i++)
    {
    	plug->output_l_p[i] = (1-v)*plug->output_l_p[i] + v*plug->input_l_p[i];
    	plug->output_r_p[i] = (1-v)*plug->output_r_p[i] + v*plug->input_r_p[i];
    	v+=step;
    }
}

static void
xfade_check (RKRLV2* plug, uint32_t period)
{
    if(*plug->bypass_p)
    {
    	plug->prev_bypass = 1;
    	xfade_out(plug,period);
    }
    else if(plug->prev_bypass)
    {
    	plug->prev_bypass = 0;
    	xfade_in(plug,period);
    }
}

//TODO: make this return error is required feature not supported
static void getFeatures(RKRLV2* plug, const LV2_Feature * const* host_features)
{
    uint8_t i,j;
    plug->period_max = INTERMEDIATE_BUFSIZE;
    plug->loading_file = 0;
    plug->file_changed = 0;
    plug->scheduler = 0;
    plug->urid_map = 0;
    for(i=0; host_features[i]; i++)
    { if(!strcmp(host_features[i]->URI,LV2_OPTIONS__options)) { LV2_Options_Option* option; option = (LV2_Options_Option*)host_features[i]->data;
            for(j=0; option[j].key; j++)
            {
                if(option[j].key == plug->URIDs.bufsz_max)
                {
                    if(option[j].type == plug->URIDs.atom_Int)
                    {
                        plug->period_max = *(const int*)option[j].value;
                    }
                    //other types?
                }
            }
        }
        else if(!strcmp(host_features[i]->URI,LV2_WORKER__schedule))
        {
            plug->scheduler = (LV2_Worker_Schedule*)host_features[i]->data;
        }
        else if(!strcmp(host_features[i]->URI,LV2_URID__map))
        {
            plug->urid_map = (LV2_URID_Map *) host_features[i]->data;
            if(plug->urid_map)
            {
                plug->URIDs.midi_MidiEvent = plug->urid_map->map(plug->urid_map->handle,LV2_MIDI__MidiEvent);
                plug->URIDs.atom_Float = plug->urid_map->map(plug->urid_map->handle,LV2_ATOM__Float);
                plug->URIDs.atom_Int = plug->urid_map->map(plug->urid_map->handle,LV2_ATOM__Int);
                plug->URIDs.atom_Object = plug->urid_map->map(plug->urid_map->handle,LV2_ATOM__Object);
                plug->URIDs.atom_Path = plug->urid_map->map(plug->urid_map->handle,LV2_ATOM__Path);
                plug->URIDs.atom_URID = plug->urid_map->map(plug->urid_map->handle,LV2_ATOM__URID);
                plug->URIDs.bufsz_max = plug->urid_map->map(plug->urid_map->handle,LV2_BUF_SIZE__maxBlockLength);
                plug->URIDs.patch_Set = plug->urid_map->map(plug->urid_map->handle,LV2_PATCH__Set);
                plug->URIDs.patch_Get = plug->urid_map->map(plug->urid_map->handle,LV2_PATCH__Get);
                plug->URIDs.patch_property = plug->urid_map->map(plug->urid_map->handle,LV2_PATCH__property);
                plug->URIDs.patch_value = plug->urid_map->map(plug->urid_map->handle,LV2_PATCH__value);
                plug->URIDs.filetype_rvb = plug->urid_map->map(plug->urid_map->handle,RVBFILE_URI);
                plug->URIDs.filetype_dly = plug->urid_map->map(plug->urid_map->handle,DLYFILE_URI);

            }
        }
    }
}


static void cleanup_rkrlv2(LV2_Handle handle)
{
    RKRLV2* plug = (RKRLV2*)handle;
    switch(plug->effectindex)
    {
    case IEQ:
    case IEQP:
        delete plug->eq;//eql, eqp, cabinet
        break;
    case ICOMP:
        delete plug->comp;
        break;
    case IDIST:
        delete plug->dist;
        break;
    case IECHO:
        delete plug->echo;
        break;
    case ICHORUS:
        delete plug->chorus;
        break;
    case IAPHASE:
        delete plug->aphase;
        break;
    case IHARM_NM:
        delete plug->harm;
        delete plug->noteID;
        delete plug->chordID;
        break;
    case IEXCITER:
        delete plug->exciter;
        break;
    case IPAN:
        delete plug->pan;
        break;
    case IAWAH:
        delete plug->alien;
        break;
    case IREV:
        delete plug->reve;
        break;
    case ICAB:
        delete plug->cab;
        break;
    case IMDEL:
        delete plug->mdel;
        break;
    case IWAH:
        delete plug->wah;
        break;
    case IDERE:
        delete plug->dere;
        break;
    case IVALVE:
        delete plug->valve;
        break;
    case IDFLANGE:
        delete plug->dflange;
        break;
    case IRING:
        delete plug->noteID;
        delete plug->ring;
        break;
    case IMBDIST:
        delete plug->mbdist;
        break;
    case IARPIE:
        delete plug->arp;
        break;
    case IEXPAND:
        delete plug->expand;
        break;
    case ISHUFF:
        delete plug->shuf;
        break;
    case ISYNTH:
        delete plug->synth;
        break;
    case IMBVOL:
        delete plug->mbvol;
        break;
    case IMUTRO:
        delete plug->mutro;
        break;
    case IECHOVERSE:
        delete plug->echoverse;
        break;
    case ICOIL:
        delete plug->coil;
        break;
    case ISHELF:
        delete plug->shelf;
        break;
    case IVOC:
        delete plug->voc;
        break;
    case ISUS:
        delete plug->sus;
        break;
    case ISEQ:
        delete plug->seq;
        break;
    case ISHIFT:
        delete plug->shift;
        break;
    case ISTOMP:
    case IFUZZ:
        delete plug->stomp;
        break;
    case IREVTRON:
        delete plug->revtron;
        delete plug->rvbfile;
        break;
    case IECHOTRON:
        delete plug->echotron;
        delete plug->dlyfile;
        break;
    case ISHARM_NM:
    	delete plug->sharm;
        delete plug->noteID;
        delete plug->chordID;
    	break;
    case IMBCOMP:
    	delete plug->mbcomp;
    	break;
    case IOPTTREM:
    	delete plug->otrem;
    	break;
    case IVIBE:
    	delete plug->vibe;
    	break;
    case IINF:
    	delete plug->inf;
    	break;
    }
    free(plug);
}

static void connect_rkrlv2_ports_w_atom(LV2_Handle handle, uint32_t port, void *data)
{
    RKRLV2* plug = (RKRLV2*)handle;
    switch(port)
    {
    case INL:
        plug->input_l_p = (float*)data;
        break;
    case INR:
        plug->input_r_p = (float*)data;
        break;
    case OUTL:
        plug->output_l_p = (float*)data;
        break;
    case OUTR:
        plug->output_r_p = (float*)data;
        break;
    case BYPASS:
        plug->atom_in_p = (const LV2_Atom_Sequence*)data;
        break;
    case PARAM0:
        plug->atom_out_p = (LV2_Atom_Sequence*)data;
        break;
    case PARAM1:
        plug->bypass_p = (float*)data;
        break;
    case PARAM2:
        plug->param_p[0] = (float*)data;
        break;
    case PARAM3:
        plug->param_p[1] = (float*)data;
        break;
    case PARAM4:
        plug->param_p[2] = (float*)data;
        break;
    case PARAM5:
        plug->param_p[3] = (float*)data;
        break;
    case PARAM6:
        plug->param_p[4] = (float*)data;
        break;
    case PARAM7:
        plug->param_p[5] = (float*)data;
        break;
    case PARAM8:
        plug->param_p[6] = (float*)data;
        break;
    case PARAM9:
        plug->param_p[7] = (float*)data;
        break;
    case PARAM10:
        plug->param_p[8] = (float*)data;
        break;
    case PARAM11:
        plug->param_p[9] = (float*)data;
        break;
    case PARAM12:
        plug->param_p[10] = (float*)data;
        break;
    case PARAM13:
        plug->param_p[11] = (float*)data;
        break;
    case PARAM14:
        plug->param_p[12] = (float*)data;
        break;
    case PARAM15:
        plug->param_p[13] = (float*)data;
        break;
    case PARAM16:
        plug->param_p[14] = (float*)data;
        break;
    case PARAM17:
        plug->param_p[15] = (float*)data;
        break;
    case PARAM18:
        plug->param_p[16] = (float*)data;
        break;
    case DBG:
        plug->param_p[17] = (float*)data;
        break;
    case EXTRA:
        plug->param_p[18] = (float*)data;
        break;
    default:
        puts("UNKNOWN PORT YO!!");
    }
}

static void connect_rkrlv2_ports(LV2_Handle handle, uint32_t port, void *data)
{
    RKRLV2* plug = (RKRLV2*)handle;
    switch(port)
    {
    case INL:
        plug->input_l_p = (float*)data;
        break;
    case INR:
        plug->input_r_p = (float*)data;
        break;
    case OUTL:
        plug->output_l_p = (float*)data;
        break;
    case OUTR:
        plug->output_r_p = (float*)data;
        break;
    case BYPASS:
        plug->bypass_p = (float*)data;
        break;
    case PARAM0:
        plug->param_p[0] = (float*)data;
        break;
    case PARAM1:
        plug->param_p[1] = (float*)data;
        break;
    case PARAM2:
        plug->param_p[2] = (float*)data;
        break;
    case PARAM3:
        plug->param_p[3] = (float*)data;
        break;
    case PARAM4:
        plug->param_p[4] = (float*)data;
        break;
    case PARAM5:
        plug->param_p[5] = (float*)data;
        break;
    case PARAM6:
        plug->param_p[6] = (float*)data;
        break;
    case PARAM7:
        plug->param_p[7] = (float*)data;
        break;
    case PARAM8:
        plug->param_p[8] = (float*)data;
        break;
    case PARAM9:
        plug->param_p[9] = (float*)data;
        break;
    case PARAM10:
        plug->param_p[10] = (float*)data;
        break;
    case PARAM11:
        plug->param_p[11] = (float*)data;
        break;
    case PARAM12:
        plug->param_p[12] = (float*)data;
        break;
    case PARAM13:
        plug->param_p[13] = (float*)data;
        break;
    case PARAM14:
        plug->param_p[14] = (float*)data;
        break;
    case PARAM15:
        plug->param_p[15] = (float*)data;
        break;
    case PARAM16:
        plug->param_p[16] = (float*)data;
        break;
    case PARAM17:
        plug->param_p[17] = (float*)data;
        break;
    case PARAM18:
        plug->param_p[18] = (float*)data;
        break;
    case DBG:
        plug->dbg_p = (float*)data;
        break;
    default:
        puts("UNKNOWN PORT YO!!");
    }
}

#endif
