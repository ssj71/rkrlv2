//Spencer Jackson
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
#include<samplerate.h>
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
#include"Phaser.h"
#include"Gate.h"

//this is the default hopefully hosts don't use periods of more than this, or they will communicate the max bufsize
#define INTERMEDIATE_BUFSIZE 1024
#define MAX_INPLACE 8192


typedef struct _RKRLV2
{
    uint8_t nparams;
    uint8_t effectindex;//index of effect
    uint16_t period_max;
    uint8_t loading_file;//flag to indicate that file load work is underway
    uint8_t init_params; //flag to indicate to force parameter (LFO) update & sample update on first run
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

    // no-inplace buffers
    float input_buf_l[MAX_INPLACE];
    float input_buf_r[MAX_INPLACE];

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
    Phaser* phase;		//42
    Gate* gate;         //43
} RKRLV2;

enum other_ports
{
    //be sure to account for index of array vs lv2 port index
    HARMONIZER_MIDI,//unassigned right now
    VOCODER_AUX_IN = 7,
    VOCODER_VU_LEVEL = 8
};

// A few helper functions taken from the RKR object
void
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
        plug->output_l_p[i] = plug->input_l_p[i] * v2 + plug->output_l_p[i] * v1;
        plug->output_r_p[i] = plug->input_r_p[i] * v2 + plug->output_r_p[i] * v1;
    };

}

//this finds the peak value
int
have_signal(float* efxoutl, float* efxoutr, uint32_t period)
{
    float tmp;
    uint32_t i;
    float il_sum = 1e-12f;
    float ir_sum = 1e-12f;
    for (i = 0; i <= period; i++) {

        tmp = fabsf (efxoutl[i]);
        if (tmp > il_sum) il_sum = tmp;
        tmp = fabsf (efxoutr[i]);
        if (tmp > ir_sum) ir_sum = tmp;
    }


    if ((il_sum+ir_sum) > 0.0004999f)  return  1;
    else  return 0;
}

static void
inplace_check (RKRLV2* plug, uint32_t period)
{
    if (period > MAX_INPLACE) {
        return;
    }
    if (plug->input_l_p == plug->output_l_p) {
        memcpy (plug->input_buf_l, plug->input_l_p, sizeof(float) * period);
        plug->input_l_p = plug->input_buf_l;
    }
    if (plug->input_r_p == plug->output_r_p) {
        memcpy (plug->input_buf_r, plug->input_r_p, sizeof(float) * period);
        plug->input_r_p = plug->input_buf_l;
    }
}

static void
bypass_stereo (RKRLV2* plug, uint32_t nframes)
{
    // copy only if needed. memcpy() src/dest memory areas must not overlap.
    if (plug->output_l_p != plug->input_l_p) {
        memcpy(plug->output_l_p,plug->input_l_p,sizeof(float)*nframes);
    }
    if (plug->output_r_p != plug->input_r_p) {
        memcpy(plug->output_r_p,plug->input_r_p,sizeof(float)*nframes);
    }
}

static void
prepare_inplace (RKRLV2* plug, uint32_t nframes)
{
	/* When a plugin processes in-place, but the
	 * host provides separate i/o buffers,
	 * copy inputs to outputs */
	bypass_stereo(plug,nframes);

	/* when a host provides inplace buffers,
	 * make a backup f x-fade IFF required
	 */
	if (*plug->bypass_p || plug->prev_bypass) {
		inplace_check (plug,nframes);
	}
}

void
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

void
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

void
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
void getFeatures(RKRLV2* plug, const LV2_Feature * const* host_features)
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

/////////////////////////////////////////
//      EFFECTS
////////////////////////////////////////

///// EQ /////////
LV2_Handle init_eqlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 12;
    plug->effectindex = IEQ;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->eq = new EQ(0,0,sample_freq, plug->period_max);

    //eq has a bunch of setup stuff. Why isn't this in the EQ initalizer?
    for (int i = 0; i <= 45; i += 5)
    {
        plug->eq->changepar (i + 10, 7);
        plug->eq->changepar (i + 14, 0);
    }

    plug->eq->changepar (11, 31);
    plug->eq->changepar (16, 63);
    plug->eq->changepar (21, 125);
    plug->eq->changepar (26, 250);
    plug->eq->changepar (31, 500);
    plug->eq->changepar (36, 1000);
    plug->eq->changepar (41, 2000);
    plug->eq->changepar (46, 4000);
    plug->eq->changepar (51, 8000);
    plug->eq->changepar (56, 16000);
    return plug;
}

void run_eqlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    i = 0;
    val = (int)*plug->param_p[0]+64;//gain
    if(plug->eq->getpar(0) != val)
    {
        plug->eq->changepar(0,val);
    }
    val = (int)*plug->param_p[1]+64;//q
    if(plug->eq->getpar(13) != val)
    {
        int j;
        for(j=0; j<10; j++)
        {
            plug->eq->changepar(j*5+13,val);
        }
    }
    for(i=2; i<plug->nparams; i++)
    {
        val = (int)*plug->param_p[i]+64;//various freq. bands
        if(plug->eq->getpar(5*i + 2) != val)
        {
            plug->eq->changepar(5*i+2,val);
        }
    }

    //eq does process in-place ?
    prepare_inplace (plug, nframes);

    //now set out ports
    plug->eq->efxoutl = plug->output_l_p;
    plug->eq->efxoutr = plug->output_r_p;

    //now run
    plug->eq->out(plug->output_l_p,plug->output_r_p,nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->eq->cleanup();
    }

    return;
}

///// comp /////////
LV2_Handle init_complv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 9;
    plug->effectindex = ICOMP;
    plug->prev_bypass = 1;

    plug->comp = new Compressor(0,0, sample_freq);

    return plug;
}

void run_complv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    for(i=0; i<plug->nparams; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->comp->getpar(i+1) != val)//this effect is 1 indexed
        {
            plug->comp->Compressor_Change(i+1,val);
        }
    }

    //comp does process in-place
    prepare_inplace (plug, nframes);

    //now set out ports
    plug->comp->efxoutl = plug->output_l_p;
    plug->comp->efxoutr = plug->output_r_p;

    //now run
    plug->comp->out(plug->output_l_p,plug->output_r_p,nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->comp->cleanup();
    }
    return;
}

///// dist /////////
LV2_Handle init_distlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 12;
    plug->effectindex = IDIST;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->dist = new Distorsion(0,0, sample_freq, plug->period_max, /*oversampling*/2,
                                /*up interpolation method*/4, /*down interpolation method*/2);

    return plug;
}

void run_distlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    i=0;
    val = (int)*plug->param_p[i];//0 Wet/dry
    if(plug->dist->getpar(i) != val)
    {
        plug->dist->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i]+64;//1 pan
    if(plug->dist->getpar(i) != val)
    {
        plug->dist->changepar(i,val);
    }
    for(i++; i<plug->nparams-1; i++) //2-10
    {
        val = (int)*plug->param_p[i];
        if(plug->dist->getpar(i) != val)
        {
            plug->dist->changepar(i,val);
        }
    }
    val = (int)*plug->param_p[i++];//skip one index, 12 octave
    if(plug->dist->getpar(i) != val)
    {
        plug->dist->changepar(i,val);
    }

    inplace_check(plug,nframes);

    //now set out ports and global period size
    plug->dist->efxoutl = plug->output_l_p;
    plug->dist->efxoutr = plug->output_r_p;

    //now run
    plug->dist->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->dist->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->dist->cleanup();
    }
    return;
}

///// echo /////////
LV2_Handle init_echolv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 9;
    plug->effectindex = IECHO;
    plug->prev_bypass = 1;

    plug->echo = new Echo(0,0,sample_freq);

    return plug;
}

void run_echolv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;


    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    i=0;
    val = (int)*plug->param_p[i];//wet/dry
    if(plug->echo->getpar(i) != val)
    {
        plug->echo->changepar(i,val);
    }
    i++;//panning is offset
    val = (int)*plug->param_p[i]+64;
    if(plug->echo->getpar(i) != val)
    {
        plug->echo->changepar(i,val);
    }
    i++;//delay is not offset
    val = (int)*plug->param_p[i];
    if(plug->echo->getpar(i) != val)
    {
        plug->echo->changepar(i,val);
    }
    i++;//LR delay is offset
    val = (int)*plug->param_p[i]+64;
    if(plug->echo->getpar(i) != val)
    {
        plug->echo->changepar(i,val);
    }
    for(i++; i<plug->nparams; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->echo->getpar(i) != val)
        {
            plug->echo->changepar(i,val);
        }
    }

    inplace_check(plug,nframes);

    //now set out ports and global period size
    plug->echo->efxoutl = plug->output_l_p;
    plug->echo->efxoutr = plug->output_r_p;

    //now run
    plug->echo->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->echo->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->echo->cleanup();
    }
    return;
}

///// chorus/flanger /////////
LV2_Handle init_choruslv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 12;
    plug->effectindex = ICHORUS;
    plug->prev_bypass = 1;

    plug->chorus = new Chorus(0,0,sample_freq);

    return plug;
}

void run_choruslv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;


    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //LFO effects require period be set before setting other params
    plug->chorus->PERIOD = nframes;

    //check and set changed parameters
    i=0;
    val = (int)*plug->param_p[i];
    if(plug->chorus->getpar(i) != val)
    {
        plug->chorus->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i] +64;// 1 pan offset
    if(plug->chorus->getpar(i) != val)
    {
        plug->chorus->changepar(i,val);
    }
    for(i++; i<5; i++) //2-4
    {
        val = (int)*plug->param_p[i];
        if(plug->chorus->getpar(i) != val)
        {
            plug->chorus->changepar(i,val);
        }
    }
    val = (int)*plug->param_p[i] +64;// 5 LR Del. offset
    if(plug->chorus->getpar(i) != val)
    {
        plug->chorus->changepar(i,val);
    }
    for(i++; i<10; i++) // 6-9
    {
        val = (int)*plug->param_p[i];
        if(plug->chorus->getpar(i) != val)
        {
            plug->chorus->changepar(i,val);
        }
    }
    //skip param 10
    for(; i<plug->nparams; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->chorus->getpar(i+1) != val)
        {
            plug->chorus->changepar(i+1,val);
        }
    }

    inplace_check(plug,nframes);

    //now set out ports and global period size
    plug->chorus->efxoutl = plug->output_l_p;
    plug->chorus->efxoutr = plug->output_r_p;

    //now run
    plug->chorus->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->chorus->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->chorus->cleanup();
    }
    return;
}

///// Analog Phaser /////////
LV2_Handle init_aphaselv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 13;
    plug->effectindex = IAPHASE;
    plug->prev_bypass = 1;

    plug->aphase = new Analog_Phaser(0,0,sample_freq);

    return plug;
}

