//Spencer Jackson
#include<lv2.h>
#include<lv2/lv2plug.in/ns/ext/urid/urid.h>
#include<lv2/lv2plug.in/ns/ext/midi/midi.h>
#include<lv2/lv2plug.in/ns/ext/atom/util.h>
#include<lv2/lv2plug.in/ns/ext/time/time.h>
#include<lv2/lv2plug.in/ns/ext/buf-size/buf-size.h>
#include<lv2/lv2plug.in/ns/ext/options/options.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
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


//this is the default hopefully hosts don't use periods of more than this, or they will communicate the max bufsize
#define INTERMEDIATE_BUFSIZE 1024



typedef struct _RKRLV2
{
    uint8_t nparams;
    uint8_t effectindex;//index of effect
    uint16_t period_max;
//    float* tempbuf_l;
//    float* tempbuf_r;

    float *input_l_p;
    float *input_r_p;
    float *output_l_p;
    float *output_r_p;
    float *param_p[16];
    float *dbg_p; 

    struct urids
    {
        LV2_URID    midi_MidiEvent;
        LV2_URID    atom_Float;
        LV2_URID    atom_Int;
        LV2_URID    bufsz_max;

    } URIDs;

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
}RKRLV2;

enum other_ports
{
	//be sure to account for index of array vs lv2 port index
	HARMONIZER_MIDI,
	VOCODER_AUX_IN = 7,
	VOCODER_VU_LEVEL = 8
};

// A few helper functions taken from the RKR object
void
wetdry_mix (float inl[], float inr[], float outl[], float outr[], float mix, uint32_t period)
{
    unsigned int i;
    float v1, v2;

    if (mix < 0.5f) {
        v1 = 1.0f;
        v2 = mix * 2.0f;
    } else {
        v1 = (1.0f - mix) * 2.0f;
        v2 = 1.0f;
    };


//   Reverb and musical delay are exceptions for some reason?
//    if ((NumEffect == 8) || (NumEffect == 15))
//        v2 *= v2;

    for (i = 0; i < period; i++) {
        outl[i] = inl[i] * v2 + outl[i] * v1;
        outr[i] = inr[i] * v2 + outr[i] * v1;
    };

}

//TODO: make this return error is required feature not supported
void getFeatures(RKRLV2* plug, const LV2_Feature * const* host_features)
{
    uint8_t i,j;
    LV2_URID_Map *urid_map;
    plug->period_max = INTERMEDIATE_BUFSIZE;
    for(i=0; host_features[i]; i++)
    {
        if(!strcmp(host_features[i]->URI,LV2_OPTIONS__options))
        {
            LV2_Options_Option* option;
            option = (LV2_Options_Option*)host_features[i]->data;
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
        else if(!strcmp(host_features[i]->URI,LV2_URID__map))
        {
            urid_map = (LV2_URID_Map *) host_features[i]->data;
            if(urid_map)
            {
                plug->URIDs.atom_Int = urid_map->map(urid_map->handle,LV2_ATOM__Int);
                plug->URIDs.atom_Float = urid_map->map(urid_map->handle,LV2_ATOM__Float);
                plug->URIDs.midi_MidiEvent = urid_map->map(urid_map->handle,LV2_MIDI__MidiEvent);
                plug->URIDs.bufsz_max = urid_map->map(urid_map->handle,LV2_BUF_SIZE__maxBlockLength);
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
    plug->effectindex = 0;

    plug->eq = new EQ(0,0,sample_freq);

    //eq has a bunch of setup stuff. Why isn't this in the EQ initalizer?
    for (int i = 0; i <= 45; i += 5) {
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

    //check and set changed parameters
    //eq1 is a little strange for parameters
    // DON'T USE THIS ONE AS AN EXAMPLE
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
        for(j=0;j<10;j++)
        {
            plug->eq->changepar(j*5+13,val);
        }
    }
    
    for(i=2;i<plug->nparams;i++)
    {
        val = (int)*plug->param_p[i]+64;//various freq. bands
        if(plug->eq->getpar(5*i + 2) != val)
        {
            plug->eq->changepar(5*i+2,val);
        }
    }

    //eq does in inline?
    memcpy(plug->output_l_p,plug->input_l_p,sizeof(float)*nframes);
    memcpy(plug->output_r_p,plug->input_r_p,sizeof(float)*nframes);

    //now set out ports
    plug->eq->efxoutl = plug->output_l_p;
    plug->eq->efxoutr = plug->output_r_p;

    //now run
    plug->eq->out(plug->output_l_p,plug->output_r_p,nframes);
    
    return;
}

///// comp /////////
LV2_Handle init_complv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 9;
    plug->effectindex = 1;

    plug->comp = new Compressor(0,0, sample_freq);

    return plug;
}

void run_complv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    //check and set changed parameters
    for(i=0;i<plug->nparams;i++)
    {
        val = (int)*plug->param_p[i];
       if(plug->comp->getpar(i+1) != val)//this effect is 1 indexed
       {
           plug->comp->Compressor_Change(i+1,val);
       }
    }
    
    //comp does in inline
    memcpy(plug->output_l_p,plug->input_l_p,sizeof(float)*nframes);
    memcpy(plug->output_r_p,plug->input_r_p,sizeof(float)*nframes);
    
    //now set out ports
    plug->comp->efxoutl = plug->output_l_p;
    plug->comp->efxoutr = plug->output_r_p;

    //now run
    plug->comp->out(plug->output_l_p,plug->output_r_p,nframes);
    
    return;
}

///// dist /////////
LV2_Handle init_distlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));
    
    plug->nparams = 12;
    plug->effectindex = 2;

    getFeatures(plug,host_features);

    plug->dist = new Distorsion(0,0, sample_freq, plug->period_max, /*oversampling*/2, 
                                /*up interpolation method*/0, /*down interpolation method*/2);

    return plug;
}

