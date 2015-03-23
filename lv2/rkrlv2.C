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

//#include"global.h"

//this is the default hopefully hosts don't use periods of more than this, or they will communicate the max bufsize
#define INTERMEDIATE_BUFSIZE 1024


///////globals/////////
//int Pexitprogram, preset;
//int commandline, gui;
//int exitwithhelp, nojack;
//int maxx_len;
//int error_num;
//int period;
//int reconota;
//int needtoloadstate;
//int needtoloadbank;
//int stecla;
//int looper_lqua;
//unsigned int SAMPLE_RATE;
//int note_active[POLY];
//int rnote[POLY];
//int gate[POLY];
//int pdata[50];
//float val_sum;
//float fPERIOD;
//unsigned int SAMPLE_RATE;
//float fSAMPLE_RATE;
//float cSAMPLE_RATE;
//float r__ratio[12];
//int Wave_res_amount;
//int Wave_up_q;
//int Wave_down_q;
//Pixmap p, mask;
//XWMHints *hints;
//float freqs[12];
//float lfreqs[12];
//float aFreq;
//char *s_uuid;


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
}RKRLV2;


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
    val = (int)*plug->param_p[i];
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
    for(i++;i<plug->nparams-1;i++)
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
    for(i++;i<10;i++)// 8-9
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

    getFeatures(plug,host_features);

    plug->wah = new DynamicFilter(0,0,sample_freq, plug->period_max);

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

/////////////////////////////////
///////// END OF FX ///////////// 
/////////////////////////////////



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
    case DBG:         plug->dbg_p = (float*)data;break; 
    default:         puts("UNKNOWN PORT YO!!");
    }
}

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
        	//delete plug->noteID; //causes double free error
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
    }
    free(plug);
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

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    switch (index) {
    case 0:
        return &eqlv2_descriptor;
    case 1:
        return &complv2_descriptor;
    case 2:
        return &distlv2_descriptor;
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
    default:
        return 0;
    }
}