void run_aphaselv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //LFO effects require period be set before setting other params
    plug->aphase->PERIOD = nframes;

    //check and set changed parameters
    for(i=0; i<5; i++) //0-4
    {
        val = (int)*plug->param_p[i];
        if(plug->aphase->getpar(i) != val)
        {
            plug->aphase->changepar(i,val);
        }
    }
    val = (int)*plug->param_p[i] +64;// 5 LR Del. offset
    if(plug->aphase->getpar(i) != val)
    {
        plug->aphase->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i];// 6
    if(plug->aphase->getpar(i) != val)
    {
        plug->aphase->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i] +64;// 7 Fb offset
    if(plug->aphase->getpar(i) != val)
    {
        plug->aphase->changepar(i,val);
    }
    for(i++; i<plug->nparams; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->aphase->getpar(i) != val)
        {
            plug->aphase->changepar(i,val);
        }
    }

    inplace_check(plug,nframes);

    //now set out ports and global period size
    plug->aphase->efxoutl = plug->output_l_p;
    plug->aphase->efxoutr = plug->output_r_p;

    //now run
    plug->aphase->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->aphase->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->aphase->cleanup();
    }
    return;
}

//////// harmonizer /////////
LV2_Handle init_harmnomidlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 10;
    plug->effectindex = IHARM_NM;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    //magic numbers: shift qual 4, downsample 5, up qual 4, down qual 2,
    plug->harm = new Harmonizer(0,0,4,5,SRC_LINEAR,SRC_SINC_FASTEST, plug->period_max, sample_freq);
    plug->noteID = new Recognize(0,0,.6, sample_freq, 440.0, plug->period_max);//.6 is default trigger value
    plug->chordID = new RecChord();



    return plug;
}

void run_harmnomidlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    i = 0;
    val = (int)*plug->param_p[i];// 0 wet/dry
    if(plug->harm->getpar(i) != val)
    {
        plug->harm->changepar(i,val);
    }
    for(i++; i<3; i++) //1-2
    {
        val = (int)*plug->param_p[i] + 64;
        if(plug->harm->getpar(i) != val)
        {
            plug->harm->changepar(i,val);
        }
    }
    val = (int)*plug->param_p[i] + 12;// 3 interval
    if(plug->harm->getpar(i) != val)
    {
        plug->harm->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i];//4 filter freq
    if(plug->harm->getpar(i) != val)
    {
        plug->harm->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i];//5 select mode
    if(plug->harm->getpar(i) != val)
    {
        plug->harm->changepar(i,val);
        plug->chordID->cleanup();
        if(!val) plug->harm->changepar(3,plug->harm->getpar(3));
    }
    for(i++; i<8; i++) //6-7
    {
        val = (int)*plug->param_p[i];
        if(plug->harm->getpar(i) != val)
        {
            plug->harm->changepar(i,val);
            plug->chordID->ctipo = plug->harm->getpar(7);//set chord type
            plug->chordID->fundi = plug->harm->getpar(6);//set root note
            plug->chordID->cc = 1;//mark chord has changed
        }
    }
    for(; i<10; i++) // 8-9
    {
        val = (int)*plug->param_p[i] + 64;
        if(plug->harm->getpar(i) != val)
        {
            plug->harm->changepar(i,val);
        }
    }
// midi mode, not implementing midi here
//    val = (int)*plug->param_p[i];// 10 midi mode
//    if(plug->aphase->getpar(i) != val)
//    {
//        plug->aphase->changepar(i,val);
//        if(!val) plug->harm->changepar(3,plug->harm->getpar(3));
//    }
    /*
    see Chord() in rkr.fl
    harmonizer, need recChord and recNote.
    see process.C ln 1507
    */

    //TODO may need to make sure input is over some threshold
    if(plug->harm->mira && plug->harm->PSELECT)
//        && have_signal( plug->input_l_p, plug->input_r_p, nframes))
    {
        plug->noteID->schmittFloat(plug->input_l_p,plug->input_r_p,nframes);
        if(plug->noteID->reconota != -1 && plug->noteID->reconota != plug->noteID->last)
        {
            if(plug->noteID->afreq > 0.0)
            {
                plug->chordID->Vamos(0,plug->harm->Pinterval - 12,plug->noteID->reconota);
                plug->harm->r_ratio = plug->chordID->r__ratio[0];//pass the found ratio
            }
        }
    }

    inplace_check(plug,nframes);

    //now set out ports and global period size
    plug->harm->efxoutl = plug->output_l_p;
    plug->harm->efxoutr = plug->output_r_p;

    //now run
    plug->harm->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->harm->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->harm->cleanup();
    }
    return;
}


///// exciter /////////
LV2_Handle init_exciterlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 13;
    plug->effectindex = IEXCITER;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->exciter = new Exciter(0,0, sample_freq, plug->period_max);

    return plug;
}

void run_exciterlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;


    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    for(i=0; i<plug->nparams; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->exciter->getpar(i) != val)
        {
            plug->exciter->changepar(i,val);
        }
    }

    //comp does process in-place
    prepare_inplace (plug, nframes);

    //now set out ports
    plug->exciter->efxoutl = plug->output_l_p;
    plug->exciter->efxoutr = plug->output_r_p;

    //now run
    plug->exciter->out(plug->output_l_p,plug->output_r_p,nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->exciter->cleanup();
    }
    return;
}

///// pan /////////
LV2_Handle init_panlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 9;
    plug->effectindex = IPAN;
    plug->prev_bypass = 1;

    plug->pan = new Pan(0,0,sample_freq);

    return plug;
}

void run_panlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //LFO effects require period be set before setting other params
    plug->pan->PERIOD = nframes;

    //check and set changed parameters
    i = 0;
    val = (int)*plug->param_p[i];// 0 wet/dry
    if(plug->pan->getpar(i) != val)
    {
        plug->pan->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i] + 64;// 1 pan
    if(plug->pan->getpar(i) != val)
    {
        plug->pan->changepar(i,val);
    }
    for(i++; i<5; i++) //2-4
    {
        val = (int)*plug->param_p[i];
        if(plug->pan->getpar(i) != val)
        {
            plug->pan->changepar(i,val);
        }
    }
    val = (int)*plug->param_p[i] +64;// 5 LR Del. offset
    if(plug->pan->getpar(i) != val)
    {
        plug->pan->changepar(i,val);
    }
    for(i++; i<plug->nparams; i++) //6-8
    {
        val = (int)*plug->param_p[i];
        if(plug->pan->getpar(i) != val)
        {
            plug->pan->changepar(i,val);
        }
    }

    inplace_check(plug,nframes);

    //now set out ports and global period size
    plug->pan->efxoutl = plug->output_l_p;
    plug->pan->efxoutr = plug->output_r_p;

    //now run
    plug->pan->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->pan->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->pan->cleanup();
    }
    return;
}

///// AlienWah /////////
LV2_Handle init_alienlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 11;
    plug->effectindex = IAWAH;
    plug->prev_bypass = 1;

    plug->alien = new Alienwah(0,0,sample_freq);

    return plug;
}

void run_alienlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;


    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //LFO effects require period be set before setting other params
    plug->alien->PERIOD = nframes;

    //check and set changed parameters
    i=0;
    val = (int)*plug->param_p[i];//0 Wet/Dry
    if(plug->alien->getpar(i) != val)
    {
        plug->alien->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i] +64;// 1 pan is offset
    if(plug->alien->getpar(i) != val)
    {
        plug->alien->changepar(i,val);
    }
    for(i++; i<5; i++) //2-4
    {
        val = (int)*plug->param_p[i];
        if(plug->alien->getpar(i) != val)
        {
            plug->alien->changepar(i,val);
        }
    }
    val = (int)*plug->param_p[i] +64;// 5 LR Del. offset
    if(plug->alien->getpar(i) != val)
    {
        plug->alien->changepar(i,val);
    }
    for(i++; i<plug->nparams; i++) //6-10
    {
        val = (int)*plug->param_p[i];
        if(plug->alien->getpar(i) != val)
        {
            plug->alien->changepar(i,val);
        }
    }

    inplace_check(plug,nframes);

    //now set out ports and global period size
    plug->alien->efxoutl = plug->output_l_p;
    plug->alien->efxoutr = plug->output_r_p;

    //now run
    plug->alien->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->alien->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->alien->cleanup();
    }
    return;
}

///// reverb /////////
LV2_Handle init_revelv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 10;
    plug->effectindex = IREV;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->reve = new Reverb(0,0,sample_freq,plug->period_max);

    return plug;
}

void run_revelv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;


    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    i=0;
    val = (int)*plug->param_p[i];//0 Wet/Dry
    if(plug->reve->getpar(i) != val)
    {
        plug->reve->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i] +64;// 1 pan is offset
    if(plug->reve->getpar(i) != val)
    {
        plug->reve->changepar(i,val);
    }
    for(i++; i<5; i++) //2-4
    {
        val = (int)*plug->param_p[i];
        if(plug->reve->getpar(i) != val)
        {
            plug->reve->changepar(i,val);
        }
    }
    for(; i<plug->nparams; i++) //7-11 (5 and 6 are skipped
    {
        val = (int)*plug->param_p[i];
        if(plug->reve->getpar(i+2) != val)
        {
            plug->reve->changepar(i+2,val);
        }
    }

    inplace_check(plug,nframes);

    //now set out ports and global period size
    plug->reve->efxoutl = plug->output_l_p;
    plug->reve->efxoutr = plug->output_r_p;

    //now run
    plug->reve->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->reve->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->reve->cleanup();
    }
    return;
}

///// EQ Parametric /////////
LV2_Handle init_eqplv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 10;
    plug->effectindex = IEQP;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->eq = new EQ(0,0,sample_freq, plug->period_max);

    //eq has a bunch of setup stuff. Why isn't this in the EQ initalizer?
    for (int i = 0; i <= 10; i += 5)
    {
        plug->eq->changepar (i + 10, 7);
        plug->eq->changepar (i + 13, 64);
        plug->eq->changepar (i + 14, 0);

    }
    return plug;
}