void run_distlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

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
    for(i++;i<plug->nparams-1;i++)//2-10
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

    //now set out ports and global period size
    plug->dist->efxoutl = plug->output_l_p;
    plug->dist->efxoutr = plug->output_r_p;

    //now run
    plug->dist->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p, plug->dist->outvolume, nframes);
    
    return;
}

///// echo /////////
LV2_Handle init_echolv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));
    
    plug->nparams = 9;
    plug->effectindex = 3;

    plug->echo = new Echo(0,0,sample_freq);

    return plug;
}

void run_echolv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

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
    for(i++;i<plug->nparams;i++)
    {
        val = (int)*plug->param_p[i];
       if(plug->echo->getpar(i) != val)
       {
           plug->echo->changepar(i,val);
       }
    }

    //now set out ports and global period size
    plug->echo->efxoutl = plug->output_l_p;
    plug->echo->efxoutr = plug->output_r_p;

    //now run
    plug->echo->out(plug->input_l_p,plug->input_r_p,nframes);
    
    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p, plug->echo->outvolume, nframes);
    
    return;
}

///// chorus/flanger /////////
LV2_Handle init_choruslv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));
    
    plug->nparams = 12;
    plug->effectindex = 4;

    plug->chorus = new Chorus(0,0,sample_freq);

    return plug;
}

void run_choruslv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

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
    for(i++;i<5;i++)//2-4
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
    for(i++;i<10;i++) // 6-9
    {
        val = (int)*plug->param_p[i];
        if(plug->chorus->getpar(i) != val)
        {
            plug->chorus->changepar(i,val);
        }
    }
    //skip param 10
    for(;i<plug->nparams;i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->chorus->getpar(i+1) != val)
        {
            plug->chorus->changepar(i+1,val);
        }
    }

    //now set out ports and global period size
    plug->chorus->efxoutl = plug->output_l_p;
    plug->chorus->efxoutr = plug->output_r_p;

    //now run
    plug->chorus->out(plug->input_l_p,plug->input_r_p,nframes);
    
    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p, plug->chorus->outvolume, nframes);
    
    return;
}

///// Analog Phaser /////////
LV2_Handle init_aphaselv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));
    
    plug->nparams = 13;
    plug->effectindex = 5;

    plug->aphase = new Analog_Phaser(0,0,sample_freq);

    return plug;
}

void run_aphaselv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    //LFO effects require period be set before setting other params
    plug->aphase->PERIOD = nframes;

    //check and set changed parameters
    for(i=0;i<5;i++)//0-4
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
    for(i++;i<plug->nparams;i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->aphase->getpar(i+1) != val)
        {
            plug->aphase->changepar(i+1,val);
        }
    }

    //now set out ports and global period size
    plug->aphase->efxoutl = plug->output_l_p;
    plug->aphase->efxoutr = plug->output_r_p;

    //now run
    plug->aphase->out(plug->input_l_p,plug->input_r_p,nframes);
    
    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p, plug->aphase->outvolume, nframes);
    
    return;
}

//////// harmonizer /////////
LV2_Handle init_harmnomidlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));
    
    plug->nparams = 10;
    plug->effectindex = 6;

    getFeatures(plug,host_features);

    //magic numbers: shift qual 4, downsample 5, up qual 4, down qual 2,
    plug->harm = new Harmonizer(0,0,4,5,4,2, plug->period_max, sample_freq);
    plug->noteID = new Recognize(0,0,.6, sample_freq, 440.0);//.6 is default trigger value
    plug->chordID = new RecChord();



    return plug;
}

void run_harmnomidlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    //check and set changed parameters
    i = 0;
    val = (int)*plug->param_p[i];// 0 wet/dry
    if(plug->harm->getpar(i) != val)
    {
        plug->harm->changepar(i,val);
    }
    for(i++;i<3;i++)//1-2
    {
        val = (int)*plug->param_p[i] + 64;
        if(plug->harm->getpar(i+1) != val)
        {
            plug->harm->changepar(i+1,val);
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
    for(i++;i<8;i++)//6-7
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
    for(;i<10;i++)// 8-9
    {
        val = (int)*plug->param_p[i] + 64;
        if(plug->harm->getpar(i+1) != val)
        {
            plug->harm->changepar(i+1,val);
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
    //now set out ports and global period size
    plug->harm->efxoutl = plug->output_l_p;
    plug->harm->efxoutr = plug->output_r_p;

    //now run
    plug->harm->out(plug->input_l_p,plug->input_r_p,nframes);
    
    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p,
    		plug->harm->outvolume, nframes);
    
    return;
}


///// exciter /////////
LV2_Handle init_exciterlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 13;
    plug->effectindex = 7;

    getFeatures(plug,host_features);

    plug->exciter = new Exciter(0,0, sample_freq, plug->period_max);

    return plug;
}

void run_exciterlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    //check and set changed parameters
    for(i=0;i<plug->nparams;i++)
    {
        val = (int)*plug->param_p[i];
       if(plug->exciter->getpar(i) != val)//this effect is 1 indexed
       {
           plug->exciter->changepar(i,val);
       }
    }

    //comp does in inline
    memcpy(plug->output_l_p,plug->input_l_p,sizeof(float)*nframes);
    memcpy(plug->output_r_p,plug->input_r_p,sizeof(float)*nframes);

    //now set out ports
    plug->exciter->efxoutl = plug->output_l_p;
    plug->exciter->efxoutr = plug->output_r_p;

    //now run
    plug->exciter->out(plug->output_l_p,plug->output_r_p,nframes);

    return;
}

