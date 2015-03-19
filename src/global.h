/*
  rakarrack - a guitar efects software

  global.h  -  Variable Definitions and functions
  Copyright (C) 2008-2010 Josep Andreu
  Author: Josep Andreu & Ryan Billing

 This program is free software; you can redistribute it and/or modify
 it under the terms of version 2 of the GNU General Public License
 as published by the Free Software Foundation.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License (version 2) for more details.

 You should have received a copy of the GNU General Public License
 (version2)  along with this program; if not, write to the Free Software Foundation,
 Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/


#ifndef DXEMU_H
#define DXEMU_H


#define D_PI 6.283185f
#define PI 3.141598f
#define LOG_10 2.302585f
#define LOG_2  0.693147f
#define LN2R 1.442695041f
#define CNST_E  2.71828182845905f
#define AMPLITUDE_INTERPOLATION_THRESHOLD 0.0001f
#define FF_MAX_VOWELS 6
#define FF_MAX_FORMANTS 12
#define FF_MAX_SEQUENCE 8
#define MAX_FILTER_STAGES 5
#define RND (rand()/(RAND_MAX+1.0))
#define RND1 (((float) rand())/(((float) RAND_MAX)+1.0f))
#define F2I(f,i) (i)=((f>0) ? ( (int)(f) ) :( (int)(f-1.0f) ))
#define dB2rap(dB) (float)((expf((dB)*LOG_10/20.0f)))
#define rap2dB(rap) (float)((20*log(rap)/LOG_10))
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define INTERPOLATE_AMPLITUDE(a,b,x,size) ( (a) + ( (b) - (a) ) * (float)(x) / (float) (size) )
#define ABOVE_AMPLITUDE_THRESHOLD(a,b) ( ( 2.0f*fabs( (b) - (a) ) / ( fabs( (b) + (a) + 0.0000000001f) ) ) > AMPLITUDE_INTERPOLATION_THRESHOLD )
#define POLY 8
#define DENORMAL_GUARD 1e-18f	// Make it smaller until CPU problem re-appears
#define SwapFourBytes(data) ( (((data) >> 24) & 0x000000ff) | (((data) >> 8) & 0x0000ff00) | (((data) << 8) & 0x00ff0000) | (((data) << 24) & 0xff000000) )
#define D_NOTE          1.059463f
#define LOG_D_NOTE      0.057762f
#define D_NOTE_SQRT     1.029302f
#define MAX_PEAKS 8
#define MAX_ALIENWAH_DELAY 100
#define ATTACK  0.175f  //crossover time for reverse delay
#define MAX_DELAY 2	// Number of Seconds
#define MAXHARMS  8    // max number of harmonics available
#define MAX_PHASER_STAGES 12
#define MAX_CHORUS_DELAY 250.0f	//ms
#define LN2                       (1.0f)  //Uncomment for att/rel to behave more like a capacitor.
#define MUG_CORR_FACT  0.4f
//Crunch waveshaping constants
#define Thi		0.67f			//High threshold for limiting onset
#define Tlo		-0.65f			//Low threshold for limiting onset
#define Tlc		-0.6139445f		//Tlo + sqrt(Tlo/500)
#define Thc		0.6365834f	        //Thi - sqrt(Thi/600)
#define CRUNCH_GAIN	100.0f			//Typical voltage gain for most OD stompboxes
#define DIV_TLC_CONST   0.002f			// 1/300
#define DIV_THC_CONST	0.0016666f		// 1/600 (approximately)
//End waveshaping constants
#define D_FLANGE_MAX_DELAY	0.055f			// Number of Seconds  - 50ms corresponds to fdepth = 20 (Hz). Added some extra for padding
#define LFO_CONSTANT		9.765625e-04		// 1/(2^LOG_FMAX - 1)
#define LOG_FMAX		10.0f			//  -- This optimizes LFO sweep for useful range.
#define MINDEPTH		20.0f			// won't allow filter lower than 20Hz
#define MAXDEPTH		15000.0f		// Keeps delay greater than 2 samples at 44kHz SR
#define MAX_EQ_BANDS 16
#define CLOSED  1
#define OPENING 2
#define OPEN    3
#define CLOSING 4
#define ENV_TR 0.0001f
#define HARMONICS 11
#define REV_COMBS 8
#define REV_APS 4
#define MAX_SFILTER_STAGES 12

#define TEMPBUFSIZE 1024

typedef union {
    float f;
    long i;
} ls_pcast32;

/*
static inline float f_pow2(float x)
{
        ls_pcast32 *px, tx, lx;
        float dx;

        px = (ls_pcast32 *)&x; // store address of float as long pointer
        tx.f = (x-0.5f) + (3<<22); // temporary value for truncation
        lx.i = tx.i - 0x4b400000; // integer power of 2
        dx = x - (float)lx.i; // float remainder of power of 2

        x = 1.0f + dx * (0.6960656421638072f + // cubic apporoximation of 2^x
                   dx * (0.224494337302845f +  // for x in the range [0, 1]
                   dx * (0.07944023841053369f)));
        (*px).i += (lx.i << 23); // add integer power of 2 to exponent

        return (*px).f;
}
*/
/*
#define P2a0  1.00000534060469
#define P2a1   0.693057900547259
#define P2a2   0.239411678986933
#define P2a3   0.0532229404911678
#define P2a4   0.00686649174914722
#include <math.h>
static inline float f_pow2(float x)
{
float y,xx, intpow;
long xint = (int) fabs(ceil(x));
xx = x - ceil(x);
xint = xint<<xint;
if(x>0) intpow = (float) xint;
else intpow = 1.0f;

y = intpow*(xx*(xx*(xx*(xx*P2a4 + P2a3) + P2a2) + P2a1) + P2a0);

return y;

}
*/