void run_eqplv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    i = 0;

    val = (int)*plug->param_p[0]+64;//gain
    if(plug->eq->getpar(0) != val)
    {
        plug->eq->changepar(0,val);
    }

    for(i=1; i<4; i++) //1-3 low band
    {
        val = (int)*plug->param_p[i]+64;
        if(plug->eq->getpar(i + 10) != val)
        {
            plug->eq->changepar(i+10,val);
        }
    }
    for(; i<7; i++) //4-6 mid band
    {
        val = (int)*plug->param_p[i]+64;
        if(plug->eq->getpar(i + 12) != val)
        {
            plug->eq->changepar(i+12,val);
        }
    }
    for(; i<plug->nparams; i++) //7-9 high band
    {
        val = (int)*plug->param_p[i]+64;
        if(plug->eq->getpar(i + 14) != val)
        {
            plug->eq->changepar(i+14,val);
        }
    }

    //eq does process in-place ?
    prepare_inplace (plug, nframes);

    //now set out ports
    plug->eq->efxoutl = plug->output_l_p;
    plug->eq->efxoutr = plug->output_r_p;

    //now run
    plug->eq->out(plug->output_l_p,plug->output_r_p,nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->eq->cleanup();
    }
    return;
}

///// Cabinet /////////
LV2_Handle init_cablv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 10;
    plug->effectindex = ICAB;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->cab = new Cabinet(0,0,sample_freq, plug->period_max);

    return plug;
}

void run_cablv2(LV2_Handle handle, uint32_t nframes)
{
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    val = (int)*plug->param_p[0]+64;//gain
    if(plug->cab->getpar(0) != val)
    {
        plug->cab->changepar(0,val);
    }

    val = (int)*plug->param_p[1];
    if(plug->cab->Cabinet_Preset != val)
    {
        plug->cab->setpreset(val);
    }

    //cab does process in-place?
    prepare_inplace (plug, nframes);

    //now set out ports
    plug->cab->efxoutl = plug->output_l_p;
    plug->cab->efxoutr = plug->output_r_p;

    //now run
    plug->cab->out(plug->output_l_p,plug->output_r_p,nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->cab->cleanup();
    }
    return;
}

///// Musical Delay /////////
LV2_Handle init_mdellv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 13;
    plug->effectindex = IMDEL;
    plug->prev_bypass = 1;

    plug->mdel = new MusicDelay (0,0,sample_freq);

    return plug;
}

void run_mdellv2(LV2_Handle handle, uint32_t nframes)
{
    int val;
    uint8_t i;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    i = 0;

    val = (int)*plug->param_p[0];//wetdry
    if(plug->mdel->getpar(0) != val)
    {
        plug->mdel->changepar(0,val);
    }
    val = (int)*plug->param_p[1]+64;//pan1
    if(plug->mdel->getpar(1) != val)
    {
        plug->mdel->changepar(1,val);
    }

    for(i=2; i<7; i++) //2-6
    {
        val = (int)*plug->param_p[i];
        if(plug->mdel->getpar(i) != val)
        {
            plug->mdel->changepar(i,val);
        }
    }
    val = (int)*plug->param_p[i]+64;//pan2
    if(plug->mdel->getpar(i) != val)
    {
        plug->mdel->changepar(i,val);
    }
    for(i++; i<plug->nparams; i++) //8-12
    {
        val = (int)*plug->param_p[i];
        if(plug->mdel->getpar(i) != val)
        {
            plug->mdel->changepar(i,val);
        }
    }

    inplace_check(plug,nframes);

    //now set out ports and global period size
    plug->mdel->efxoutl = plug->output_l_p;
    plug->mdel->efxoutr = plug->output_r_p;

    //now run
    plug->mdel->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->mdel->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->mdel->cleanup();
    }
    return;
}

///// wahwah /////////
LV2_Handle init_wahlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 11;
    plug->effectindex = IWAH;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->wah = new DynamicFilter(0,0,sample_freq, plug->period_max);

    return plug;
}

void run_wahlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //LFO effects require period be set before setting other params
    plug->wah->PERIOD = nframes;

    //check and set changed parameters
    i=0;
    val = (int)*plug->param_p[i];
    if(plug->wah->getpar(i) != val)
    {
        plug->wah->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i] +64;// 1 pan offset
    if(plug->wah->getpar(i) != val)
    {
        plug->wah->changepar(i,val);
    }
    for(i++; i<5; i++) //2-4
    {
        val = (int)*plug->param_p[i];
        if(plug->wah->getpar(i) != val)
        {
            plug->wah->changepar(i,val);
        }
    }
    val = (int)*plug->param_p[i] +64;// 5 LR Del. offset
    if(plug->wah->getpar(i) != val)
    {
        plug->wah->changepar(i,val);
    }
    for(i++; i<plug->nparams; i++) // 6-10
    {
        val = (int)*plug->param_p[i];
        if(plug->wah->getpar(i) != val)
        {
            plug->wah->changepar(i,val);
        }
    }

    inplace_check(plug,nframes);

    //now set out ports and global period size
    plug->wah->efxoutl = plug->output_l_p;
    plug->wah->efxoutr = plug->output_r_p;

    //now run
    plug->wah->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->wah->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->wah->cleanup();
    }
    return;
}

///// derelict /////////
LV2_Handle init_derelv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 12;
    plug->effectindex = IDERE;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->dere = new NewDist(0,0, sample_freq, plug->period_max, /*oversampling*/2,
                             /*up interpolation method*/4, /*down interpolation method*/2);

    return plug;
}

void run_derelv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    i=0;
    val = (int)*plug->param_p[i];//0 Wet/dry
    if(plug->dere->getpar(i) != val)
    {
        plug->dere->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i]+64;//1 pan
    if(plug->dere->getpar(i) != val)
    {
        plug->dere->changepar(i,val);
    }
    for(i++; i<plug->nparams; i++) //2-11
    {
        val = (int)*plug->param_p[i];
        if(plug->dere->getpar(i) != val)
        {
            plug->dere->changepar(i,val);
        }
    }

    inplace_check(plug,nframes);

    //now set out ports and global period size
    plug->dere->efxoutl = plug->output_l_p;
    plug->dere->efxoutr = plug->output_r_p;

    //now run
    plug->dere->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->dere->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->dere->cleanup();
    }
    return;
}

///// valve /////////
LV2_Handle init_valvelv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 13;
    plug->effectindex = IVALVE;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->valve = new Valve(0,0, sample_freq, plug->period_max);

    return plug;
}

void run_valvelv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    i=0;
    val = (int)*plug->param_p[i];//0 Wet/dry
    if(plug->valve->getpar(i) != val)
    {
        plug->valve->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i]+64;//1 pan
    if(plug->valve->getpar(i) != val)
    {
        plug->valve->changepar(i,val);
    }
    for(i++; i<plug->nparams; i++) //2-12
    {
        val = (int)*plug->param_p[i];
        if(plug->valve->getpar(i) != val)
        {
            plug->valve->changepar(i,val);
        }
    }

    inplace_check(plug,nframes);

    //now set out ports and global period size
    plug->valve->efxoutl = plug->output_l_p;
    plug->valve->efxoutr = plug->output_r_p;

    //now run
    plug->valve->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->valve->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->valve->cleanup();
    }
    return;
}

///// dual flange /////////
LV2_Handle init_dflangelv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 15;
    plug->effectindex = IDFLANGE;
    plug->prev_bypass = 1;

    plug->dflange = new Dflange(0,0, sample_freq);

    return plug;
}

void run_dflangelv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //lfo effects must set period before params
    plug->dflange->PERIOD = nframes;

    //check and set changed parameters
    i=0;
    val = (int)*plug->param_p[i]-64;//0 Wet/dry
    if(plug->dflange->getpar(i) != val)
    {
        plug->dflange->changepar(i,val);
    }
    for(i++; i<plug->nparams; i++) //1-14
    {
        val = (int)*plug->param_p[i];
        if(plug->dflange->getpar(i) != val)
        {
            plug->dflange->changepar(i,val);
        }
    }

    //now set out ports and global period size
    plug->dflange->efxoutl = plug->output_l_p;
    plug->dflange->efxoutr = plug->output_r_p;

    //dflange does process in-place?
    prepare_inplace (plug, nframes);

    //now set out ports
    plug->dflange->efxoutl = plug->output_l_p;
    plug->dflange->efxoutr = plug->output_r_p;

    //now run
    plug->dflange->out(plug->output_l_p,plug->output_r_p,nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->dflange->cleanup();
    }
    return;
}

//////// ring /////////
LV2_Handle init_ringlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 13;
    plug->effectindex = IRING;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    //magic numbers: shift qual 4, downsample 5, up qual 4, down qual 2,
    plug->ring = new Ring(0,0, sample_freq);
    plug->noteID = new Recognize(0,0,.6, sample_freq, 440.0, plug->period_max);//.6 is default trigger value

    return plug;
}

void run_ringlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    i = 0;
    val = (int)*plug->param_p[i] - 64;// 0 wet/dry
    if(plug->ring->getpar(i) != val)
    {
        plug->ring->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i];// 1 pan
    if(plug->ring->getpar(i) != val)
    {
        plug->ring->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i] - 64;// 2 L/R cross
    if(plug->ring->getpar(i) != val)
    {
        plug->ring->changepar(i,val);
    }
    for(i++; i<plug->nparams; i++) //3-12
    {
        val = (int)*plug->param_p[i];
        if(plug->ring->getpar(i) != val)
        {
            plug->ring->changepar(i,val);
        }
    }

    inplace_check(plug,nframes);

//see process.C ln 1539

    //TODO may need to make sure input is over some threshold
    if(plug->ring->Pafreq)
    {
        //copy over the data so that noteID doesn't tamper with it
        bypass_stereo (plug,nframes);
        plug->noteID->schmittFloat(plug->output_l_p,plug->output_r_p,nframes);
        if(plug->noteID->reconota != -1 && plug->noteID->reconota != plug->noteID->last)
        {
            if(plug->noteID->afreq > 0.0)
            {
                plug->ring->Pfreq = lrintf(plug->noteID->lafreq);//round
                plug->noteID->last = plug->noteID->reconota;
            }
        }
    }

    //now set out ports and global period size
    plug->ring->efxoutl = plug->output_l_p;
    plug->ring->efxoutr = plug->output_r_p;

    //now run
    plug->ring->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->ring->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->ring->cleanup();
    }
    return;
}


///// mbdist /////////
LV2_Handle init_mbdistlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 15;
    plug->effectindex = IMBDIST;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->mbdist = new MBDist(0,0, sample_freq, plug->period_max, /*oversampling*/2,
                              /*up interpolation method*/4, /*down interpolation method*/2);

    return plug;
}

void run_mbdistlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    i=0;
    val = (int)*plug->param_p[i];//0 Wet/dry
    if(plug->mbdist->getpar(i) != val)
    {
        plug->mbdist->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i]+64;//1 pan
    if(plug->mbdist->getpar(i) != val)
    {
        plug->mbdist->changepar(i,val);
    }
    for(i++; i<plug->nparams; i++) //2-12
    {
        val = (int)*plug->param_p[i];
        if(plug->mbdist->getpar(i) != val)
        {
            plug->mbdist->changepar(i,val);
        }
    }

    inplace_check(plug,nframes);

    //now set out ports and global period size
    plug->mbdist->efxoutl = plug->output_l_p;
    plug->mbdist->efxoutr = plug->output_r_p;

    //now run
    plug->mbdist->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->mbdist->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->mbdist->cleanup();
    }
    return;
}