///// pan /////////
LV2_Handle init_panlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 9;
    plug->effectindex = 8;

    plug->pan = new Pan(0,0,sample_freq);

    return plug;
}

void run_panlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

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
    for(i++;i<5;i++)//2-4
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
    for(i++;i<plug->nparams;i++)//6-8
    {
        val = (int)*plug->param_p[i];
        if(plug->pan->getpar(i+1) != val)
        {
            plug->pan->changepar(i+1,val);
        }
    }

    //now set out ports and global period size
    plug->pan->efxoutl = plug->output_l_p;
    plug->pan->efxoutr = plug->output_r_p;

    //now run
    plug->pan->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p, plug->pan->outvolume, nframes);

    return;
}

///// AlienWah /////////
LV2_Handle init_alienlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 11;
    plug->effectindex = 9;

    plug->alien = new Alienwah(0,0,sample_freq);

    return plug;
}

void run_alienlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

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
    for(i++;i<5;i++)//2-4
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
    for(i++;i<plug->nparams;i++)//6-10
    {
        val = (int)*plug->param_p[i];
        if(plug->alien->getpar(i+1) != val)
        {
            plug->alien->changepar(i+1,val);
        }
    }

    //now set out ports and global period size
    plug->alien->efxoutl = plug->output_l_p;
    plug->alien->efxoutr = plug->output_r_p;

    //now run
    plug->alien->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p, plug->alien->outvolume, nframes);

    return;
}

///// reverb /////////
LV2_Handle init_revelv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 10;
    plug->effectindex = 10;

    getFeatures(plug,host_features);

    plug->reve = new Reverb(0,0,sample_freq,plug->period_max);

    return plug;
}

void run_revelv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

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
    for(i++;i<5;i++)//2-4
    {
        val = (int)*plug->param_p[i];
        if(plug->reve->getpar(i) != val)
        {
            plug->reve->changepar(i,val);
        }
    }
    for(;i<plug->nparams;i++)//7-11 (5 and 6 are skipped
    {
        val = (int)*plug->param_p[i];
        if(plug->reve->getpar(i+2) != val)
        {
            plug->reve->changepar(i+2,val);
        }
    }

    //now set out ports and global period size
    plug->reve->efxoutl = plug->output_l_p;
    plug->reve->efxoutr = plug->output_r_p;

    //now run
    plug->reve->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p, plug->reve->outvolume, nframes);

    return;
}

///// EQ Parametric /////////
LV2_Handle init_eqplv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 10;
    plug->effectindex = 11;

    plug->eq = new EQ(0,0,sample_freq);

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

    //check and set changed parameters
    i = 0;

    val = (int)*plug->param_p[0]+64;//gain
    if(plug->eq->getpar(0) != val)
    {
        plug->eq->changepar(0,val);
    }
    
    for(i=1;i<4;i++)//1-3 low band
    {
        val = (int)*plug->param_p[i]+64;
        if(plug->eq->getpar(i + 10) != val)
        {
            plug->eq->changepar(i+10,val);
        }
    }
    for(;i<7;i++)//4-6 mid band
    {
        val = (int)*plug->param_p[i]+64;
        if(plug->eq->getpar(i + 12) != val)
        {
            plug->eq->changepar(i+12,val);
        }
    }
    for(;i<plug->nparams;i++)//7-9 high band
    {
        val = (int)*plug->param_p[i]+64;
        if(plug->eq->getpar(i + 14) != val)
        {
            plug->eq->changepar(i+14,val);
        }
    }

    //eq does it inline?
    memcpy(plug->output_l_p,plug->input_l_p,sizeof(float)*nframes);
    memcpy(plug->output_r_p,plug->input_r_p,sizeof(float)*nframes);

    //now set out ports
    plug->eq->efxoutl = plug->output_l_p;
    plug->eq->efxoutr = plug->output_r_p;

    //now run
    plug->eq->out(plug->output_l_p,plug->output_r_p,nframes);
    
    return;
}

///// Cabinet /////////
LV2_Handle init_cablv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 10;
    plug->effectindex = 12;

    plug->cab = new Cabinet(0,0,sample_freq);

    return plug;
}

void run_cablv2(LV2_Handle handle, uint32_t nframes)
{
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

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

    //cab does it inline?
    memcpy(plug->output_l_p,plug->input_l_p,sizeof(float)*nframes);
    memcpy(plug->output_r_p,plug->input_r_p,sizeof(float)*nframes);

    //now set out ports
    plug->cab->efxoutl = plug->output_l_p;
    plug->cab->efxoutr = plug->output_r_p;

    //now run
    plug->cab->out(plug->output_l_p,plug->output_r_p,nframes);
    
    return;
}

///// Musical Delay /////////
LV2_Handle init_mdellv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 13;
    plug->effectindex = 13;

    plug->mdel = new MusicDelay (0,0,sample_freq);

    return plug;
}

void run_mdellv2(LV2_Handle handle, uint32_t nframes)
{
    int val;
    uint8_t i;

    RKRLV2* plug = (RKRLV2*)handle;

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
    
    for(i=2;i<7;i++)//2-6 
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
    for(i++;i<plug->nparams;i++)//8-12
    {
        val = (int)*plug->param_p[i];
        if(plug->mdel->getpar(i) != val)
        {
            plug->mdel->changepar(i,val);
        }
    }

    //now set out ports and global period size
    plug->mdel->efxoutl = plug->output_l_p;
    plug->mdel->efxoutr = plug->output_r_p;

    //now run
    plug->mdel->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p, plug->mdel->outvolume, nframes);


    return;
}