//The below pow function really works & is good to 16 bits, but is it faster than math lib powf()???
//globals
#include <math.h>
static const float a[5] = { 1.00000534060469, 0.693057900547259, 0.239411678986933, 0.0532229404911678, 0.00686649174914722 };
//lookup for positive powers of 2
static const float pw2[25] = {1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f, 64.0f, 128.0f, 256.0f, 512.0f, 1024.0f, 2048.0f, 4096.0f, 8192.0f, 16384.0f, 32768.0f, 65536.0f, 131072.0f, 262144.0f, 524288.0f, 1048576.0f, 2097152.0f, 4194304.0f, 8388608.0f, 16777216.0f};
//negative powers of 2, notice ipw2[0] will never be indexed.
static const float ipw2[25] = {1.0, 5.0e-01, 2.5e-01, 1.25e-01, 6.25e-02, 3.125e-02, 1.5625e-02, 7.8125e-03, 3.90625e-03, 1.953125e-03, 9.765625e-04, 4.8828125e-04, 2.44140625e-04, 1.220703125e-04, 6.103515625e-05, 3.0517578125e-05, 1.52587890625e-05, 7.62939453125e-06, 3.814697265625e-06, 1.9073486328125e-06, 9.5367431640625e-07, 4.76837158203125e-07, 2.38418579101562e-07, 1.19209289550781e-07, 5.96046447753906e-08};

static inline float f_pow2(float x)
{
    float y = 0.0f;

    if(x >=24) return pw2[24];
    else if (x <= -24.0f) return ipw2[24];
    else {
        float whole =  ceilf(x);
        int xint = (int) whole;
        x = x - whole;

        if (xint>=0) {
            y = pw2[xint]*(x*(x*(x*(x*a[4] + a[3]) + a[2]) + a[1]) + a[0]);

        } else  {

            y = ipw2[-xint]*(x*(x*(x*(x*a[4] + a[3]) + a[2]) + a[1]) + a[0]);

        }

        return y;
    }

}

#define f_exp(x) f_pow2(x * LN2R)