///// arp /////////
LV2_Handle init_arplv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 11;
    plug->effectindex = IARPIE;
    plug->prev_bypass = 1;

    plug->arp = new Arpie(0,0,sample_freq);

    return plug;
}

void run_arplv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    i=0;
    val = (int)*plug->param_p[i];//w/d
    if(plug->arp->getpar(i) != val)
    {
        plug->arp->changepar(i,val);
    }
    i++;//panning is offset
    val = (int)*plug->param_p[i]+64;
    if(plug->arp->getpar(i) != val)
    {
        plug->arp->changepar(i,val);
    }
    i++;//delay is not offset
    val = (int)*plug->param_p[i];
    if(plug->arp->getpar(i) != val)
    {
        plug->arp->changepar(i,val);
    }
    i++;//LR delay is offset
    val = (int)*plug->param_p[i]+64;
    if(plug->arp->getpar(i) != val)
    {
        plug->arp->changepar(i,val);
    }
    for(i++; i<plug->nparams; i++) //rest are not offset
    {
        val = (int)*plug->param_p[i];
        if(plug->arp->getpar(i) != val)
        {
            plug->arp->changepar(i,val);
        }
    }

    inplace_check(plug,nframes);

    //now set out ports and global period size
    plug->arp->efxoutl = plug->output_l_p;
    plug->arp->efxoutr = plug->output_r_p;

    //now run
    plug->arp->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->arp->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->arp->cleanup();
    }
    return;
}

///// expand /////////
LV2_Handle init_expandlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 7;
    plug->effectindex = IEXPAND;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->expand = new Expander(0,0, sample_freq, plug->period_max);

    return plug;
}

void run_expandlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    for(i=0; i<plug->nparams; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->expand->getpar(i+1) != val)//this effect is 1 indexed
        {
            plug->expand->Expander_Change(i+1,val);
        }
    }

    //comp does process in-place
    prepare_inplace (plug, nframes);

    //now set out ports
    plug->expand->efxoutl = plug->output_l_p;
    plug->expand->efxoutr = plug->output_r_p;

    //now run
    plug->expand->out(plug->output_l_p,plug->output_r_p,nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->expand->cleanup();
    }
    return;
}

///// shuffle /////////
LV2_Handle init_shuflv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 11;
    plug->effectindex = ISHUFF;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->shuf = new Shuffle(0,0,sample_freq,plug->period_max);

    return plug;
}

void run_shuflv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    for(i=0; i<plug->nparams; i++) //rest are not offset
    {
        val = (int)*plug->param_p[i];
        if(plug->shuf->getpar(i) != val)
        {
            plug->shuf->changepar(i,val);
        }
    }

    inplace_check(plug,nframes);

    //now set out ports and global period size
    plug->shuf->efxoutl = plug->output_l_p;
    plug->shuf->efxoutr = plug->output_r_p;

    //now run
    plug->shuf->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->shuf->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->shuf->cleanup();
    }
    return;
}


///// synth /////////
LV2_Handle init_synthlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 16;
    plug->effectindex = ISYNTH;
    plug->prev_bypass = 1;

    plug->synth = new Synthfilter(0,0,sample_freq);

    return plug;
}

void run_synthlv2(LV2_Handle handle, uint32_t nframes)
{
    unsigned int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //LFO effects require period be set before setting other params
    plug->synth->PERIOD = nframes;

    //check and set changed parameters
    for(i=0; i<5; i++) //0-4
    {
        val = (int)*plug->param_p[i];
        if(plug->synth->getpar(i) != val)
        {
            plug->synth->changepar(i,val);
        }
    }
    val = (int)*plug->param_p[i] +64;// 5 LR Del. offset
    if(plug->synth->getpar(i) != val)
    {
        plug->synth->changepar(i,val);
    }
    for(i++; i<plug->nparams; i++) //6-10
    {
        val = (int)*plug->param_p[i];
        if(plug->synth->getpar(i) != val)
        {
            plug->synth->changepar(i,val);
        }
    }

    inplace_check(plug,nframes);

    //now set out ports and global period size
    plug->synth->efxoutl = plug->output_l_p;
    plug->synth->efxoutr = plug->output_r_p;

    //now run
    plug->synth->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->synth->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->synth->cleanup();
    }
    return;
}

///// mbvol /////////
LV2_Handle init_mbvollv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 14;
    plug->effectindex = IMBVOL;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->mbvol = new MBVvol(0,0,sample_freq,plug->period_max);

    return plug;
}

void run_mbvollv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    plug->mbvol->PERIOD = nframes;

    //check and set changed parameters
    for(i=0; i<3; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->mbvol->getpar(i) != val)
        {
            plug->mbvol->changepar(i,val);
        }
    }
    val = (int)*plug->param_p[i] +64;//3 LR delay
    if(plug->mbvol->getpar(i) != val)
    {
        plug->mbvol->changepar(i,val);
    }
    for(i++; i<6; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->mbvol->getpar(i) != val)
        {
            plug->mbvol->changepar(i,val);
        }
    }
    val = (int)*plug->param_p[i] +64;//6 LR delay
    if(plug->mbvol->getpar(i) != val)
    {
        plug->mbvol->changepar(i,val);
    }
    for(i++; i<plug->nparams; i++) //skip legacy combi setting
    {
        val = (int)*plug->param_p[i];
        if(plug->mbvol->getpar(i+1) != val)
        {
            plug->mbvol->changepar(i+1,val);
        }
    }

    inplace_check(plug,nframes);

    //now set out ports and global period size
    plug->mbvol->efxoutl = plug->output_l_p;
    plug->mbvol->efxoutr = plug->output_r_p;

    //now run
    plug->mbvol->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->mbvol->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->mbvol->cleanup();
    }
    return;
}

///// mutro /////////
LV2_Handle init_mutrolv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 19;
    plug->effectindex = IMUTRO;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->mutro = new RyanWah(0,0,sample_freq, plug->period_max);

    return plug;
}

void run_mutrolv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    plug->mutro->PERIOD = nframes;

    //check and set changed parameters
    for(i=0; i<5; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->mutro->getpar(i) != val)
        {
            plug->mutro->changepar(i,val);
        }
    }
    val = (int)*plug->param_p[i] +64;//5 LR delay
    if(plug->mutro->getpar(i) != val)
    {
        plug->mutro->changepar(i,val);
    }
    for(i++; i<17; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->mutro->getpar(i) != val)
        {
            plug->mutro->changepar(i,val);
        }
    }
    for(; i<plug->nparams; i++) //skip legacy mode and preset setting
    {
        val = (int)*plug->param_p[i];
        if(plug->mutro->getpar(i+2) != val)
        {
            plug->mutro->changepar(i+2,val);
        }
    }

    inplace_check(plug,nframes);

    //now set out ports and global period size
    plug->mutro->efxoutl = plug->output_l_p;
    plug->mutro->efxoutr = plug->output_r_p;

    //now run
    plug->mutro->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->mutro->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->mutro->cleanup();
    }
    return;
}

///// echoverse /////////
LV2_Handle init_echoverselv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 10;
    plug->effectindex = IECHOVERSE;
    plug->prev_bypass = 1;

    plug->echoverse = new RBEcho(0,0,sample_freq);

    return plug;
}

void run_echoverselv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    i=0;
    val = (int)*plug->param_p[i];//wet/dry
    if(plug->echoverse->getpar(i) != val)
    {
        plug->echoverse->changepar(i,val);
    }
    i++;//1 panning is offset
    val = (int)*plug->param_p[i]+64;
    if(plug->echoverse->getpar(i) != val)
    {
        plug->echoverse->changepar(i,val);
    }
    i++;//2 delay is not offset
    val = (int)*plug->param_p[i];
    if(plug->echoverse->getpar(i) != val)
    {
        plug->echoverse->changepar(i,val);
    }
    for(i++; i<5; i++) //3,4 LR delay and angle is offset
    {
        val = (int)*plug->param_p[i]+64;
        if(plug->echoverse->getpar(i) != val)
        {
            plug->echoverse->changepar(i,val);
        }
    }
    for( ; i<plug->nparams; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->echoverse->getpar(i) != val)
        {
            plug->echoverse->changepar(i,val);
        }
    }

    inplace_check(plug,nframes);

    //now set out ports and global period size
    plug->echoverse->efxoutl = plug->output_l_p;
    plug->echoverse->efxoutr = plug->output_r_p;

    //now run
    plug->echoverse->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->echoverse->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->echoverse->cleanup();
    }
    return;
}

///// coilcrafter /////////
LV2_Handle init_coillv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 7;
    plug->effectindex = ICOIL;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->coil = new CoilCrafter(0,0,sample_freq,plug->period_max);

    return plug;
}

void run_coillv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    i=0;
    val = (int)*plug->param_p[i];//wet/dry
    if(plug->coil->getpar(i) != val)
    {
        plug->coil->changepar(i,val);
    }
    for(i++; i<plug->nparams; i++) //skip origin and destinations
    {
        val = (int)*plug->param_p[i];
        if(plug->coil->getpar(i+2) != val)
        {
            plug->coil->changepar(i+2,val);
        }
    }
    //coilcrafter does process in-place
    prepare_inplace (plug, nframes);

    //now set out ports
    plug->coil->efxoutl = plug->output_l_p;
    plug->coil->efxoutr = plug->output_r_p;

    //now run
    plug->coil->out(plug->output_l_p,plug->output_r_p,nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->coil->cleanup();
    }
    return;
}

///// shelfboost /////////
LV2_Handle init_shelflv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 5;
    plug->effectindex = ISHELF;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->shelf = new ShelfBoost(0,0,sample_freq, plug->period_max);

    return plug;
}

void run_shelflv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    for(i=0; i<plug->nparams; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->shelf->getpar(i) != val)
        {
            plug->shelf->changepar(i,val);
        }
    }
    //coilcrafter does process in-place
    prepare_inplace (plug, nframes);

    //now set out ports
    plug->shelf->efxoutl = plug->output_l_p;
    plug->shelf->efxoutr = plug->output_r_p;

    //now run
    plug->shelf->out(plug->output_l_p,plug->output_r_p,nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->shelf->cleanup();
    }
    return;
}

///// Vocoder /////////
LV2_Handle init_voclv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 7;
    plug->effectindex = IVOC;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->voc = new Vocoder(0,0,0,/*bands*/32,/*downsamplex2*/5,/*upsample quality*/4,
                            /*downsample quality*/ 2,sample_freq,plug->period_max);

    return plug;
}