///// wahwah /////////
LV2_Handle init_wahlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));
    
    plug->nparams = 11;
    plug->effectindex = 14;


    plug->wah = new DynamicFilter(0,0,sample_freq);

    return plug;
}

void run_wahlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

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
    for(i++;i<5;i++)//2-4
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
    for(i++;i<plug->nparams;i++) // 6-11
    {
        val = (int)*plug->param_p[i];
        if(plug->wah->getpar(i) != val)
        {
            plug->wah->changepar(i,val);
        }
    }

    //now set out ports and global period size
    plug->wah->efxoutl = plug->output_l_p;
    plug->wah->efxoutr = plug->output_r_p;

    //now run
    plug->wah->out(plug->input_l_p,plug->input_r_p,nframes);
    
    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p, plug->wah->outvolume, nframes);
    
    return;
}

///// derelict /////////
LV2_Handle init_derelv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 12;
    plug->effectindex = 15;

    getFeatures(plug,host_features);

    plug->dere = new NewDist(0,0, sample_freq, plug->period_max, /*oversampling*/2,
                                /*up interpolation method*/0, /*down interpolation method*/2);

    return plug;
}

void run_derelv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

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
    for(i++;i<plug->nparams;i++)//2-11
    {
        val = (int)*plug->param_p[i];
       if(plug->dere->getpar(i) != val)
       {
           plug->dere->changepar(i,val);
       }
    }

    //now set out ports and global period size
    plug->dere->efxoutl = plug->output_l_p;
    plug->dere->efxoutr = plug->output_r_p;

    //now run
    plug->dere->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p, plug->dere->outvolume, nframes);

    return;
}

///// valve /////////
LV2_Handle init_valvelv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 13;
    plug->effectindex = 16;

    getFeatures(plug,host_features);

    plug->valve = new Valve(0,0, sample_freq, plug->period_max);

    return plug;
}

void run_valvelv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

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
    for(i++;i<plug->nparams;i++)//2-12
    {
        val = (int)*plug->param_p[i];
       if(plug->valve->getpar(i) != val)
       {
           plug->valve->changepar(i,val);
       }
    }

    //now set out ports and global period size
    plug->valve->efxoutl = plug->output_l_p;
    plug->valve->efxoutr = plug->output_r_p;

    //now run
    plug->valve->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p, plug->valve->outvolume, nframes);

    return;
}

///// dual flange /////////
LV2_Handle init_dflangelv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 15;
    plug->effectindex = 17;

    plug->dflange = new Dflange(0,0, sample_freq);

    return plug;
}

void run_dflangelv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    //lfo effects must set period before params
    plug->dflange->PERIOD = nframes;

    //check and set changed parameters
    i=0;
    val = (int)*plug->param_p[i]-64;//0 Wet/dry
    if(plug->dflange->getpar(i) != val)
    {
        plug->dflange->changepar(i,val);
    }
    for(i++;i<plug->nparams;i++)//1-14
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

    //dflange does it inline?
    memcpy(plug->output_l_p,plug->input_l_p,sizeof(float)*nframes);
    memcpy(plug->output_r_p,plug->input_r_p,sizeof(float)*nframes);

    //now set out ports
    plug->dflange->efxoutl = plug->output_l_p;
    plug->dflange->efxoutr = plug->output_r_p;

    //now run
    plug->dflange->out(plug->output_l_p,plug->output_r_p,nframes);

    return;
}

//////// ring /////////
LV2_Handle init_ringlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 13;
    plug->effectindex = 18;

    //magic numbers: shift qual 4, downsample 5, up qual 4, down qual 2,
    plug->ring = new Ring(0,0, sample_freq);
    plug->noteID = new Recognize(0,0,.6, sample_freq, 440.0);//.6 is default trigger value

    return plug;
}

void run_ringlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

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
    for(i++;i<plug->nparams;i++)//3-12
    {
        val = (int)*plug->param_p[i];
        if(plug->ring->getpar(i) != val)
        {
            plug->ring->changepar(i,val);
        }
    }
//see process.C ln 1539

    //TODO may need to make sure input is over some threshold
    if(plug->ring->Pafreq)
    {
    	//copy over the data so that noteID doesn't tamper with it
        memcpy(plug->output_l_p,plug->input_l_p,sizeof(float)*nframes);
        memcpy(plug->output_r_p,plug->input_r_p,sizeof(float)*nframes);
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
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p,
    		plug->ring->outvolume, nframes);

    return;
}


///// mbdist /////////
LV2_Handle init_mbdistlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 15;
    plug->effectindex = 19;

    getFeatures(plug,host_features);

    plug->mbdist = new MBDist(0,0, sample_freq, plug->period_max, /*oversampling*/2,
                                /*up interpolation method*/0, /*down interpolation method*/2);

    return plug;
}

void run_mbdistlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

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
    for(i++;i<plug->nparams;i++)//2-12
    {
        val = (int)*plug->param_p[i];
       if(plug->mbdist->getpar(i) != val)
       {
           plug->mbdist->changepar(i,val);
       }
    }

    //now set out ports and global period size
    plug->mbdist->efxoutl = plug->output_l_p;
    plug->mbdist->efxoutr = plug->output_r_p;

    //now run
    plug->mbdist->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p, plug->mbdist->outvolume, nframes);

    return;
}