//#include "config.h"
#include <signal.h>
#include <dirent.h>
#include <search.h>
#include <sys/time.h>
#ifdef NOTLV2
#include <alsa/asoundlib.h>
#include <X11/xpm.h>
#include <jack/jack.h>
#include <jack/midiport.h>
#include <FL/Fl_Preferences.H>
#else
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#endif
#include "FPreset.h"
#if(0)
#include "Reverb.h"
#include "Chorus.h"
#include "Echo.h"
#include "Phaser.h"
#include "Distorsion.h"
#include "EQ.h"
#include "Compressor.h"
#include "Alienwah.h"
#include "DynamicFilter.h"
#include "Pan.h"
#include "Harmonizer.h"
#include "MusicDelay.h"
#include "Gate.h"
#include "Tuner.h"
#include "MIDIConverter.h"
#include "RecognizeNote.h"
#include "RecChord.h"
#include "NewDist.h"
#include "APhaser.h"
#include "Valve.h"
#include "Dual_Flange.h"
#include "Ring.h"
#include "Exciter.h"
#include "MBDist.h"
#include "Arpie.h"
#include "Expander.h"
#include "Shuffle.h"
#include "Synthfilter.h"
#include "MBVvol.h"
#include "Convolotron.h"
#include "Resample.h"
#include "Looper.h"
#include "RyanWah.h"
#include "RBEcho.h"
#include "CoilCrafter.h"
#include "ShelfBoost.h"
#include "Vocoder.h"
#include "Sustainer.h"
#include "Sequence.h"
#include "Shifter.h"
#include "StompBox.h"
#include "Reverbtron.h"
#include "Echotron.h"
#include "StereoHarm.h"
#include "CompBand.h"
#include "Opticaltrem.h"
#include "Vibe.h"
#include "Infinity.h"
#include "beattracker.h"
#endif


#if(0)
extern int Pexitprogram, preset;
extern int commandline, gui;
extern int exitwithhelp, nojack;
extern int maxx_len;
extern int error_num;
//extern int period;
extern int reconota;
extern int needtoloadstate;
extern int needtoloadbank;
extern int stecla;
extern int looper_lqua;
//extern unsigned int SAMPLE_RATE;
extern int note_active[POLY];
extern int rnote[POLY];
extern int gate[POLY];
//extern int pdata[50];
extern float val_sum;
//extern float fPERIOD;
//extern unsigned int SAMPLE_RATE;
//extern float fSAMPLE_RATE;
//extern float cSAMPLE_RATE;
extern float r__ratio[12];
//extern int Wave_res_amount;
//extern int Wave_up_q;
//extern int Wave_down_q;
extern Pixmap p, mask;
extern XWMHints *hints;
extern float freqs[12];
extern float lfreqs[12];
extern float aFreq;
extern char *s_uuid;
extern char *statefile;
extern char *filetoload;
extern char *banktoload;
class RKR
{



public:

    RKR ();
    ~RKR ();

    void Alg (float *inl, float *inr,float *origl, float *origr ,void *);
    void Control_Gain (float *origl, float *origr);
    void Control_Volume (float *origl, float *origr);

    void Vol_Efx (int NumEffect, float volume);
    void Vol2_Efx ();
    void Vol3_Efx ();
    void cleanup_efx ();
    void midievents();
    void miramidi ();
    void calculavol (int i);
    void Bank_to_Preset (int Num);
    void Preset_to_Bank (int i);
    void Actualizar_Audio ();
    void loadfile (char *filename);
    void getbuf (char *buf, int j);
    void putbuf (char *buf, int j);
    void savefile (char *filename);
    void SaveIntPreset(int num, char *name);
    void DelIntPreset(int num, char *name);
    void MergeIntPreset(char *filename);
    void loadmiditable (char *filename);
    void savemiditable (char *filename);
    void loadskin (char *filename);
    void saveskin (char *filename);
    int loadbank (char *filename);
    void loadnames();
    int savebank (char *filename);
    void ConvertOldFile(char *filename);
    void ConvertReverbFile(char * filename);
    void dump_preset_names ();
    void New ();
    void New_Bank ();
    void Adjust_Upsample();
    void add_metro();
    void init_rkr ();
    int Message (int prio, const char *labelwin, const char *message_text);
    char *PrefNom (const char *dato);
    void EQ1_setpreset (int npreset);
    void EQ2_setpreset (int npreset);
    int Cabinet_setpreset (int npreset);
    void InitMIDI ();
    void ConnectMIDI ();
    void ActiveUn(int value);
    void ActOnOff();
    void jack_process_midievents (jack_midi_event_t *midievent);
    void process_midi_controller_events(int parameter, int value);
    int ret_Tempo(int value);
    int ret_LPF(int value);
    int ret_HPF(int value);
    void Conecta ();
    void disconectaaconnect ();
    void conectaaconnect ();
    int BigEndian();
    void fix_endianess();
    void copy_IO();
    void convert_IO();
    int CheckOldBank(char *filename);
    int Get_Bogomips();
    int checkonoff(int value);
    int TapTempo();
    void TapTempo_Timeout(int state);
    void Update_tempo();
    int checkforaux();
    void Error_Handle(int num);
    void update_freqs(float val);