void run_voclv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    i=0;
    val = (int)*plug->param_p[i];//wet/dry
    if(plug->voc->getpar(i) != val)
    {
        plug->voc->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i]+64;//pan
    if(plug->voc->getpar(i) != val)
    {
        plug->voc->changepar(i,val);
    }
    for(i++; i<plug->nparams; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->voc->getpar(i) != val)
        {
            plug->voc->changepar(i,val);
        }
    }

    inplace_check(plug,nframes);

    //set aux input and out ports
    plug->voc->auxresampled = plug->param_p[VOCODER_AUX_IN];
    plug->voc->efxoutl = plug->output_l_p;
    plug->voc->efxoutr = plug->output_r_p;

    //now run
    plug->voc->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->voc->outvolume, nframes);

    //and set VU meter
    *plug->param_p[VOCODER_VU_LEVEL] = plug->voc->vulevel;

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->voc->cleanup();
    }
    return;
}

///// Sustainer /////////
LV2_Handle init_suslv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 2;
    plug->effectindex = ISUS;
    plug->prev_bypass = 1;


    plug->sus = new Sustainer(0,0,sample_freq);

    return plug;
}

void run_suslv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    for(i=0; i<plug->nparams; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->sus->getpar(i) != val)
        {
            plug->sus->changepar(i,val);
        }
    }

    //sustainer does process in-place
    prepare_inplace (plug, nframes);

    //now set out ports
    plug->sus->efxoutl = plug->output_l_p;
    plug->sus->efxoutr = plug->output_r_p;

    //now run
    plug->sus->out(plug->output_l_p,plug->output_r_p,nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->sus->cleanup();
    }
    return;
}

///// Sequence /////////
LV2_Handle init_seqlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 15;
    plug->effectindex = ISEQ;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->seq = new Sequence(0,0,/*shifter quality*/4,/*downsamplex2*/5,/*upsample quality*/4,
                             /*downsample quality*/ 2,sample_freq,plug->period_max);

    return plug;
}

void run_seqlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    for(i=0; i<10; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->seq->getpar(i) != val)
        {
            plug->seq->changepar(i,val);
        }
    }
    val = (int)*plug->param_p[i]+64;//Q or panning
    if(plug->seq->getpar(i) != val)
    {
        plug->seq->changepar(i,val);
    }
    for(i++; i<plug->nparams; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->seq->getpar(i) != val)
        {
            plug->seq->changepar(i,val);
        }
    }

    inplace_check(plug,nframes);

    //set out ports
    plug->seq->efxoutl = plug->output_l_p;
    plug->seq->efxoutr = plug->output_r_p;

    //now run
    plug->seq->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->seq->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->seq->cleanup();
    }
    return;
}

///// Shifter /////////
LV2_Handle init_shiftlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 10;
    plug->effectindex = ISHIFT;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->shift = new Shifter(0,0,/*shifter quality*/4,/*downsamplex2*/5,/*upsample quality*/4,
                              /*downsample quality*/ 2,sample_freq,plug->period_max);

    return plug;
}

void run_shiftlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    i=0;
    val = (int)*plug->param_p[i];//wet/dry
    if(plug->shift->getpar(i) != val)
    {
        plug->shift->changepar(i,val);
    }
    for(i++; i<3; i++) //pan, gain
    {
        val = (int)*plug->param_p[i]+64;
        if(plug->shift->getpar(i) != val)
        {
            plug->shift->changepar(i,val);
        }
    }
    for(; i<plug->nparams; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->shift->getpar(i) != val)
        {
            plug->shift->changepar(i,val);
        }
    }

    inplace_check(plug,nframes);

    //set out ports
    plug->shift->efxoutl = plug->output_l_p;
    plug->shift->efxoutr = plug->output_r_p;

    //now run
    plug->shift->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->shift->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->shift->cleanup();
    }
    return;
}

///// StompBox /////////
LV2_Handle init_stomplv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 6;
    plug->effectindex = ISTOMP;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->stomp = new StompBox(0,0, sample_freq, plug->period_max, /*oversampling*/2,
                               /*up interpolation method*/SRC_LINEAR, /*down interpolation method*/SRC_SINC_FASTEST);

    return plug;
}

void run_stomplv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    for(i=0; i<plug->nparams; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->stomp->getpar(i) != val)
        {
            plug->stomp->changepar(i,val);
        }
    }

    //stompbox does process in-place
    prepare_inplace (plug, nframes);

    //now set out ports
    plug->stomp->efxoutl = plug->output_l_p;
    plug->stomp->efxoutr = plug->output_r_p;

    //now run
    plug->stomp->out(plug->output_l_p,plug->output_r_p,nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->stomp->cleanup();
    }
    return;
}
///// StompBox Fuzz /////////
LV2_Handle init_stomp_fuzzlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    //this is the same but has better labeling as controls act differently
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 5;
    plug->effectindex = IFUZZ;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->stomp = new StompBox(0,0, sample_freq, plug->period_max, /*oversampling*/1,
                               /*up interpolation method*/SRC_LINEAR, /*down interpolation method*/SRC_LINEAR);
    plug->stomp->changepar(5,7);//set to fuzz

    return plug;
}

///// Reverbtron /////////
LV2_Handle init_revtronlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 14;
    plug->effectindex = IREVTRON;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);
    if(!plug->scheduler || !plug->urid_map)
    {
    	//a required feature was not supported by host
    	free(plug);
    	return 0;
    }
    lv2_atom_forge_init(&plug->forge, plug->urid_map);

    plug->revtron = new Reverbtron(0,0, sample_freq, plug->period_max, /*downsample*/5, /*up interpolation method*/SRC_LINEAR, /*down interpolation method*/SRC_SINC_FASTEST);
    plug->revtron->changepar(4,1);//set to user selected files
    plug->rvbfile = new RvbFile;

    return plug;
}

void run_revtronlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    for(i=0; i<4; i++)//skip user
    {
        val = (int)*plug->param_p[i];
        if(plug->revtron->getpar(i) != val)
        {
            plug->revtron->changepar(i,val);
        }
    }
    for(; i+1<8; i++)//skip file num
    {
        val = (int)*plug->param_p[i];
        if(plug->revtron->getpar(i+1) != val)
        {
            plug->revtron->changepar(i+1,val);
        }
    }
    for(; i+2<11; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->revtron->getpar(i+2) != val)
        {
            plug->revtron->changepar(i+2,val);
        }
    }
    val = (int)*plug->param_p[i]+64;//11 panning
    if(plug->revtron->getpar(i+2) != val)
    {
        plug->revtron->changepar(i+2,val);
    }
    for(i++; i<plug->nparams; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->revtron->getpar(i+2) != val)
        {
            plug->revtron->changepar(i+2,val);
        }
    }

    // Set up forge to write directly to notify output port.
    const uint32_t notify_capacity = plug->atom_out_p->atom.size;
    lv2_atom_forge_set_buffer(&plug->forge, (uint8_t*)plug->atom_out_p, notify_capacity);

    // Start a sequence in the notify output port.
    lv2_atom_forge_sequence_head(&plug->forge, &plug->atom_frame, 0);

    //if we loaded a state, send the new file name to the host to display
    if(plug->file_changed)
    {
    	plug->file_changed = 0;
    	lv2_atom_forge_frame_time(&plug->forge, 0);
        LV2_Atom_Forge_Frame frame;
        lv2_atom_forge_object( &plug->forge, &frame, 0, plug->URIDs.patch_Set);

        lv2_atom_forge_key(&plug->forge, plug->URIDs.patch_property);
        lv2_atom_forge_urid(&plug->forge, plug->URIDs.filetype_rvb);
        lv2_atom_forge_key(&plug->forge, plug->URIDs.patch_value);
        lv2_atom_forge_path(&plug->forge, plug->revtron->File.Filename, strlen(plug->revtron->File.Filename)+1);

        lv2_atom_forge_pop(&plug->forge, &frame);
    }


    //see if there's a file
    LV2_ATOM_SEQUENCE_FOREACH( plug->atom_in_p, ev)
    {
        if (ev->body.type == plug->URIDs.atom_Object)
        {
            const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev->body;
            if (obj->body.otype == plug->URIDs.patch_Set)
            {
                // Get the property the set message is setting
                const LV2_Atom* property = NULL;
                lv2_atom_object_get(obj, plug->URIDs.patch_property, &property, 0);
                if (property && property->type == plug->URIDs.atom_URID)
                {
                    const uint32_t key = ((const LV2_Atom_URID*)property)->body;
                    if (key == plug->URIDs.filetype_rvb)
                    {
                        // a new file! pass the atom to the worker thread to load it
                        plug->scheduler->schedule_work(plug->scheduler->handle, lv2_atom_total_size(&ev->body), &ev->body);
                    }//property is rvb file
                }//property is URID
            }
            else if (obj->body.otype == plug->URIDs.patch_Get)
            {
                // Received a get message, emit our state (probably to UI)
                lv2_atom_forge_frame_time(&plug->forge, ev->time.frames );//use current event's time
                LV2_Atom_Forge_Frame frame;
                lv2_atom_forge_object( &plug->forge, &frame, 0, plug->URIDs.patch_Set);

            	lv2_atom_forge_key(&plug->forge, plug->URIDs.patch_property);
            	lv2_atom_forge_urid(&plug->forge, plug->URIDs.filetype_rvb);
            	lv2_atom_forge_key(&plug->forge, plug->URIDs.patch_value);
            	lv2_atom_forge_path(&plug->forge, plug->revtron->File.Filename, strlen(plug->revtron->File.Filename)+1);

            	lv2_atom_forge_pop(&plug->forge, &frame);
            }
        }//atom is object
    }//each atom in sequence

    inplace_check(plug,nframes);

    //now set out ports
    plug->revtron->efxoutl = plug->output_l_p;
    plug->revtron->efxoutr = plug->output_r_p;

    //now run
    plug->revtron->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->revtron->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->revtron->cleanup();
    }
    return;
}

static LV2_Worker_Status revwork(LV2_Handle handle, LV2_Worker_Respond_Function respond, LV2_Worker_Respond_Handle rhandle, uint32_t size, const void* data)
{

    RKRLV2* plug = (RKRLV2*)handle;
    LV2_Atom_Object* obj = (LV2_Atom_Object*)data;
    const LV2_Atom* file_path;

    //work was scheduled to load a new file
    lv2_atom_object_get(obj, plug->URIDs.patch_value, &file_path, 0);
    if (file_path && file_path->type == plug->URIDs.atom_Path)
    {
        // Load file.
        char* path = (char*)LV2_ATOM_BODY_CONST(file_path);
        //the file is too large for a host's circular buffer
        //so store it in the plugin for the response to use
        //to prevent threading issues, we'll use a simple
        //flag as a crude mutex
        while(plug->loading_file)
        	usleep(1000);
        plug->loading_file = 1;
        *plug->rvbfile = plug->revtron->loadfile(path);
        respond(rhandle,0,0);
    }//got file
    else
        return LV2_WORKER_ERR_UNKNOWN;

    return LV2_WORKER_SUCCESS;
}