///// arp /////////
LV2_Handle init_arplv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));
    
    plug->nparams = 11;
    plug->effectindex = 20;

    plug->arp = new Arpie(0,0,sample_freq);

    return plug;
}

void run_arplv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

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
    for(i++;i<plug->nparams;i++)//rest are not offset
    {
        val = (int)*plug->param_p[i];
       if(plug->arp->getpar(i) != val)
       {
           plug->arp->changepar(i,val);
       }
    }

    //now set out ports and global period size
    plug->arp->efxoutl = plug->output_l_p;
    plug->arp->efxoutr = plug->output_r_p;

    //now run
    plug->arp->out(plug->input_l_p,plug->input_r_p,nframes);
    
    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p, plug->arp->outvolume, nframes);
    
    return;
}

///// expand /////////
LV2_Handle init_expandlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 7;
    plug->effectindex = 21;

    plug->expand = new Expander(0,0, sample_freq);

    return plug;
}

void run_expandlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    //check and set changed parameters
    for(i=0;i<plug->nparams;i++)
    {
        val = (int)*plug->param_p[i];
       if(plug->expand->getpar(i+1) != val)//this effect is 1 indexed
       {
           plug->expand->Expander_Change(i+1,val);
       }
    }

    //comp does in inline
    memcpy(plug->output_l_p,plug->input_l_p,sizeof(float)*nframes);
    memcpy(plug->output_r_p,plug->input_r_p,sizeof(float)*nframes);

    //now set out ports
    plug->expand->efxoutl = plug->output_l_p;
    plug->expand->efxoutr = plug->output_r_p;

    //now run
    plug->expand->out(plug->output_l_p,plug->output_r_p,nframes);

    return;
}

///// shuffle /////////
LV2_Handle init_shuflv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 11;
    plug->effectindex = 22;

    getFeatures(plug,host_features);

    plug->shuf = new Shuffle(0,0,sample_freq,plug->period_max);

    return plug;
}

void run_shuflv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    //check and set changed parameters
    for(i=0;i<plug->nparams;i++)//rest are not offset
    {
        val = (int)*plug->param_p[i];
       if(plug->shuf->getpar(i) != val)
       {
           plug->shuf->changepar(i,val);
       }
    }

    //now set out ports and global period size
    plug->shuf->efxoutl = plug->output_l_p;
    plug->shuf->efxoutr = plug->output_r_p;

    //now run
    plug->shuf->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p, plug->shuf->outvolume, nframes);

    return;
}


///// synth /////////
LV2_Handle init_synthlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 16;
    plug->effectindex = 23;

    plug->synth = new Synthfilter(0,0,sample_freq);

    return plug;
}

void run_synthlv2(LV2_Handle handle, uint32_t nframes)
{
    unsigned int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    //LFO effects require period be set before setting other params
    plug->synth->PERIOD = nframes;

    //check and set changed parameters
    for(i=0;i<5;i++)//0-4
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
    for(i++;i<plug->nparams;i++)//6-10
    {
        val = (int)*plug->param_p[i];
        if(plug->synth->getpar(i+1) != val)
        {
            plug->synth->changepar(i+1,val);
        }
    }

    //now set out ports and global period size
    plug->synth->efxoutl = plug->output_l_p;
    plug->synth->efxoutr = plug->output_r_p;

    //now run
    plug->synth->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p, plug->synth->outvolume, nframes);

    return;
}

///// mbvol /////////
LV2_Handle init_mbvollv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 14;
    plug->effectindex = 24;

    getFeatures(plug,host_features);

    plug->mbvol = new MBVvol(0,0,sample_freq,plug->period_max);

    return plug;
}

void run_mbvollv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    plug->mbvol->PERIOD = nframes;

    //check and set changed parameters
    for(i=0;i<3;i++)
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
    for(i++;i<6;i++)
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
    for(i++;i<plug->nparams;i++)//skip legacy combi setting
    {
        val = (int)*plug->param_p[i];
       if(plug->mbvol->getpar(i+1) != val)
       {
           plug->mbvol->changepar(i+1,val);
       }
    }

    //now set out ports and global period size
    plug->mbvol->efxoutl = plug->output_l_p;
    plug->mbvol->efxoutr = plug->output_r_p;

    //now run
    plug->mbvol->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p, plug->mbvol->outvolume, nframes);

    return;
}

///// mutro /////////
LV2_Handle init_mutrolv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 19;
    plug->effectindex = 25;


    plug->mutro = new RyanWah(0,0,sample_freq);

    return plug;
}

void run_mutrolv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    plug->mutro->PERIOD = nframes;

    //check and set changed parameters
    for(i=0;i<5;i++)
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
    for(i++;i<17;i++)
    {
        val = (int)*plug->param_p[i];
       if(plug->mutro->getpar(i) != val)
       {
           plug->mutro->changepar(i,val);
       }
    }
    for(;i<plug->nparams;i++)//skip legacy mode and preset setting
    {
        val = (int)*plug->param_p[i];
       if(plug->mutro->getpar(i+2) != val)
       {
           plug->mutro->changepar(i+2,val);
       }
    }

    //now set out ports and global period size
    plug->mutro->efxoutl = plug->output_l_p;
    plug->mutro->efxoutr = plug->output_r_p;

    //now run
    plug->mutro->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p, plug->mutro->outvolume, nframes);

    return;
}

///// echoverse /////////
LV2_Handle init_echoverselv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 10;
    plug->effectindex = 26;

    plug->echoverse = new RBEcho(0,0,sample_freq);

    return plug;
}