    class FPreset *Fpre;
    class Reverb *efx_Rev;
    class Chorus *efx_Chorus;
    class Chorus *efx_Flanger;
    class Phaser *efx_Phaser;
    class Analog_Phaser *efx_APhaser;
    class EQ *efx_EQ1;
    class EQ *efx_EQ2;
    class Echo *efx_Echo;
    class Distorsion *efx_Distorsion;
    class Distorsion *efx_Overdrive;
    class Compressor *efx_Compressor;
    class DynamicFilter *efx_WhaWha;
    class Alienwah *efx_Alienwah;
    class EQ *efx_Cabinet;
    class Pan *efx_Pan;
    class Harmonizer *efx_Har;
    class MusicDelay *efx_MusDelay;
    class Gate *efx_Gate;
    class NewDist *efx_NewDist;
    class Tuner *efx_Tuner;
    class MIDIConverter *efx_MIDIConverter;
    class metronome *M_Metronome;
    class beattracker *beat;

    class Recognize *RecNote;
    class RecChord *RC;
    class Compressor *efx_FLimiter;
    class Valve *efx_Valve;
    class Dflange *efx_DFlange;
    class Ring *efx_Ring;
    class Exciter *efx_Exciter;
    class MBDist *efx_MBDist;
    class Arpie *efx_Arpie;
    class Expander *efx_Expander;
    class Synthfilter *efx_Synthfilter;
    class Shuffle *efx_Shuffle;
    class MBVvol *efx_MBVvol;
    class Convolotron *efx_Convol;
    class Resample *U_Resample;
    class Resample *D_Resample;
    class Resample *A_Resample;
    class AnalogFilter *DC_Offsetl;
    class AnalogFilter *DC_Offsetr;
    class Looper *efx_Looper;
    class RyanWah *efx_RyanWah;
    class RBEcho *efx_RBEcho;
    class CoilCrafter *efx_CoilCrafter;
    class ShelfBoost *efx_ShelfBoost;
    class Vocoder *efx_Vocoder;
    class Sustainer *efx_Sustainer;
    class Sequence *efx_Sequence;
    class Shifter *efx_Shifter;
    class StompBox *efx_StompBox;
    class Reverbtron *efx_Reverbtron;
    class Echotron *efx_Echotron;
    class StereoHarm *efx_StereoHarm;
    class CompBand *efx_CompBand;
    class Opticaltrem *efx_Opticaltrem;
    class Vibe *efx_Vibe;
    class Infinity *efx_Infinity;

    jack_client_t *jackclient;
    jack_options_t options;
    jack_status_t status;
    char jackcliname[64];