static LV2_Worker_Status revwork_response(LV2_Handle handle, uint32_t size, const void* data)
{
    RKRLV2* plug = (RKRLV2*)handle;
    plug->revtron->applyfile(*plug->rvbfile);
    plug->loading_file = 0;//clear flag for next file load
    return LV2_WORKER_SUCCESS;
}

static LV2_State_Status revsave(LV2_Handle handle, LV2_State_Store_Function  store, LV2_State_Handle state_handle,
		uint32_t flags, const LV2_Feature* const* features)
{
    RKRLV2* plug = (RKRLV2*)handle;

    LV2_State_Map_Path* map_path = NULL;
    for (int i = 0; features[i]; ++i) 
    {
        if (!strcmp(features[i]->URI, LV2_STATE__mapPath)) 
        {
            map_path = (LV2_State_Map_Path*)features[i]->data;
        }
    }

    char* abstractpath = map_path->abstract_path(map_path->handle, plug->revtron->File.Filename);

    store(state_handle, plug->URIDs.filetype_rvb, abstractpath, strlen(plug->revtron->File.Filename) + 1,
    		plug->URIDs.atom_Path, LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);

    free(abstractpath);

    return LV2_STATE_SUCCESS;
}

static LV2_State_Status revrestore(LV2_Handle handle, LV2_State_Retrieve_Function retrieve,
		LV2_State_Handle state_handle, uint32_t flags, const LV2_Feature* const* features)
{
    RKRLV2* plug = (RKRLV2*)handle;

    size_t   size;
    uint32_t type;
    uint32_t valflags;

    const void* value = retrieve( state_handle, plug->URIDs.filetype_rvb, &size, &type, &valflags);

    if (value) 
    {
            char* path = (char*)value;
            RvbFile f = plug->revtron->loadfile(path);
            plug->revtron->applyfile(f);
            plug->file_changed = 1;
    }

    return LV2_STATE_SUCCESS;
}

static const void* revtron_extension_data(const char* uri)
{
    static const LV2_Worker_Interface worker = { revwork, revwork_response, NULL };
    static const LV2_State_Interface state_iface = { revsave, revrestore };
    if (!strcmp(uri, LV2_STATE__interface))
    {
        return &state_iface;
    }
    else if (!strcmp(uri, LV2_WORKER__interface))
    {
        return &worker;
    }
    return NULL;
}

///// Echotron /////////
LV2_Handle init_echotronlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 13;
    plug->effectindex = IECHOTRON;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);
    if(!plug->scheduler || !plug->urid_map)
    {
    	//a required feature was not supported by host
    	free(plug);
    	return 0;
    }
    lv2_atom_forge_init(&plug->forge, plug->urid_map);

    plug->echotron = new Echotron(0,0, sample_freq, plug->period_max);
    plug->echotron->changepar(4,1);//set to user selected files
    plug->dlyfile = new DlyFile;

    return plug;
}

void run_echotronlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    i=0;
    val = (int)*plug->param_p[i];//0 w/d
    if(plug->echotron->getpar(i) != val)
    {
        plug->echotron->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i]+64;//1 fliter depth
    if(plug->echotron->getpar(i) != val)
    {
        plug->echotron->changepar(i,val);
    }
    for(i++; i<4; i++)//skip user
    {
        val = (int)*plug->param_p[i];
        if(plug->echotron->getpar(i) != val)
        {
            plug->echotron->changepar(i,val);
        }
    }
    for(; i+1<7; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->echotron->getpar(i+1) != val)
        {
            plug->echotron->changepar(i+1,val);
        }
    }
    val = (int)*plug->param_p[i]+64;//7 l/R cross
    if(plug->echotron->getpar(i+1) != val)
    {
        plug->echotron->changepar(i+1,val);
    }
    for(i++; i+2<11; i++)//skip file num
    {
        val = (int)*plug->param_p[i];
        if(plug->echotron->getpar(i+2) != val)
        {
            plug->echotron->changepar(i+2,val);
        }
    }
    val = (int)*plug->param_p[i]+64;//11 panning
    if(plug->echotron->getpar(i+2) != val)
    {
        plug->echotron->changepar(i+2,val);
    }
    for(i++; i<plug->nparams; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->echotron->getpar(i+2) != val)
        {
            plug->echotron->changepar(i+2,val);
        }
    }

    // Set up forge to write directly to notify output port.
    const uint32_t notify_capacity = plug->atom_out_p->atom.size;
    lv2_atom_forge_set_buffer(&plug->forge, (uint8_t*)plug->atom_out_p, notify_capacity);

    // Start a sequence in the notify output port.
    lv2_atom_forge_sequence_head(&plug->forge, &plug->atom_frame, 0);

    //if we loaded a state, send the new file name to the host to display
    if(plug->file_changed)
    {
    	plug->file_changed = 0;
    	lv2_atom_forge_frame_time(&plug->forge, 0);
        LV2_Atom_Forge_Frame frame;
        lv2_atom_forge_object( &plug->forge, &frame, 0, plug->URIDs.patch_Set);

        lv2_atom_forge_key(&plug->forge, plug->URIDs.patch_property);
        lv2_atom_forge_urid(&plug->forge, plug->URIDs.filetype_dly);
        lv2_atom_forge_key(&plug->forge, plug->URIDs.patch_value);
        lv2_atom_forge_path(&plug->forge, plug->echotron->File.Filename, strlen(plug->echotron->File.Filename)+1);

        lv2_atom_forge_pop(&plug->forge, &frame);
    }


    //see if there's a file
    LV2_ATOM_SEQUENCE_FOREACH( plug->atom_in_p, ev)
    {
        if (ev->body.type == plug->URIDs.atom_Object)
        {
            const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev->body;
            if (obj->body.otype == plug->URIDs.patch_Set)
            {
                // Get the property the set message is setting
                const LV2_Atom* property = NULL;
                lv2_atom_object_get(obj, plug->URIDs.patch_property, &property, 0);
                if (property && property->type == plug->URIDs.atom_URID)
                {
                    const uint32_t key = ((const LV2_Atom_URID*)property)->body;
                    if (key == plug->URIDs.filetype_dly)
                    {
                        // a new file! pass the atom to the worker thread to load it
                        plug->scheduler->schedule_work(plug->scheduler->handle, lv2_atom_total_size(&ev->body), &ev->body);
                    }//property is dly file
                }//property is URID
            }
            else if (obj->body.otype == plug->URIDs.patch_Get)
            {
                // Received a get message, emit our state (probably to UI)
                lv2_atom_forge_frame_time(&plug->forge, ev->time.frames );//use current event's time
                LV2_Atom_Forge_Frame frame;
                lv2_atom_forge_object( &plug->forge, &frame, 0, plug->URIDs.patch_Set);

            	lv2_atom_forge_key(&plug->forge, plug->URIDs.patch_property);
            	lv2_atom_forge_urid(&plug->forge, plug->URIDs.filetype_dly);
            	lv2_atom_forge_key(&plug->forge, plug->URIDs.patch_value);
            	lv2_atom_forge_path(&plug->forge, plug->echotron->File.Filename, strlen(plug->echotron->File.Filename)+1);

            	lv2_atom_forge_pop(&plug->forge, &frame);
            }
        }//atom is object
    }//each atom in sequence

    inplace_check(plug,nframes);

    //now set out ports
    plug->echotron->efxoutl = plug->output_l_p;
    plug->echotron->efxoutr = plug->output_r_p;

    //now run
    plug->echotron->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->echotron->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->echotron->cleanup();
    }
    return;
}

static LV2_Worker_Status echowork(LV2_Handle handle, LV2_Worker_Respond_Function respond, LV2_Worker_Respond_Handle rhandle, uint32_t size, const void* data)
{

    RKRLV2* plug = (RKRLV2*)handle;
    LV2_Atom_Object* obj = (LV2_Atom_Object*)data;
    const LV2_Atom* file_path;

    //work was scheduled to load a new file
    lv2_atom_object_get(obj, plug->URIDs.patch_value, &file_path, 0);
    if (file_path && file_path->type == plug->URIDs.atom_Path)
    {
        // Load file.
        char* path = (char*)LV2_ATOM_BODY_CONST(file_path);
        //the file is too large for a host's circular buffer
        //so store it in the plugin for the response to use
        //to prevent threading issues, we'll use a simple
        //flag as a crude mutex
        while(plug->loading_file)
        	usleep(1000);
        plug->loading_file = 1;
        *plug->dlyfile = plug->echotron->loadfile(path);
        respond(rhandle,0,0);
    }//got file
    else
        return LV2_WORKER_ERR_UNKNOWN;

    return LV2_WORKER_SUCCESS;
}

static LV2_Worker_Status echowork_response(LV2_Handle handle, uint32_t size, const void* data)
{
    RKRLV2* plug = (RKRLV2*)handle;
    plug->echotron->applyfile(*plug->dlyfile);
    plug->loading_file = 0;//clear flag for next file load
    return LV2_WORKER_SUCCESS;
}

static LV2_State_Status echosave(LV2_Handle handle, LV2_State_Store_Function  store, LV2_State_Handle state_handle,
		uint32_t flags, const LV2_Feature* const* features)
{
    RKRLV2* plug = (RKRLV2*)handle;

    LV2_State_Map_Path* map_path = NULL;
    for (int i = 0; features[i]; ++i)
    {
        if (!strcmp(features[i]->URI, LV2_STATE__mapPath))
        {
            map_path = (LV2_State_Map_Path*)features[i]->data;
        }
    }

    char* abstractpath = map_path->abstract_path(map_path->handle, plug->echotron->File.Filename);

    store(state_handle, plug->URIDs.filetype_dly, abstractpath, strlen(plug->echotron->File.Filename) + 1,
    		plug->URIDs.atom_Path, LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);

    free(abstractpath);

    return LV2_STATE_SUCCESS;
}

static LV2_State_Status echorestore(LV2_Handle handle, LV2_State_Retrieve_Function retrieve,
		LV2_State_Handle state_handle, uint32_t flags, const LV2_Feature* const* features)
{
    RKRLV2* plug = (RKRLV2*)handle;

    size_t   size;
    uint32_t type;
    uint32_t valflags;

    const void* value = retrieve( state_handle, plug->URIDs.filetype_dly, &size, &type, &valflags);

    if (value)
    {
            char* path = (char*)value;
            DlyFile f = plug->echotron->loadfile(path);
            plug->echotron->applyfile(f);
            plug->file_changed = 1;
    }

    return LV2_STATE_SUCCESS;
}