void run_echoverselv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

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
    for(i++;i<5;i++)//3,4 LR delay and angle is offset
    {
        val = (int)*plug->param_p[i]+64;
       if(plug->echoverse->getpar(i) != val)
       {
           plug->echoverse->changepar(i,val);
       }
    }
    for( ;i<plug->nparams;i++)
    {
        val = (int)*plug->param_p[i];
       if(plug->echoverse->getpar(i) != val)
       {
           plug->echoverse->changepar(i,val);
       }
    }

    //now set out ports and global period size
    plug->echoverse->efxoutl = plug->output_l_p;
    plug->echoverse->efxoutr = plug->output_r_p;

    //now run
    plug->echoverse->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p, plug->echoverse->outvolume, nframes);

    return;
}

///// coilcrafter /////////
LV2_Handle init_coillv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 7;
    plug->effectindex = 27;

    getFeatures(plug,host_features);

    plug->coil = new CoilCrafter(0,0,sample_freq,plug->period_max);

    return plug;
}

void run_coillv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    //check and set changed parameters
    i=0;
    val = (int)*plug->param_p[i];//wet/dry
    if(plug->coil->getpar(i) != val)
    {
        plug->coil->changepar(i,val);
    }
    for(i++;i<plug->nparams;i++)//skip origin and destinations
    {
        val = (int)*plug->param_p[i];
       if(plug->coil->getpar(i+2) != val)
       {
           plug->coil->changepar(i+2,val);
       }
    }
     //coilcrafter does it inline
    memcpy(plug->output_l_p,plug->input_l_p,sizeof(float)*nframes);
    memcpy(plug->output_r_p,plug->input_r_p,sizeof(float)*nframes);

    //now set out ports
    plug->coil->efxoutl = plug->output_l_p;
    plug->coil->efxoutr = plug->output_r_p;

    //now run
    plug->coil->out(plug->output_l_p,plug->output_r_p,nframes);

    return;
}

///// shelfboost /////////
LV2_Handle init_shelflv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 5;
    plug->effectindex = 28;

    plug->shelf = new ShelfBoost(0,0,sample_freq);

    return plug;
}

void run_shelflv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    //check and set changed parameters
    for(i=0;i<plug->nparams;i++)
    {
        val = (int)*plug->param_p[i];
       if(plug->shelf->getpar(i) != val)
       {
           plug->shelf->changepar(i,val);
       }
    }
     //coilcrafter does it inline
    memcpy(plug->output_l_p,plug->input_l_p,sizeof(float)*nframes);
    memcpy(plug->output_r_p,plug->input_r_p,sizeof(float)*nframes);

    //now set out ports
    plug->shelf->efxoutl = plug->output_l_p;
    plug->shelf->efxoutr = plug->output_r_p;

    //now run
    plug->shelf->out(plug->output_l_p,plug->output_r_p,nframes);

    return;
}

///// Vocoder /////////
LV2_Handle init_voclv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 7;
    plug->effectindex = 29;

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
    for(i++;i<plug->nparams;i++)
    {
        val = (int)*plug->param_p[i];
       if(plug->voc->getpar(i) != val)
       {
           plug->voc->changepar(i,val);
       }
    }

    //set aux input and out ports
    plug->voc->auxresampled = plug->param_p[VOCODER_AUX_IN];
    plug->voc->efxoutl = plug->output_l_p;
    plug->voc->efxoutr = plug->output_r_p;

    //now run
    plug->voc->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p, plug->voc->outvolume, nframes);

    //and set VU meter
    *plug->param_p[VOCODER_VU_LEVEL] = plug->voc->vulevel;

    return;
}

///// Sustainer /////////
LV2_Handle init_suslv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 2;
    plug->effectindex = 30;


    plug->sus = new Sustainer(0,0,sample_freq);

    return plug;
}

void run_suslv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    //check and set changed parameters
    for(i=0;i<plug->nparams;i++)
    {
        val = (int)*plug->param_p[i];
       if(plug->sus->getpar(i) != val)
       {
           plug->sus->changepar(i,val);
       }
    }

     //sustainer does it inline
    memcpy(plug->output_l_p,plug->input_l_p,sizeof(float)*nframes);
    memcpy(plug->output_r_p,plug->input_r_p,sizeof(float)*nframes);

    //now set out ports
    plug->sus->efxoutl = plug->output_l_p;
    plug->sus->efxoutr = plug->output_r_p;

    //now run
    plug->sus->out(plug->output_l_p,plug->output_r_p,nframes);

    return;
}

///// Sequence /////////
LV2_Handle init_seqlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 15;
    plug->effectindex = 31;

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

    //check and set changed parameters
    for(i=0;i<10;i++)
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
    for(i++;i<plug->nparams;i++)
    {
        val = (int)*plug->param_p[i];
       if(plug->seq->getpar(i) != val)
       {
           plug->seq->changepar(i,val);
       }
    }

    //set out ports
    plug->seq->efxoutl = plug->output_l_p;
    plug->seq->efxoutr = plug->output_r_p;

    //now run
    plug->seq->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p, plug->seq->outvolume, nframes);

    return;
}

///// Shifter /////////
LV2_Handle init_shiftlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 10;
    plug->effectindex = 32;

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

    //check and set changed parameters
    i=0;
    val = (int)*plug->param_p[i];//wet/dry
    if(plug->shift->getpar(i) != val)
    {
        plug->shift->changepar(i,val);
    }
    for(i++;i<3;i++)//pan, gain
    {
       val = (int)*plug->param_p[i]+64;
       if(plug->shift->getpar(i) != val)
       {
           plug->shift->changepar(i,val);
       }
    }
    for(;i<plug->nparams;i++)
    {
       val = (int)*plug->param_p[i];
       if(plug->shift->getpar(i) != val)
       {
           plug->shift->changepar(i,val);
       }
    }

    //set out ports
    plug->shift->efxoutl = plug->output_l_p;
    plug->shift->efxoutr = plug->output_r_p;

    //now run
    plug->shift->out(plug->input_l_p,plug->input_r_p,nframes);

    //and for whatever reason we have to do the wet/dry mix ourselves
    wetdry_mix(plug->input_l_p, plug->input_r_p, plug->output_l_p, plug->output_r_p, plug->shift->outvolume, nframes);

    return;
}