    int db6booster;
    int jdis;
    int jshut;
    int DC_Offset;
    int Bypass;
    int MIDIConverter_Bypass;
    int Metro_Bypass;
    int Tuner_Bypass;
    int Tap_Bypass;
    int ACI_Bypass;
    int Reverb_Bypass;
    int Chorus_Bypass;
    int Flanger_Bypass;
    int Phaser_Bypass;
    int Overdrive_Bypass;
    int Distorsion_Bypass;
    int Echo_Bypass;
    int EQ1_Bypass;
    int EQ2_Bypass;
    int Compressor_Bypass;
    int WhaWha_Bypass;
    int Alienwah_Bypass;
    int Cabinet_Bypass;
    int Pan_Bypass;
    int Harmonizer_Bypass;
    int MusDelay_Bypass;
    int Gate_Bypass;
    int NewDist_Bypass;
    int APhaser_Bypass;
    int Valve_Bypass;
    int DFlange_Bypass;
    int Ring_Bypass;
    int Exciter_Bypass;
    int MBDist_Bypass;
    int Arpie_Bypass;
    int Expander_Bypass;
    int Shuffle_Bypass;
    int Synthfilter_Bypass;
    int MBVvol_Bypass;
    int Convol_Bypass;
    int Looper_Bypass;
    int RyanWah_Bypass;
    int RBEcho_Bypass;
    int CoilCrafter_Bypass;
    int ShelfBoost_Bypass;
    int Vocoder_Bypass;
    int Sustainer_Bypass;
    int Sequence_Bypass;
    int Shifter_Bypass;
    int StompBox_Bypass;
    int Reverbtron_Bypass;
    int Echotron_Bypass;
    int StereoHarm_Bypass;
    int CompBand_Bypass;
    int Opticaltrem_Bypass;
    int Vibe_Bypass;
    int Infinity_Bypass;
    int Bypass_B;
    int Reverb_B;
    int Chorus_B;
    int Flanger_B;
    int Phaser_B;
    int APhaser_B;
    int DFlange_B;
    int Overdrive_B;
    int Distorsion_B;
    int Echo_B;
    int EQ1_B;
    int EQ2_B;
    int Compressor_B;
    int WhaWha_B;
    int Alienwah_B;
    int Cabinet_B;
    int Pan_B;
    int Harmonizer_B;
    int MusDelay_B;
    int Gate_B;
    int NewDist_B;
    int Valve_B;
    int Ring_B;
    int Exciter_B;
    int MBDist_B;
    int Arpie_B;
    int Expander_B;
    int Shuffle_B;
    int Synthfilter_B;
    int MBVvol_B;
    int Convol_B;
    int Looper_B;
    int RyanWah_B;
    int RBEcho_B;
    int CoilCrafter_B;
    int ShelfBoost_B;
    int Vocoder_B;
    int Sustainer_B;
    int Sequence_B;
    int Shifter_B;
    int StompBox_B;
    int Reverbtron_B;
    int Echotron_B;
    int StereoHarm_B;
    int CompBand_B;
    int Opticaltrem_B;
    int Vibe_B;
    int Infinity_B;

    int Cabinet_Preset;
    int Selected_Preset;
    int lv[70][20];
    int saved_order[16];
    int efx_order[16];
    int new_order[16];
    int availables[60];
    int active[12];
    int MidiCh;
    int HarCh;
    int init_state;
    int actuvol;
    int help_displayed;
    int modified;
    int autoassign;
    int comemouse;
    int aconnect_MI;
    int aconnect_JA;
    int aconnect_JIA;

    int cuan_jack;
    int cuan_ijack;
    int IsCoIn;
    int Cyoin;
    int Pyoin;
    int Ccin;
    int Pcin;

    // bank of flags telling GUI which midi controlled items to update
    int Mcontrol[500];
    // flag telling GUI that Mcontrol has at least one set flag
    int RControl;
    int ControlGet;
    int CountWait;
    int XUserMIDI[128][20];

    int eff_filter;
    int Har_Down;
    int Har_U_Q;
    int Har_D_Q;
    int Rev_Down;
    int Rev_U_Q;
    int Rev_D_Q;
    int Con_Down;
    int Con_U_Q;
    int Con_D_Q;
    int Shi_Down;
    int Shi_U_Q;
    int Shi_D_Q;
    int Seq_Down;
    int Seq_U_Q;
    int Seq_D_Q;
    int Voc_Down;
    int Voc_U_Q;
    int Voc_D_Q;
    int Ste_Down;
    int Ste_U_Q;
    int Ste_D_Q;