static const void* echotron_extension_data(const char* uri)
{
    static const LV2_Worker_Interface worker = { echowork, echowork_response, NULL };
    static const LV2_State_Interface state_iface = { echosave, echorestore };
    if (!strcmp(uri, LV2_STATE__interface))
    {
        return &state_iface;
    }
    else if (!strcmp(uri, LV2_WORKER__interface))
    {
        return &worker;
    }
    return NULL;
}


//////// stereo harmonizer /////////
LV2_Handle init_sharmnomidlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 11;
    plug->effectindex = ISHARM_NM;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    //magic numbers: shift qual 4, downsample 5, up qual 4, down qual 2,
    plug->sharm = new StereoHarm(0,0,4,5,4,2, plug->period_max, sample_freq);
    plug->noteID = new Recognize(0,0,.6, sample_freq, 440.0, plug->period_max);//.6 is default trigger value
    plug->chordID = new RecChord();



    return plug;
}

void run_sharmnomidlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    i = 0;
    val = (int)*plug->param_p[i];// 0 wet/dry
    if(plug->sharm->getpar(i) != val)
    {
        plug->sharm->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i] + 64;// 1 gain l
    if(plug->sharm->getpar(i) != val)
    {
        plug->sharm->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i] + 12;// 2 interval l
    if(plug->sharm->getpar(i) != val)
    {
        plug->sharm->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i];// 3 chroma l
    if(plug->sharm->getpar(i) != val)
    {
        plug->sharm->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i] + 64;// 4 gain r
    if(plug->sharm->getpar(i) != val)
    {
        plug->sharm->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i] + 12;// 5 interval r
    if(plug->sharm->getpar(i) != val)
    {
        plug->sharm->changepar(i,val);
    }
    for(i++; i<10; i++) // 6-11
    {
        val = (int)*plug->param_p[i];
        if(plug->sharm->getpar(i) != val)
        {
            plug->sharm->changepar(i,val);
        }
    }
// skip midi mode, not implementing midi here
//    val = (int)*plug->param_p[i];// 10 midi mode
//    if(plug->aphase->getpar(i) != val)
//    {
//        plug->aphase->changepar(i,val);
//        if(!val) plug->harm->changepar(3,plug->harm->getpar(3));
//    }
    val = (int)*plug->param_p[i];// 11 lr cr.
    if(plug->sharm->getpar(i+1) != val)
    {
        plug->sharm->changepar(i+1,val);
    }
    /*
    see Chord() in rkr.fl
    harmonizer, need recChord and recNote.
    see process.C ln 1507
    */

    //TODO may need to make sure input is over some threshold
    if(plug->sharm->mira && plug->sharm->PSELECT)
    {
        plug->noteID->schmittFloat(plug->input_l_p,plug->input_r_p,nframes);
        if(plug->noteID->reconota != -1 && plug->noteID->reconota != plug->noteID->last)
        {
            if(plug->noteID->afreq > 0.0)
            {
                plug->chordID->Vamos(1,plug->sharm->Pintervall - 12,plug->noteID->reconota);
                plug->chordID->Vamos(2,plug->sharm->Pintervalr - 12,plug->noteID->reconota);
                plug->sharm->r_ratiol = plug->chordID->r__ratio[1];//pass the found ratio
                plug->sharm->r_ratior = plug->chordID->r__ratio[2];//pass the found ratio
            }
        }
    }

    inplace_check(plug,nframes);

    //now set out ports and global period size
    plug->sharm->efxoutl = plug->output_l_p;
    plug->sharm->efxoutr = plug->output_r_p;

    //now run
    plug->sharm->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->sharm->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->sharm->cleanup();
    }
    return;
}

///// MB comp /////////
LV2_Handle init_mbcomplv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 13;
    plug->effectindex = IMBCOMP;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->mbcomp = new CompBand(0,0, sample_freq, plug->period_max);

    return plug;
}

void run_mbcomplv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    for(i=0; i<plug->nparams; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->mbcomp->getpar(i) != val)
        {
            plug->mbcomp->changepar(i,val);
        }
    }

    inplace_check(plug,nframes);

    //now set out ports
    plug->mbcomp->efxoutl = plug->output_l_p;
    plug->mbcomp->efxoutr = plug->output_r_p;

    //now run
    plug->mbcomp->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->mbcomp->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->mbcomp->cleanup();
    }
    return;
}

///// OptTrem /////////
LV2_Handle init_otremlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 6;
    plug->effectindex = IOPTTREM;
    plug->prev_bypass = 1;

    plug->otrem = new Opticaltrem(0,0, sample_freq);

    return plug;
}

void run_otremlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //LFO effects require period be set before setting other params
    plug->otrem->PERIOD = nframes;

    //check and set changed parameters
    for(i=0; i<4; i++)//0-4
    {
        val = (int)*plug->param_p[i];
        if(plug->otrem->getpar(i) != val)
        {
            plug->otrem->changepar(i,val);
        }
    }
    for(; i<6; i++)//4-5 pan and st del
    {
        val = (int)*plug->param_p[i]+64;
        if(plug->otrem->getpar(i) != val)
        {
            plug->otrem->changepar(i,val);
        }
    }
    val = (int)*plug->param_p[i];//6 invert
    if(plug->otrem->getpar(i) != val)
    {
        plug->otrem->changepar(i,val);
    }

    //optotrem does process in-place
    prepare_inplace (plug, nframes);

    //now set out ports
    plug->otrem->efxoutl = plug->output_l_p;
    plug->otrem->efxoutr = plug->output_r_p;

    //now run
    plug->otrem->out(plug->output_l_p,plug->output_r_p,nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->otrem->cleanup();
    }
    return;
}

///// Vibe /////////
LV2_Handle init_vibelv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 11;
    plug->effectindex = IVIBE;
    plug->prev_bypass = 1;

    plug->vibe = new Vibe(0,0, sample_freq);

    return plug;
}

void run_vibelv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //LFO effects require period be set before setting other params
    plug->vibe->PERIOD = nframes;

    //check and set changed parameters
    for(i=0; i<4; i++)//0-4
    {
        val = (int)*plug->param_p[i];
        if(plug->vibe->getpar(i) != val)
        {
            plug->vibe->changepar(i,val);
        }
    }
    for(; i<6; i++)//4-5 pan and st del
    {
        val = (int)*plug->param_p[i]+64;
        if(plug->vibe->getpar(i) != val)
        {
            plug->vibe->changepar(i,val);
        }
    }
    val = (int)*plug->param_p[i];//6 wet/dry
    if(plug->vibe->getpar(i) != val)
    {
        plug->vibe->changepar(i,val);
    }
    i++;
    val = (int)*plug->param_p[i]+64;//7 FB
    if(plug->vibe->getpar(i) != val)
    {
        plug->vibe->changepar(i,val);
    }
    for(i++; i<plug->nparams; i++)//8-11 the rest
    {
        val = (int)*plug->param_p[i];
        if(plug->vibe->getpar(i) != val)
        {
            plug->vibe->changepar(i,val);
        }
    }

    inplace_check(plug,nframes);

    //now set out ports
    plug->vibe->efxoutl = plug->output_l_p;
    plug->vibe->efxoutr = plug->output_r_p;

    //now run
    plug->vibe->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->vibe->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->vibe->cleanup();
    }
    return;
}

///// Infinity /////////
LV2_Handle init_inflv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 18;
    plug->effectindex = IINF;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->inf = new Infinity(0,0, sample_freq, plug->period_max);

    return plug;
}

void run_inflv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    for(i=0; i<plug->nparams; i++)//0-17
    {
        val = (int)*plug->param_p[i];
        if(plug->inf->getpar(i) != val)
        {
            plug->inf->changepar(i,val);
        }
    }

    inplace_check(plug,nframes);

    //now set out ports
    plug->inf->efxoutl = plug->output_l_p;
    plug->inf->efxoutr = plug->output_r_p;

    //now run
    plug->inf->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->inf->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->inf->cleanup();
    }
    return;
}

///// Phaser /////////
LV2_Handle init_phaselv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 12;
    plug->effectindex = IPHASE;
    plug->prev_bypass = 1;

    plug->phase = new Phaser(0,0,sample_freq);
    plug->init_params = 1; // LFO init

    return plug;
}

void run_phaselv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //LFO effects require period be set before setting other params
    if(plug->init_params)
    {
        plug->phase->PERIOD = nframes;
        plug->phase->lfo->updateparams(nframes);
        plug->init_params = 0; // so we only do this once
    }

    //check and set changed parameters
    for(i=0; i<1; i++) //0-4
    {
        val = (int)*plug->param_p[i];
        if(plug->phase->getpar(i) != val)
        {
            plug->phase->changepar(i,val);
        }
    }
    val = (int)*plug->param_p[i] +64;// 1 Pan offset
    if(plug->phase->getpar(i) != val)
    {
        plug->phase->changepar(i,val);
    }
    for(i++; i<9; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->phase->getpar(i) != val)
        {
            plug->phase->changepar(i,val);
        }
    }
    val = (int)*plug->param_p[i] +64 ;// 9 l/r cross offset
    if(plug->phase->getpar(i) != val)
    {
        plug->phase->changepar(i,val);
    }
    for(i++; i<plug->nparams; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->phase->getpar(i) != val)
        {
            plug->phase->changepar(i,val);
        }
    }

    inplace_check(plug,nframes);

    //now set out ports and global period size
    plug->phase->efxoutl = plug->output_l_p;
    plug->phase->efxoutr = plug->output_r_p;

    //now run
    plug->phase->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug, plug->phase->outvolume, nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->phase->cleanup();
    }
    return;
}

///// NoiseGate /////////
LV2_Handle init_gatelv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 7;
    plug->effectindex = IGATE;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->gate = new Gate(0,0, sample_freq, plug->period_max);

    return plug;
}

void run_gatelv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        bypass_stereo (plug, nframes);
        return;
    }

    //check and set changed parameters
    for(i=0; i<plug->nparams; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->gate->getpar(i+1) != val)//this effect is 1 indexed
        {
            plug->gate->Gate_Change(i+1,val);
        }
    }

    //gate does process in-place
    prepare_inplace (plug, nframes);

    //now set out ports
    plug->gate->efxoutl = plug->output_l_p;
    plug->gate->efxoutr = plug->output_r_p;

    //now run
    plug->gate->out(plug->output_l_p,plug->output_r_p,nframes);

    xfade_check(plug,nframes);
    if(plug->prev_bypass)
    {
        plug->gate->cleanup();
    }
    return;
}