/////////////////////////////////
///////// END OF FX ///////////// 
/////////////////////////////////



void cleanup_rkrlv2(LV2_Handle handle)
{
    RKRLV2* plug = (RKRLV2*)handle;
    switch(plug->effectindex)
    {
        case 0:
        case 11:
            delete plug->eq;//eql, eqp, cabinet
            break;
        case 1:
        	delete plug->comp;
        	break;
        case 2:
        	delete plug->dist;
        	break;
        case 3:
        	delete plug->echo;
        	break;
        case 4:
        	delete plug->chorus;
        	break;
        case 5:
        	delete plug->aphase;
        	break;
        case 6:
        	delete plug->harm;
        	delete plug->noteID;
        	delete plug->chordID;
        	break;
        case 7:
            delete plug->exciter;
            break;
        case 8:
        	delete plug->pan;
        	break;
        case 9:
        	delete plug->alien;
        	break;
        case 10:
            delete plug->reve;
            break;
        case 12:
            delete plug->cab;
            break;
        case 13:
            delete plug->mdel;
            break;
        case 14:
            delete plug->wah;
            break;
        case 15:
            delete plug->dere;
            break;
        case 16:
            delete plug->valve;
            break;
        case 17:
            delete plug->dflange;
            break;
        case 18:
        	delete plug->noteID;
            delete plug->ring;
            break;
        case 19:
        	delete plug->mbdist; 
            break;
        case 20:
        	delete plug->arp; 
            break;
        case 21:
        	delete plug->expand;
            break;
        case 22:
        	delete plug->shuf;
            break;
        case 23:
        	delete plug->synth;
            break;
        case 24:
        	delete plug->mbvol;
            break;
        case 25:
        	delete plug->mutro;
            break;
        case 26:
        	delete plug->echoverse;
        	break;
        case 27:
        	delete plug->coil;
        	break;
        case 28:
        	delete plug->shelf;
        	break;
        case 29:
        	delete plug->voc;
        	break;
        case 30:
        	delete plug->sus;
        	break;
        case 31:
        	delete plug->seq;
        	break;
        case 32:
        	delete plug->shift;
        	break;
    }
    free(plug);
}

void connect_rkrlv2_ports(LV2_Handle handle, uint32_t port, void *data)
{
    RKRLV2* plug = (RKRLV2*)handle;
    switch(port)
    {
    case INL:         plug->input_l_p = (float*)data;break;
    case INR:         plug->input_r_p = (float*)data;break;
    case OUTL:        plug->output_l_p = (float*)data;break;
    case OUTR:        plug->output_r_p = (float*)data;break;
    case PARAM0:      plug->param_p[0] = (float*)data;break;
    case PARAM1:      plug->param_p[1] = (float*)data;break;
    case PARAM2:      plug->param_p[2] = (float*)data;break;
    case PARAM3:      plug->param_p[3] = (float*)data;break;
    case PARAM4:      plug->param_p[4] = (float*)data;break;
    case PARAM5:      plug->param_p[5] = (float*)data;break;
    case PARAM6:      plug->param_p[6] = (float*)data;break;
    case PARAM7:      plug->param_p[7] = (float*)data;break;
    case PARAM8:      plug->param_p[8] = (float*)data;break;
    case PARAM9:      plug->param_p[9] = (float*)data;break;
    case PARAM10:     plug->param_p[10] = (float*)data;break;
    case PARAM11:     plug->param_p[11] = (float*)data;break;
    case PARAM12:     plug->param_p[12] = (float*)data;break;
    case PARAM13:     plug->param_p[13] = (float*)data;break;
    case PARAM14:     plug->param_p[14] = (float*)data;break;
    case PARAM15:     plug->param_p[15] = (float*)data;break;
    case PARAM16:     plug->param_p[16] = (float*)data;break;
    case PARAM17:     plug->param_p[17] = (float*)data;break;
    case PARAM18:     plug->param_p[18] = (float*)data;break;
    case DBG:         plug->dbg_p = (float*)data;break;
    default:         puts("UNKNOWN PORT YO!!");
    }
}