    int Metro_Vol;
    int M_Metro_Sound;
    int deachide;
    int font;
    int flpos;
    int upsample;
    int UpQual;
    int DownQual;
    int UpAmo;
    int J_SAMPLE_RATE;
    int J_PERIOD;
    int m_displayed;
    int Mvalue;
    int Mnumeff[32];
    int OnOffC;

    int sw_stat;
    int MIDIway;
    int NumParams;
    int NumEffects;
    int relfontsize;
    int resolution;
    int sh;
    int sschema;
    int slabel_color;
    int sfore_color;
    int sback_color;
    int sleds_color;

    int have_signal;
    int OnCounter;
    int t_periods;

    //   Recognize

    int last;

    // Harmonizer
    int HarQual;
    int SteQual;

    // Tap Tempo

    int tempocnt;
    int Tap_Display;
    int Tap_Selection;
    int Tap_TempoSet;
    int Tap_SetValue;
    int t_timeout;
    int jt_state;
    int Tap_Updated;
    int note_old;
    int cents_old;

    int cpufp;
    int mess_dis;
    int numpi, numpo, numpa, numpmi, numpmo;
    int numpc;
    int midi_table;
    int a_bank;
    int new_bank_loaded;

    int Aux_Gain;
    int Aux_Threshold;
    int Aux_MIDI;
    int Aux_Minimum;
    int Aux_Maximum;
    int Aux_Source;
    int last_auxvalue;
    int ena_tool;
    int VocBands;
    int RCOpti;

    int M_Metro_Tempo;
    int M_Metro_Bar;
    int mtc_counter;
    int EnableBackgroundImage;
    int ML_filter;
    int ML_clist[150];

    long Tap_time_Init;

    double Tap_timeB;
    double Tap_timeC;
    double jt_tempo;

    double tempobuf[6];


    double u_down;
    double u_up;

    timeval timeA;

    float booster;
    float cpuload;
    float rtrig;

    float *efxoutl;
    float *efxoutr;
    float *auxdata;
    float *auxresampled;
    float *anall;
    float *analr;
    float *smpl;
    float *smpr;
    float *denormal;
    float *m_ticks;

    float Master_Volume;
    float Input_Gain;
    float Fraction_Bypass;
    float Log_I_Gain;
    float Log_M_Volume;
    float M_Metro_Vol;


    float old_il_sum;
    float old_ir_sum;
    float old_vl_sum;
    float old_vr_sum;
    float val_vl_sum;
    float val_vr_sum;
    float val_il_sum;
    float val_ir_sum;
    float old_a_sum;
    float val_a_sum;


    float bogomips;
    float looper_size;

// Tunner

    float nfreq_old;
    float afreq_old;

    char tmpprefname[128];

    char *Preset_Name;
    char *Author;
    char *Bank_Saved;
    char *UserRealName;



    char MID[128];
    char BankFilename[128];
    char UDirFilename[128];
    char BackgroundImage[256];




    struct Effects_Names {
        char Nom[24];
        int Pos;
        int Type;

    } efx_names[70];

    struct Effects_Params {
        char Nom[32];
        int Ato;
        int Effect;
    } efx_params[500];


    struct Preset_Bank_Struct {
        char Preset_Name[64];
        char Author[64];
        char Classe[36];
        char Type[4];
        char ConvoFiname[128];
        char cInput_Gain[64];
        char cMaster_Volume[64];
        char cBalance[64];
        float Input_Gain;
        float Master_Volume;
        float Balance;
        int Bypass;
        char RevFiname[128];
        char EchoFiname[128];
        int lv[70][20];
        int XUserMIDI[128][20];
        int XMIDIrangeMin[128];
        int XMIDIrangeMax[128];
    } Bank[62];


    struct MIDI_table {
        int bank;
        int preset;
    } M_table[128];

    struct Bank_Names {
        char Preset_Name[64];
    } B_Names[4][62];


    // Alsa MIDI

    snd_seq_t *midi_in, *midi_out;


    struct JackPorts {
        char name[128];
    } jack_po[16],jack_poi[16];




};

struct list_element {
    struct list_element *forward;
    struct list_element *backward;
    char *name;
};
#endif

#endif