/////////////////////////////////
//       END OF FX
/////////////////////////////////



void cleanup_rkrlv2(LV2_Handle handle)
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
    case IPHASE:
        delete plug->phase;
        break;
    case IGATE:
        delete plug->gate;
        break;
    }
    free(plug);
}

void connect_rkrlv2_ports_w_atom(LV2_Handle handle, uint32_t port, void *data)
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

void connect_rkrlv2_ports(LV2_Handle handle, uint32_t port, void *data)
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


/////////////////////////////////
//    Plugin Descriptors
////////////////////////////////

static const LV2_Descriptor eqlv2_descriptor=
{
    EQLV2_URI,
    init_eqlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_eqlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor complv2_descriptor=
{
    COMPLV2_URI,
    init_complv2,
    connect_rkrlv2_ports,
    0,//activate
    run_complv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor distlv2_descriptor=
{
    DISTLV2_URI,
    init_distlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_distlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor echolv2_descriptor=
{
    ECHOLV2_URI,
    init_echolv2,
    connect_rkrlv2_ports,
    0,//activate
    run_echolv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor choruslv2_descriptor=
{
    CHORUSLV2_URI,
    init_choruslv2,
    connect_rkrlv2_ports,
    0,//activate
    run_choruslv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor aphaselv2_descriptor=
{
    APHASELV2_URI,
    init_aphaselv2,
    connect_rkrlv2_ports,
    0,//activate
    run_aphaselv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor harmnomidlv2_descriptor=
{
    HARMNOMIDLV2_URI,
    init_harmnomidlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_harmnomidlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor exciterlv2_descriptor=
{
    EXCITERLV2_URI,
    init_exciterlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_exciterlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor panlv2_descriptor=
{
    PANLV2_URI,
    init_panlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_panlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor alienlv2_descriptor=
{
    ALIENLV2_URI,
    init_alienlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_alienlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor revelv2_descriptor=
{
    REVELV2_URI,
    init_revelv2,
    connect_rkrlv2_ports,
    0,//activate
    run_revelv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor eqplv2_descriptor=
{
    EQPLV2_URI,
    init_eqplv2,
    connect_rkrlv2_ports,
    0,//activate
    run_eqplv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor cablv2_descriptor=
{
    CABLV2_URI,
    init_cablv2,
    connect_rkrlv2_ports,
    0,//activate
    run_cablv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor mdellv2_descriptor=
{
    MDELLV2_URI,
    init_mdellv2,
    connect_rkrlv2_ports,
    0,//activate
    run_mdellv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor wahlv2_descriptor=
{
    WAHLV2_URI,
    init_wahlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_wahlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor derelv2_descriptor=
{
    DERELV2_URI,
    init_derelv2,
    connect_rkrlv2_ports,
    0,//activate
    run_derelv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor valvelv2_descriptor=
{
    VALVELV2_URI,
    init_valvelv2,
    connect_rkrlv2_ports,
    0,//activate
    run_valvelv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor dflangelv2_descriptor=
{
    DFLANGELV2_URI,
    init_dflangelv2,
    connect_rkrlv2_ports,
    0,//activate
    run_dflangelv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor ringlv2_descriptor=
{
    RINGLV2_URI,
    init_ringlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_ringlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor mbdistlv2_descriptor=
{
    MBDISTLV2_URI,
    init_mbdistlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_mbdistlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor arplv2_descriptor=
{
    ARPIELV2_URI,
    init_arplv2,
    connect_rkrlv2_ports,
    0,//activate
    run_arplv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor expandlv2_descriptor=
{
    EXPANDLV2_URI,
    init_expandlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_expandlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor shuflv2_descriptor=
{
    SHUFFLELV2_URI,
    init_shuflv2,
    connect_rkrlv2_ports,
    0,//activate
    run_shuflv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor synthlv2_descriptor=
{
    SYNTHLV2_URI,
    init_synthlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_synthlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor mbvollv2_descriptor=
{
    MBVOLLV2_URI,
    init_mbvollv2,
    connect_rkrlv2_ports,
    0,//activate
    run_mbvollv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor mutrolv2_descriptor=
{
    MUTROLV2_URI,
    init_mutrolv2,
    connect_rkrlv2_ports,
    0,//activate
    run_mutrolv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor echoverselv2_descriptor=
{
    ECHOVERSELV2_URI,
    init_echoverselv2,
    connect_rkrlv2_ports,
    0,//activate
    run_echoverselv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor coillv2_descriptor=
{
    COILLV2_URI,
    init_coillv2,
    connect_rkrlv2_ports,
    0,//activate
    run_coillv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor shelflv2_descriptor=
{
    SHELFLV2_URI,
    init_shelflv2,
    connect_rkrlv2_ports,
    0,//activate
    run_shelflv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor voclv2_descriptor=
{
    VOCODERLV2_URI,
    init_voclv2,
    connect_rkrlv2_ports,
    0,//activate
    run_voclv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor suslv2_descriptor=
{
    SUSTAINLV2_URI,
    init_suslv2,
    connect_rkrlv2_ports,
    0,//activate
    run_suslv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor seqlv2_descriptor=
{
    SEQUENCELV2_URI,
    init_seqlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_seqlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor shiftlv2_descriptor=
{
    SHIFTERLV2_URI,
    init_shiftlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_shiftlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor stomplv2_descriptor=
{
    STOMPLV2_URI,
    init_stomplv2,
    connect_rkrlv2_ports,
    0,//activate
    run_stomplv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor stompfuzzlv2_descriptor=
{
    STOMPFUZZLV2_URI,
    init_stomp_fuzzlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_stomplv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};


static const LV2_Descriptor revtronlv2_descriptor=
{
    REVTRONLV2_URI,
    init_revtronlv2,
    connect_rkrlv2_ports_w_atom,
    0,//activate
    run_revtronlv2,
    0,//deactivate
    cleanup_rkrlv2,
    revtron_extension_data
};

static const LV2_Descriptor echotronlv2_descriptor=
{
    ECHOTRONLV2_URI,
    init_echotronlv2,
    connect_rkrlv2_ports_w_atom,
    0,//activate
    run_echotronlv2,
    0,//deactivate
    cleanup_rkrlv2,
    echotron_extension_data
};

static const LV2_Descriptor sharmnomidlv2_descriptor=
{
    SHARMNOMIDLV2_URI,
    init_sharmnomidlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_sharmnomidlv2,
    0,//deactivate
    cleanup_rkrlv2
};

static const LV2_Descriptor mbcomplv2_descriptor=
{
    MBCOMPLV2_URI,
    init_mbcomplv2,
    connect_rkrlv2_ports,
    0,//activate
    run_mbcomplv2,
    0,//deactivate
    cleanup_rkrlv2
};

static const LV2_Descriptor otremlv2_descriptor=
{
    OPTTREMLV2_URI,
    init_otremlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_otremlv2,
    0,//deactivate
    cleanup_rkrlv2
};

static const LV2_Descriptor vibelv2_descriptor=
{
    VIBELV2_URI,
    init_vibelv2,
    connect_rkrlv2_ports,
    0,//activate
    run_vibelv2,
    0,//deactivate
    cleanup_rkrlv2
};

static const LV2_Descriptor inflv2_descriptor=
{
    INFLV2_URI,
    init_inflv2,
    connect_rkrlv2_ports,
    0,//activate
    run_inflv2,
    0,//deactivate
    cleanup_rkrlv2
};

static const LV2_Descriptor phaselv2_descriptor=
{
    PHASELV2_URI,
    init_phaselv2,
    connect_rkrlv2_ports,
    0,//activate
    run_phaselv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor gatelv2_descriptor=
{
    GATELV2_URI,
    init_gatelv2,
    connect_rkrlv2_ports,
    0,//activate
    run_gatelv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    switch (index)
    {
    case IEQ:
        return &eqlv2_descriptor ;
    case ICOMP:
        return &complv2_descriptor ;
    case IDIST:
        return &distlv2_descriptor ;
    case IECHO:
        return &echolv2_descriptor ;
    case ICHORUS:
        return &choruslv2_descriptor ;
    case IAPHASE:
        return &aphaselv2_descriptor ;
    case IHARM_NM:
        return &harmnomidlv2_descriptor ;
    case IEXCITER:
        return &exciterlv2_descriptor ;
    case IPAN:
        return &panlv2_descriptor ;
    case IAWAH:
        return &alienlv2_descriptor ;
    case IREV:
        return &revelv2_descriptor ;
    case IEQP:
        return &eqplv2_descriptor ;
    case ICAB:
        return &cablv2_descriptor ;
    case IMDEL:
        return &mdellv2_descriptor ;
    case IWAH:
        return &wahlv2_descriptor ;
    case IDERE:
        return &derelv2_descriptor ;
    case IVALVE:
        return &valvelv2_descriptor ;
    case IDFLANGE:
        return &dflangelv2_descriptor ;
    case IRING:
        return &ringlv2_descriptor ;
    case IMBDIST:
        return &mbdistlv2_descriptor ;
    case IARPIE:
        return &arplv2_descriptor ;
    case IEXPAND:
        return &expandlv2_descriptor ;
    case ISHUFF:
        return &shuflv2_descriptor ;
    case ISYNTH:
        return &synthlv2_descriptor ;
    case IMBVOL:
        return &mbvollv2_descriptor ;
    case IMUTRO:
        return &mutrolv2_descriptor ;
    case IECHOVERSE:
        return &echoverselv2_descriptor ;
    case ICOIL:
        return &coillv2_descriptor ;
    case ISHELF:
        return &shelflv2_descriptor ;
    case IVOC:
        return &voclv2_descriptor ;
    case ISUS:
        return &suslv2_descriptor ;
    case ISEQ:
        return &seqlv2_descriptor ;
    case ISHIFT:
        return &shiftlv2_descriptor ;
    case ISTOMP:
        return &stomplv2_descriptor ;
    case IFUZZ:
        return &stompfuzzlv2_descriptor ;
    case IREVTRON:
        return &revtronlv2_descriptor ;
    case IECHOTRON:
        return &echotronlv2_descriptor ;
    case ISHARM_NM:
    	return &sharmnomidlv2_descriptor ;
    case IMBCOMP:
    	return &mbcomplv2_descriptor ;
    case IOPTTREM:
    	return &otremlv2_descriptor ;
    case IVIBE:
    	return &vibelv2_descriptor ;
    case IINF:
    	return &inflv2_descriptor ;
    case IPHASE:
        return &phaselv2_descriptor ;
    case IGATE:
        return &gatelv2_descriptor ;
    default:
        return 0;
    }
}