static const LV2_Descriptor eqlv2_descriptor={
    EQLV2_URI,
    init_eqlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_eqlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor complv2_descriptor={
    COMPLV2_URI,
    init_complv2,
    connect_rkrlv2_ports,
    0,//activate
    run_complv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor distlv2_descriptor={
    DISTLV2_URI,
    init_distlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_distlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor echolv2_descriptor={
    ECHOLV2_URI,
    init_echolv2,
    connect_rkrlv2_ports,
    0,//activate
    run_echolv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor choruslv2_descriptor={
    CHORUSLV2_URI,
    init_choruslv2,
    connect_rkrlv2_ports,
    0,//activate
    run_choruslv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor aphaselv2_descriptor={
    APHASELV2_URI,
    init_aphaselv2,
    connect_rkrlv2_ports,
    0,//activate
    run_aphaselv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor harmnomidlv2_descriptor={
    HARMNOMIDLV2_URI,
    init_harmnomidlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_harmnomidlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor exciterlv2_descriptor={
    EXCITERLV2_URI,
    init_exciterlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_exciterlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor panlv2_descriptor={
    PANLV2_URI,
    init_panlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_panlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor alienlv2_descriptor={
    ALIENLV2_URI,
    init_alienlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_alienlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor revelv2_descriptor={
    REVELV2_URI,
    init_revelv2,
    connect_rkrlv2_ports,
    0,//activate
    run_revelv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor eqplv2_descriptor={
    EQPLV2_URI,
    init_eqplv2,
    connect_rkrlv2_ports,
    0,//activate
    run_eqplv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor cablv2_descriptor={
    CABLV2_URI,
    init_cablv2,
    connect_rkrlv2_ports,
    0,//activate
    run_cablv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor mdellv2_descriptor={
    MDELLV2_URI,
    init_mdellv2,
    connect_rkrlv2_ports,
    0,//activate
    run_mdellv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor wahlv2_descriptor={
    WAHLV2_URI,
    init_wahlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_wahlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor derelv2_descriptor={
    DERELV2_URI,
    init_derelv2,
    connect_rkrlv2_ports,
    0,//activate
    run_derelv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor valvelv2_descriptor={
    VALVELV2_URI,
    init_valvelv2,
    connect_rkrlv2_ports,
    0,//activate
    run_valvelv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor dflangelv2_descriptor={
    DFLANGELV2_URI,
    init_dflangelv2,
    connect_rkrlv2_ports,
    0,//activate
    run_dflangelv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor ringlv2_descriptor={
    RINGLV2_URI,
    init_ringlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_ringlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor mbdistlv2_descriptor={
    MBDISTLV2_URI,
    init_mbdistlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_mbdistlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor arplv2_descriptor={
    ARPIELV2_URI,
    init_arplv2,
    connect_rkrlv2_ports,
    0,//activate
    run_arplv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor expandlv2_descriptor={
    EXPANDLV2_URI,
    init_expandlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_expandlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor shuflv2_descriptor={
    SHUFFLELV2_URI,
    init_shuflv2,
    connect_rkrlv2_ports,
    0,//activate
    run_shuflv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor synthlv2_descriptor={
    SYNTHLV2_URI,
    init_synthlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_synthlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor mbvollv2_descriptor={
    MBVOLLV2_URI,
    init_mbvollv2,
    connect_rkrlv2_ports,
    0,//activate
    run_mbvollv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor mutrolv2_descriptor={
    MUTROLV2_URI,
    init_mutrolv2,
    connect_rkrlv2_ports,
    0,//activate
    run_mutrolv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor echoverselv2_descriptor={
    ECHOVERSELV2_URI,
    init_echoverselv2,
    connect_rkrlv2_ports,
    0,//activate
    run_echoverselv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor coillv2_descriptor={
    COILLV2_URI,
    init_coillv2,
    connect_rkrlv2_ports,
    0,//activate
    run_coillv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor shelflv2_descriptor={
    SHELFLV2_URI,
    init_shelflv2,
    connect_rkrlv2_ports,
    0,//activate
    run_shelflv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor voclv2_descriptor={
    VOCODERLV2_URI,
    init_voclv2,
    connect_rkrlv2_ports,
    0,//activate
    run_voclv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor suslv2_descriptor={
    SUSTAINLV2_URI,
    init_suslv2,
    connect_rkrlv2_ports,
    0,//activate
    run_suslv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor seqlv2_descriptor={
    SEQUENCELV2_URI,
    init_seqlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_seqlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

static const LV2_Descriptor shiftlv2_descriptor={
    SHIFTERLV2_URI,
    init_shiftlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_shiftlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    switch (index) {
    case 0:
        return &eqlv2_descriptor ;
    case 1:
        return &complv2_descriptor ;
    case 2:
        return &distlv2_descriptor ;
    case 3:
        return &echolv2_descriptor ;
    case 4:
        return &choruslv2_descriptor ;
    case 5:
        return &aphaselv2_descriptor ;
    case 6:
        return &harmnomidlv2_descriptor ;
    case 7:
    	return &exciterlv2_descriptor ;
    case 8:
    	return &panlv2_descriptor ;
    case 9:
    	return &alienlv2_descriptor ;
    case 10:
    	return &revelv2_descriptor ;
    case 11:
    	return &eqplv2_descriptor ;
    case 12:
    	return &cablv2_descriptor ;
    case 13:
    	return &mdellv2_descriptor ;
    case 14:
    	return &wahlv2_descriptor ;
    case 15:
    	return &derelv2_descriptor ;
    case 16:
    	return &valvelv2_descriptor ;
    case 17:
    	return &dflangelv2_descriptor ;
    case 18:
    	return &ringlv2_descriptor ;
    case 19:
    	return &mbdistlv2_descriptor ;
    case 20:
    	return &arplv2_descriptor ;
    case 21:
    	return &expandlv2_descriptor ;
    case 22:
    	return &shuflv2_descriptor ;
    case 23:
    	return &synthlv2_descriptor ;
    case 24:
    	return &mbvollv2_descriptor ;
    case 25:
    	return &mutrolv2_descriptor ;
    case 26:
    	return &echoverselv2_descriptor ;
    case 27:
    	return &coillv2_descriptor ;
    case 28:
    	return &shelflv2_descriptor ;
    case 29:
    	return &voclv2_descriptor ;
    case 30:
    	return &suslv2_descriptor ;
    case 31:
    	return &seqlv2_descriptor ;
    case 32:
    	return &shiftlv2_descriptor ;
    default:
        return 0;
    }
}
