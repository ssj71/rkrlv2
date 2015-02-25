/*
  rakarrack - a guitar effects software

 process.C  -  mainloop functions
  Copyright (C) 2008-2010 Josep Andreu
  Author: Josep Andreu

 This program is free software; you can redistribute it and/or modify
 it under the terms of version 2 of the GNU General Public License
 as published by the Free Software Foundation.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License (version 2) for more details.

 You should have received a copy of the GNU General Public License
 (version2)  along with this program; if not, write to the Free Software
 Foundation,
 Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <FL/Fl_Preferences.H>
#include "global.h"

int Pexitprogram, preset;
int commandline;
int exitwithhelp, gui, nojack;
int period;
int note_active[POLY];
int rnote[POLY];
int gate[POLY];
int reconota;
int maxx_len;
int error_num;
int stecla;
int looper_lqua;
int needtoloadstate;
int needtoloadbank;
unsigned int SAMPLE_RATE;
float fPERIOD;
float fSAMPLE_RATE;
float cSAMPLE_RATE;
//int Wave_res_amount;
//int Wave_up_q;
//int Wave_down_q;
int pdata[50];
float val_sum;
float r__ratio[12];
float freqs[12];
float lfreqs[12];
float aFreq;
char *s_uuid;
char *statefile;
char *filetoload;
char *banktoload;
Fl_Preferences rakarrack (Fl_Preferences::USER, WEBSITE, PACKAGE);
Pixmap p, mask;
XWMHints *hints;

RKR::RKR ()
{
    db6booster=0;
    jdis=0;
    jshut=0;
    char temp[256];
    ML_filter=0;
    error_num = 0;
    eff_filter = 0;
    OnOffC = 0;
    flpos = 0;
    Har_Down = 0;
    Rev_Down = 0;
    Con_Down = 0;
    Shi_Down = 0;
    Seq_Down = 0;
    cpufp = 0;
    numpc = 0;
    numpi = 0;
    numpo = 0;
    numpa = 0;
    numpmi = 0;
    numpmo = 0;
    mess_dis = 0;
    mtc_counter = 0;
    nojack = 0;
    memset (Mcontrol, 0, sizeof (Mcontrol));
    Mvalue = 0;
    actuvol= 0;
    OnCounter=0;


    sprintf (temp, "rakarrack");

#ifdef JACK_SESSION
    jackclient = jack_client_open (temp, JackSessionID, NULL, s_uuid);
#else
    jackclient = jack_client_open (temp, options, &status, NULL);
#endif


    if (jackclient == NULL) {
        fprintf (stderr, "Cannot make a jack client, is jackd running?\n");
        nojack = 1;
        exitwithhelp = 1;
        return;

    }

    strcpy (jackcliname, jack_get_client_name (jackclient));




    J_SAMPLE_RATE = jack_get_sample_rate (jackclient);
    J_PERIOD = jack_get_buffer_size (jackclient);

    rakarrack.get(PrefNom("Disable Warnings"),mess_dis,0);
    rakarrack.get (PrefNom ("Filter DC Offset"), DC_Offset, 0);
    rakarrack.get (PrefNom ("UpSampling"), upsample, 0);
    rakarrack.get (PrefNom ("UpQuality"), UpQual, 4);
    rakarrack.get (PrefNom ("DownQuality"), DownQual, 4);
    rakarrack.get (PrefNom ("UpAmount"), UpAmo, 0);


    Adjust_Upsample();

    rakarrack.get (PrefNom ("Looper Size"), looper_size, 1);
    rakarrack.get (PrefNom ("Calibration"), aFreq, 440.0f);
    update_freqs(aFreq);

    rakarrack.get (PrefNom ("Vocoder Bands"), VocBands, 32);
    rakarrack.get (PrefNom ("Recognize Trigger"), rtrig, .6f);


    Fraction_Bypass = 1.0f;
    Master_Volume = 0.50f;
    Input_Gain = 0.50f;
    Cabinet_Preset = 0;

    rakarrack.get (PrefNom("Harmonizer Downsample"),Har_Down,5);
    rakarrack.get (PrefNom("Harmonizer Up Quality"),Har_U_Q,4);
    rakarrack.get (PrefNom("Harmonizer Down Quality"),Har_D_Q,2);

    rakarrack.get (PrefNom("StereoHarm Downsample"),Ste_Down,5);
    rakarrack.get (PrefNom("StereoHarm Up Quality"),Ste_U_Q,4);
    rakarrack.get (PrefNom("StereoHarm Down Quality"),Ste_D_Q,2);

    rakarrack.get (PrefNom("Reverbtron Downsample"),Rev_Down,5);
    rakarrack.get (PrefNom("Reverbtron Up Quality"),Rev_U_Q,4);
    rakarrack.get (PrefNom("Reverbtron Down Quality"),Rev_D_Q,2);

    rakarrack.get (PrefNom("Convolotron Downsample"),Con_Down,6);
    rakarrack.get (PrefNom("Convolotron Up Quality"),Con_U_Q,4);
    rakarrack.get (PrefNom("Convolotron Down Quality"),Con_D_Q,2);

    rakarrack.get (PrefNom("Sequence Downsample"),Seq_Down,5);
    rakarrack.get (PrefNom("Sequence Up Quality"),Seq_U_Q,4);
    rakarrack.get (PrefNom("Sequence Down Quality"),Seq_D_Q,2);

    rakarrack.get (PrefNom("Shifter Downsample"),Shi_Down,5);
    rakarrack.get (PrefNom("Shifter Up Quality"),Shi_U_Q,4);
    rakarrack.get (PrefNom("Shifter Down Quality"),Shi_D_Q,2);

    rakarrack.get (PrefNom("Vocoder Downsample"),Voc_Down,5);
    rakarrack.get (PrefNom("Vocoder Up Quality"),Voc_U_Q,4);
    rakarrack.get (PrefNom("Vocoder Down Quality"),Voc_D_Q,2);


//    rakarrack.get (PrefNom("Waveshape Resampling"),Wave_res_amount,5);
//    rakarrack.get (PrefNom("Waveshape Up Quality"),Wave_up_q,4);
//    rakarrack.get (PrefNom("Waveshape Down Quality"),Wave_down_q,2);




    rakarrack.get (PrefNom ("Harmonizer Quality"), HarQual, 4);
    rakarrack.get (PrefNom ("StereoHarm Quality"), SteQual, 4);

    rakarrack.get (PrefNom ("Auto Connect Jack"), aconnect_JA, 1);
    rakarrack.get (PrefNom ("Auto Connect Jack In"), aconnect_JIA, 1);

    rakarrack.get (PrefNom ("Auto Connect Num"), cuan_jack, 2);
    rakarrack.get (PrefNom ("Auto Connect In Num"), cuan_ijack, 1);

    int i;
    memset (temp, 0, sizeof (temp));
    char j_names[128];

    static const char *jack_names[] =
    { "system:playback_1", "system:playback_2" };

    for (i = 0; i < cuan_jack; i++) {
        memset (temp, 0, sizeof (temp));
        sprintf (temp, "Jack Port %d", i + 1);
        if (i < 2)
            strcpy (j_names, jack_names[i]);
        else
            strcpy (j_names, "");
        rakarrack.get (PrefNom (temp), jack_po[i].name, j_names, 128);

    }

    memset(j_names,0, sizeof(j_names));

    static const char *jack_inames[] =
    { "system:capture_1", "system:capture_2" };

    for (i = 0; i < cuan_ijack; i++) {
        memset (temp, 0, sizeof (temp));
        sprintf (temp, "Jack Port In %d", i + 1);
        if (i < 1)
            strcpy (j_names, jack_inames[i]);
        else
            strcpy (j_names, "");
        rakarrack.get (PrefNom (temp), jack_poi[i].name, j_names, 128);
    }


    bogomips = 0.0f;
    i = Get_Bogomips();



    efxoutl = (float *) malloc (sizeof (float) * period);
    efxoutr = (float *) malloc (sizeof (float) * period);

    smpl = (float *) malloc (sizeof (float) * period);
    smpr = (float *) malloc (sizeof (float) * period);

    anall = (float *) malloc (sizeof (float) * period);
    analr = (float *) malloc (sizeof (float) * period);

    auxdata = (float *) malloc (sizeof (float) * period);
    auxresampled = (float *) malloc (sizeof (float) * period);

    m_ticks = (float *) malloc (sizeof (float) * period);



    Fpre = new FPreset();
    DC_Offsetl = new AnalogFilter (1, 20, 1, 0);
    DC_Offsetr = new AnalogFilter (1, 20, 1, 0);
    M_Metronome = new metronome();
    efx_Chorus = new Chorus (efxoutl, efxoutr);
    efx_Flanger = new Chorus (efxoutl, efxoutr);
    efx_Rev = new Reverb (efxoutl, efxoutr);
    efx_Echo = new Echo (efxoutl, efxoutr);
    efx_Phaser = new Phaser (efxoutl, efxoutr);
    efx_APhaser = new Analog_Phaser(efxoutl, efxoutr);
    efx_Distorsion = new Distorsion (efxoutl, efxoutr);
    efx_Overdrive = new Distorsion (efxoutl, efxoutr);
    efx_EQ2 = new EQ (efxoutl, efxoutr);
    efx_EQ1 = new EQ (efxoutl, efxoutr);
    efx_Compressor = new Compressor (efxoutl, efxoutr);
    efx_WhaWha = new DynamicFilter (efxoutl, efxoutr);
    efx_Alienwah = new Alienwah (efxoutl, efxoutr);
    efx_Cabinet = new EQ (efxoutl, efxoutr);
    efx_Pan = new Pan (efxoutl, efxoutr);
    efx_Har = new Harmonizer (efxoutl, efxoutr, (long) HarQual, Har_Down, Har_U_Q, Har_D_Q);
    efx_MusDelay = new MusicDelay (efxoutl, efxoutr);
    efx_Gate = new Gate (efxoutl, efxoutr);
    efx_NewDist = new NewDist(efxoutl, efxoutr);
    efx_FLimiter = new Compressor (efxoutl, efxoutr);
    efx_Valve = new Valve(efxoutl, efxoutr);
    efx_DFlange = new Dflange(efxoutl,efxoutr);
    efx_Ring = new Ring(efxoutl,efxoutr);
    efx_Exciter = new Exciter(efxoutl,efxoutr);
    efx_MBDist = new MBDist(efxoutl,efxoutr);
    efx_Arpie = new Arpie(efxoutl,efxoutr);
    efx_Expander = new Expander(efxoutl,efxoutr);
    efx_Shuffle = new Shuffle(efxoutl,efxoutr);
    efx_Synthfilter = new Synthfilter(efxoutl,efxoutr);
    efx_MBVvol = new MBVvol(efxoutl,efxoutr);
    efx_Convol = new Convolotron(efxoutl,efxoutr,Con_Down,Con_U_Q,Con_D_Q);
    efx_Looper = new Looper(efxoutl,efxoutr,looper_size);
    efx_RyanWah = new RyanWah(efxoutl,efxoutr);
    efx_RBEcho = new RBEcho(efxoutl,efxoutr);
    efx_CoilCrafter = new CoilCrafter(efxoutl,efxoutr);
    efx_ShelfBoost = new ShelfBoost(efxoutl,efxoutr);
    efx_Vocoder = new Vocoder(efxoutl,efxoutr,auxresampled,VocBands,Voc_Down, Voc_U_Q, Voc_D_Q);
    efx_Sustainer = new Sustainer(efxoutl,efxoutr);
    efx_Sequence = new Sequence(efxoutl,efxoutr, (long) HarQual, Seq_Down, Seq_U_Q, Seq_D_Q);
    efx_Shifter =  new Shifter(efxoutl,efxoutr, (long) HarQual, Shi_Down, Shi_U_Q, Shi_D_Q);
    efx_StompBox = new StompBox(efxoutl,efxoutr);
    efx_Reverbtron = new Reverbtron(efxoutl,efxoutr,Rev_Down, Rev_U_Q, Rev_D_Q);
    efx_Echotron = new Echotron(efxoutl,efxoutr);
    efx_StereoHarm = new StereoHarm(efxoutl, efxoutr, (long) SteQual, Ste_Down, Ste_U_Q, Ste_D_Q);
    efx_CompBand = new CompBand(efxoutl,efxoutr);
    efx_Opticaltrem = new Opticaltrem(efxoutl,efxoutr);
    efx_Vibe = new Vibe(efxoutl,efxoutr);
    efx_Infinity = new Infinity(efxoutl,efxoutr);

    U_Resample = new Resample(UpQual);
    D_Resample = new Resample(DownQual);
    A_Resample = new Resample(3);

    beat = new beattracker();
    efx_Tuner = new Tuner ();
    efx_MIDIConverter = new MIDIConverter(jackcliname);
    RecNote = new Recognize (efxoutl, efxoutr, rtrig);
    RC = new RecChord ();


    Preset_Name = (char *) malloc (sizeof (char) * 64);
    memset (Preset_Name, 0, sizeof (char) * 64);
    Author = (char *) malloc (sizeof (char) * 64);
    memset (Author, 0, sizeof (char) * 64);
    Bank_Saved = (char *) malloc (sizeof (char) * 128);
    memset (Bank_Saved, 0, sizeof (char) * 128);
    UserRealName = (char *) malloc (sizeof (char) * 128);
    memset (UserRealName, 0, sizeof (char) * 128);

// Names

    /*
    //Filter

    1   - Distortion
    2   - Modulation
    4   - Time
    8   - Emulation
    16  - Filters
    32  - Dynamics
    64  - Processing & EQ
    128 - Synthesis
    */

    NumEffects = 47;

    {
        static const char *los_names[] = {
            "AlienWah","11","16",
            "Analog Phaser","18","2",
            "Arpie","24","4",
            "Cabinet","12","8",
            "Chorus","5","2",
            "Coil Crafter","33","8",
            "CompBand","43","8",
            "Compressor","1","32",
            "Convolotron","29","8",
            "Derelict","17","1",
            "DistBand","23","1",
            "Distortion","2","1",
            "Dual Flange","20","2",
            "Echo","4","4",
            "Echotron","41","4",
            "Echoverse","32","4",
            "EQ","0","64",
            "Exciter","22","64",
            "Expander","25","32",
            "Flanger","7","2",
            "Harmonizer","14","128",
            "Infinity","46","16",
            "Looper","30","128",
            "MusicalDelay","15","4",
            "MuTroMojo","31","16",
            "NoiseGate","16","32",
            "Opticaltrem","44","2",
            "Overdrive","3","1",
            "Pan","13","64",
            "Parametric EQ","9","64",
            "Phaser","6","2",
            "Reverb","8","4",
            "Reverbtron","40","4",
            "Ring","21","128",
            "Sequence","37","128",
            "ShelfBoost","34","64",
            "Shifter","38","128",
            "Shuffle","26","64",
            "StereoHarm","42","128",
            "StompBox","39","9",
            "Sustainer","36","32",
            "Synthfilter","27","16",
            "Valve","19","9",
            "VaryBand","28","2",
            "Vibe","45","2",
            "Vocoder","35","128",
            "WahWah","10","16"

        };

        for (i = 0; i < NumEffects*3; i+=3) {
            strcpy (efx_names[i/3].Nom, los_names[i]);
            sscanf(los_names[i+1],"%d",&efx_names[i/3].Pos);
            sscanf(los_names[i+2],"%d",&efx_names[i/3].Type);


        }
    }


    NumParams= 377;

    {
        static const char *los_params[] = {

            "Alienwah Depth","20","11",
            "Alienwah Fb","82","11",
            "Alienwah Tempo","76","11",
            "Alienwah LR_Cr","96","11",
            "Alienwah Pan","61","11",
            "Alienwah Phase","115","11",
            "Alienwah Rnd.","109","11",
            "Alienwah St_df.","103","11",
            "Alienwah Wet/Dry","55","11",
            "Analog Phaser Depth","120","18",
            "Analog Phaser Distortion","118","18",
            "Analog Phaser Feedback","122","18",
            "Analog Phaser Tempo","119","18",
            "Analog Phaser Mismatch","123","18",
            "Analog Phaser St.df","124","18",
            "Analog Phaser Wet-Dry","117","18",
            "Analog Phaser Width","121","18",
            "Arpie Arpe's","213","24",
            "Arpie Damp","219","24",
            "Arpie Fb","218","24",
            "Arpie LR_Cr","217","24",
            "Arpie LRdl","216","24",
            "Arpie Pan","214","24",
            "Arpie Tempo","215","24",
            "Arpie WD","212","24",
            "AutoPan/Extra Stereo Tempo","77","13",
            "AutoPan/Extra Stereo Pan","67","13",
            "AutoPan/Extra Stereo Rnd","110","13",
            "AutoPan/Extra Stereo St_df.","104","13",
            "AutoPan/Extra Stereo Wet/Dry","58","13",
            "Balance","12","50",
            "Chorus Depth","23","5",
            "Chorus Fb","79","5",
            "Chorus Tempo","72","5",
            "Chorus LR_Cr","91","5",
            "Chorus Pan","50","5",
            "Chorus Rnd","105","5",
            "Chorus St_df","99","5",
            "Chorus Wet/Dry","52","5",
            "CoilCrafter Freq1","288","33",
            "CoilCrafter Freq2","290","33",
            "CoilCrafter Gain","286","33",
            "CoilCrafter Q1","289","33",
            "CoilCrafter Q2","291","33",
            "CoilCrafter Tone","287","33",
            "CompBand Cross 1","378","43",
            "CompBand Cross 2","379","43",
            "CompBand Cross 3","380","43",
            "CompBand Gain","369","43",
            "CompBand H Ratio","373","43",
            "CompBand H Thres","377","43",
            "CompBand L Ratio","370","43",
            "CompBand L Thres","374","43",
            "CompBand MH Ratio","372","43",
            "CompBand MH Thres","376","43",
            "CompBand ML Ratio","371","43",
            "CompBand ML Thres","375","43",
            "CompBand WD ","368","43",
            "Compressor A.Time","142","1",
            "Compressor Knee","145","1",
            "Compressor Output","147","1",
            "Compressor Ratio","144","1",
            "Compressor R.Time","143","1",
            "Compressor Threshold","146","1",
            "Convolotron Damp","283","29",
            "Convolotron Fb","284","29",
            "Convolotron Length","285","29",
            "Convolotron Level","282","29",
            "Convolotron Pan","281","29",
            "Convolotron WD","280","29",
            "Derelict Dist Color","6","17",
            "Derelict Dist Drive","2","17",
            "Derelict Dist HPF","5","17",
            "Derelict Dist Level","3","17",
            "Derelict Dist LPF","4","17",
            "Derelict Dist LR Cross","127","17",
            "Derelict Dist Pan","126","17",
            "Derelict Sub Octave","8","17",
            "Derelict Wet-Dry","125","17",
            "DistBand Cross1","209","23",
            "DistBand Cross2","210","23",
            "DistBand Drive","204","23",
            "DistBand H.Gain","208","23",
            "DistBand Level","205","23",
            "DistBand L.Gain","206","23",
            "DistBand LR_Cr","203","23",
            "DistBand M.Gain","207","23",
            "DistBand Pan","211","23",
            "DistBand WD","202","23",
            "Distortion Drive","69","2",
            "Distortion HPF","89","2",
            "Distortion Level","71","2",
            "Distortion LPF","86","2",
            "Distortion LR_Cr","95","2",
            "Distortion Pan","48","2",
            "Distortion Sub Octave","9","2",
            "Distortion Wet/Dry","30","2",
            "Dual Flange Depth","161","20",
            "Dual Flange FB","164","20",
            "Dual Flange LPF","165","20",
            "Dual Flange LR_Cr","160","20",
            "Dual Flange Offset","163","20",
            "Dual Flange Pan","159","20",
            "Dual Flange Rnd","168","20",
            "Dual Flange St_df.","167","20",
            "Dual Flange Tempo","166","20",
            "Dual Flange WD","158","20",
            "Dual Flange Width","162","20",
            "Echo Fb","78","4",
            "Echo LR_Cr","97","4",
            "Echo Pan","46","4",
            "Echo Wet/Dry","59","4",
            "Echotron WD","348","41",
            "Echotron Pan","349","41",
            "Echotron Tempo","350","41",
            "Echotron Damp","351","41",
            "Echotron Fb","352","41",
            "Echotron LR_Cr","353","41",
            "Echotron Width","354","41",
            "Echotron Depth","355","41",
            "Echotron St_df","356","41",
            "Echotron #","357","41",
            "Echoverse Angle","311","32",
            "Echoverse Damp","309","32",
            "Echoverse E.S.","310","32",
            "Echoverse Fb","308","32",
            "Echoverse LRdl","307","32",
            "Echoverse Pan","305","32",
            "Echoverse Reverse","304","32",
            "Echoverse Tempo","306","32",
            "Echoverse WD","303","32",
            "EQ 125 Hz","134","0",
            "EQ 16 Khz","141","0",
            "EQ 1 Khz","137","0",
            "EQ 250 Hz","135","0",
            "EQ 2 Khz","138","0",
            "EQ 31 Hz","132","0",
            "EQ 4 Khz","139","0",
            "EQ 500 Hz","136","0",
            "EQ 63 Hz","133","0",
            "EQ 8 Khz","140","0",
            "EQ Gain","130","0",
            "EQ Q","131","0",
            "Exciter Gain","189","22",
            "Exciter Har 10","201","22",
            "Exciter Har 1","192","22",
            "Exciter Har 2","193","22",
            "Exciter Har 3","194","22",
            "Exciter Har 4","195","22",
            "Exciter Har 5","196","22",
            "Exciter Har 6","197","22",
            "Exciter Har 7","198","22",
            "Exciter Har 8","199","22",
            "Exciter Har 9","200","22",
            "Exciter HPF","191","22",
            "Exciter LPF","190","22",
            "Expander A.Time","220","25",
            "Expander HPF","226","25",
            "Expander Level","224","25",
            "Expander LPF","225","25",
            "Expander R.Time","221","25",
            "Expander Shape","222","25",
            "Expander Threshold","223","25",
            "Flanger Depth","22","7",
            "Flanger Fb","80","7",
            "Flanger Tempo","73","7",
            "Flanger LR_Cr","92","7",
            "Flanger Pan","51","7",
            "Flanger Rnd","106","7",
            "Flanger St_df","100","7",
            "Flanger Wet/Dry","53","7",
            "Harmonizer Freq","26","14",
            "Harmonizer Interval","27","14",
            "Harmonizer Pan","49","14",
            "Harmonizer Wet/Dry","31","14",
            "Infinity WD","395","46",
            "Infinity Res","396","46",
            "Infinity AutoPan","397","46",
            "Infinity St_df","398","46",
            "Infinity Start","399","46",
            "Infinity End","400","46",
            "Infinity Tempo","401","46",
            "Infinity Subdiv","402","46",
            "Input","14","50",
            "Looper Auto Play"," 271","30",
            "Looper Clear","279","30",
            "Looper Level 1","268","30",
            "Looper Level 2","269","30",
            "Looper Play","272","30",
            "Looper R1","275","30",
            "Looper R2","276","30",
            "Looper Record","274","30",
            "Looper Reverse","270","30",
            "Looper Stop","273","30",
            "Looper Track 1","277","30",
            "Looper Track 2","278","30",
            "Looper WD","267","30",
            "Multi On/Off","116","50",
            "Musical Delay Fb 1","83","15",
            "Musical Delay Fb 2","84","15",
            "Musical Delay Gain 1","24","15",
            "Musical Delay Gain 2","25","15",
            "Musical Delay LR_Cr","98","15",
            "Musical Delay Pan 1","62","15",
            "Musical Delay Pan 2","65","15",
            "Musical Delay Wet/Dry","56","15",
            "MuTroMojo E. Sens","265","31",
            "MuTroMojo BP","258","31",
            "MuTroMojo HP","259","31",
            "MuTroMojo LP","257","31",
            "MuTroMojo Range","263","31",
            "MuTroMojo Res","262","31",
            "MuTroMojo Smooth","266","31",
            "MuTroMojo Tempo","261","31",
            "MuTroMojo Wah","264","31",
            "MuTroMojo WD","256","31",
            "MuTroMojo Width","260","31",
            "Opticaltrem Depth","381","44",
            "Opticaltrem Pan","385","44",
            "Opticaltrem Rnd","383","44",
            "Opticaltrem St.df","384","44",
            "Opticaltrem Tempo","382","44",
            "Overdrive Drive","68","3",
            "Overdrive Level","70","3",
            "Overdrive LPF","85","3",
            "Overdrive HPF","88","3",
            "Overdrive LR_Cr","94","3",
            "Overdrive Pan","47","3",
            "Overdrive Wet/Dry","29","3",
            "P.EQ Gain","148","9",
            "P.EQ High Freq","155","9",
            "P.EQ High Gain","156","9",
            "P.EQ High Q","157","9",
            "P.EQ Low Freq","149","9",
            "P.EQ Low Gain","150","9",
            "P.EQ Low Q","151","9",
            "P.EQ Mid Freq","152","9",
            "P.EQ Mid Gain","153","9",
            "P.EQ Mid Q","154","9",
            "Phaser Depth","21","6",
            "Phaser Fb","81","6",
            "Phaser Tempo","74","6",
            "Phaser LR_Cr","93","6",
            "Phaser Pan","60","6",
            "Phaser Phase","114","6",
            "Phaser Rnd","107","6",
            "Phaser St_df","101","6",
            "Phaser Wet/Dry","54","6",
            "Reverb HPF","90","8",
            "Reverb LPF","87","8",
            "Reverb Pan","63","8",
            "Reverb Wet/Dry","57","8",
            "Reverbtron WD","339","40",
            "Reverbtron Pan","340","40",
            "Reverbtron Level","341","40",
            "Reverbtron Damp","342","40",
            "Reverbtron Fb","343","40",
            "Reverbtron Length","344","40",
            "Reverbtron Stretch","345","40",
            "Reverbtron I.Del","346","40",
            "Reverbtron Fade","347","40",
            "Ring Depth","183","21",
            "Ring Freq","184","21",
            "Ring Input","180","21",
            "Ring Level","181","21",
            "Ring LR_Cr","179","21",
            "Ring Pan","182","21",
            "Ring Saw","187","21",
            "Ring Sin","185","21",
            "Ring Squ","188","21",
            "Ring Tri","186","21",
            "Ring WD","178","21",
            "Sequence WD","314","37",
            "Sequence 1","315","37",
            "Sequence 2","316","37",
            "Sequence 3","317","37",
            "Sequence 4","318","37",
            "Sequence 5","319","37",
            "Sequence 6","320","37",
            "Sequence 7","321","37",
            "Sequence 8","322","37",
            "Sequence Tempo","323","37",
            "Sequence Q","324","37",
            "Sequence St.df","325","37",
            "ShelfBoost Gain","292","34",
            "ShelfBoost Level","293","34",
            "ShelfBoost Pres","295","34",
            "ShelfBoost Tone","294","34",
            "Shifter WD","326","38",
            "Shifter Int","327","38",
            "Shifter Gain","328","38",
            "Shifter Pan","329","38",
            "Shifter Attack","330","38",
            "Shifter Decay","331","38",
            "Shifter Thrshold","332","38",
            "Shifter Whamy","333","38",
            "Shuffle High Freq","234","26",
            "Shuffle High Gain","235","26",
            "Shuffle Low Freq","228","26",
            "Shuffle Low Gain","229","26",
            "Shuffle M.H. Freq","232","26",
            "Shuffle M.H. Gain","233","26",
            "Shuffle M.L. Freq","230","26",
            "Shuffle M.L. Gain","231","26",
            "Shuffle Q","236","26",
            "Shuffle WD","227","26",
            "StereoHarm Chord","367","42",
            "StereoHarm Chrm L","360","42",
            "StereoHarm Chrm R","363","42",
            "StereoHarm Gain L","361","42",
            "StereoHarm Gain R","364","42",
            "StereoHarm Int L","359","42",
            "StereoHarm Int R","362","42",
            "StereoHarm LR_Cr","365","42",
            "StereoHarm Note","366","42",
            "StereoHarm WD","358","42",
            "StompBox Level","334","39",
            "StompBox Gain","335","39",
            "StompBox Low","336","39",
            "StompBox Mid","337","39",
            "StompBox High","338","39",
            "Sustainer Gain","312","36",
            "Sustainer Sustain","313","36",
            "Synthfilter A.Time","245","27",
            "Synthfilter Depth","243","27",
            "Synthfilter Distort","238","27",
            "Synthfilter E.Sens","244","27",
            "Synthfilter Fb","242","27",
            "Synthfilter Offset","247","27",
            "Synthfilter R.Time","246","27",
            "Synthfilter St.df","240","27",
            "Synthfilter Tempo","239","27",
            "Synthfilter WD","237","27",
            "Synthfilter Width","241","27",
            "Valve Dist","174","19",
            "Valve Drive","173","19",
            "Valve HPF","177","19",
            "Valve Level","172","19",
            "Valve LPF","176","19",
            "Valve LR_Cr","170","19",
            "Valve Pan","171","19",
            "Valve Presence","175","19",
            "Valve WD","169","19",
            "VaryBand Cross1","253","28",
            "VaryBand Cross2","254","28",
            "VaryBand Cross3","255","28",
            "VaryBand St.df 1","250","28",
            "VaryBand St.df 2","252","28",
            "VaryBand Tempo 1","249","28",
            "VaryBand Tempo 2","251","28",
            "VaryBand WD","248","28",
            "Vibe Depth","388","45",
            "Vibe Fb","392","45",
            "Vibe LR_Cr","393","45",
            "Vibe Pan","394","45",
            "Vibe Rnd","390","45",
            "Vibe St_df","391","45",
            "Vibe Tempo","389","45",
            "Vibe WD","386","45",
            "Vibe Width","387","45",
            "Vocoder Input","298","35",
            "Vocoder Level","302","35",
            "Vocoder Muf.","299","35",
            "Vocoder Pan","297","35",
            "Vocoder Q","300","35",
            "Vocoder Ring","301","35",
            "Vocoder WD","296","35",
            "Volume","7","50",
            "WahWah Amp S.","111","10",
            "WahWah Amp S.I.","112","10",
            "WahWah Depth","1","10",
            "WahWah Tempo","75","10",
            "WahWah Pan","66","10",
            "WahWah Rnd.","108","10",
            "WahWah Smooth","113","10",
            "WahWah St_df.","102","10",
            "WahWah WD","28","10"
        };
        for(i=0; i<NumParams; i++) {
            strcpy (efx_params[i].Nom, los_params[i*3]);
            sscanf(los_params[i*3+1],"%d",&efx_params[i].Ato);
            sscanf(los_params[i*3+2],"%d",&efx_params[i].Effect);

        }
    }

// Init Preset

    New ();

// Init Bank

    New_Bank ();
    init_rkr ();

}



RKR::~RKR ()
{
};




void
RKR::init_rkr ()
{

    Tuner_Bypass = 0;
    MIDIConverter_Bypass = 0;
    Metro_Bypass = 0;

    for (int i = 0; i <= 45; i += 5) {
        efx_EQ1->changepar (i + 10, 7);
        efx_EQ1->changepar (i + 14, 0);
    }

    efx_EQ1->changepar (11, 31);
    efx_EQ1->changepar (16, 63);
    efx_EQ1->changepar (21, 125);
    efx_EQ1->changepar (26, 250);
    efx_EQ1->changepar (31, 500);
    efx_EQ1->changepar (36, 1000);
    efx_EQ1->changepar (41, 2000);
    efx_EQ1->changepar (46, 4000);
    efx_EQ1->changepar (51, 8000);
    efx_EQ1->changepar (56, 16000);

    for (int i = 0; i <= 10; i += 5) {
        efx_EQ2->changepar (i + 10, 7);
        efx_EQ2->changepar (i + 13, 64);
        efx_EQ2->changepar (i + 14, 0);

    }


    efx_FLimiter->Compressor_Change_Preset(0,3);


    old_il_sum = -0.0f;
    old_ir_sum = -0.0f;

    old_vl_sum = -0.0f;
    old_vr_sum = -0.0f;

    old_a_sum = -0.0f;
    val_a_sum = -0.0f;

    val_il_sum = -0.0f;
    val_ir_sum = -0.0f;

    val_vl_sum = -0.0f;
    val_vr_sum = -0.0f;

    last_auxvalue = 0;
    note_old = 0;
    nfreq_old = 0;
    afreq_old = 0;
    cents_old = 0;

    MidiCh = 0;
    RControl = 0;
    ControlGet = 0;

    help_displayed = 0;
    modified = 0;


    tempocnt=0;
    for(int i=0; i<6; i++)tempobuf[i]=0;
    Tap_timeB = 0;
    Tap_Display = 0;
    Tap_Selection = 0;
    Tap_TempoSet = 90;

// Load Preset Bank File

    char temp[128];
    memset (temp, 0, sizeof (temp));
    sprintf (temp, "%s/Default.rkrb", DATADIR);
    rakarrack.get (PrefNom ("Bank Filename"), BankFilename, temp, 127);
    loadnames();

    if (commandline == 0) {
        loadbank (BankFilename);
        a_bank=3;

    }
    RC->cleanup ();
    reconota = -1;

}


void
RKR::Adjust_Upsample()
{

    if(upsample) {
        SAMPLE_RATE = J_SAMPLE_RATE*(UpAmo+2);
        period = J_PERIOD*(UpAmo+2);
        u_up = (double)UpAmo+2.0;
        u_down = 1.0 / u_up;


    } else {
        SAMPLE_RATE = J_SAMPLE_RATE;
        period = J_PERIOD;
    }

    fSAMPLE_RATE = (float) SAMPLE_RATE;
    cSAMPLE_RATE = 1.0f / (float)SAMPLE_RATE;
    fPERIOD= float(period);
    t_periods = J_SAMPLE_RATE / 12 / J_PERIOD;

}





void
RKR::ConnectMIDI ()
{

// Get config settings and init settings
// Get MIDI IN Setting

    rakarrack.get (PrefNom ("Auto Connect MIDI IN"), aconnect_MI, 0);
    rakarrack.get (PrefNom ("MIDI IN Device"), MID, "", 40);
    if (aconnect_MI)
        Conecta ();


}

int
RKR::Cabinet_setpreset (int npreset)
{

    const int PRESET_SIZE = 81;
    const int NUM_PRESETS = 11;
    int presets[NUM_PRESETS][PRESET_SIZE] = {

        //Marshall-4-12
        {
            2, 1135, 0, 0, 0, 8, 116, 87, 71, 0, 7, 128, 77, 53, 0, 7, 825, 84, 64, 0,
            7, 1021, 49, 19, 0, 7, 2657, 75, 95, 0, 7, 3116, 72, 86, 0, 7, 10580, 31, 64, 0,
            7, 17068, 28, 68, 0, 3, 17068, 0, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0,
            67
        },


        //Celestion G12M
        {
            2, 704, 64, 64, 0, 7, 64, 90, 111, 0, 7, 151, 69, 59, 0, 7, 1021, 56, 25, 0,
            7, 1562, 56, 62, 0, 7, 2389, 71, 77, 0, 7, 5896, 53, 69, 0, 7, 6916, 59, 83, 0,
            7, 7691, 52, 102, 0, 7, 9021, 43, 64, 0, 7, 15347, 32, 59, 0, 3, 17068, 1, 58, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 67
        },


        //Jensen Alnico P12-N
        {
            2, 270, 0, 0, 0, 7, 414, 46, 56, 0, 7, 1647, 46, 84, 0, 7, 7293, 53, 69, 0,
            7, 10032, 43, 109, 0, 7, 12408, 41, 86, 0, 7, 14553, 41, 90, 0, 7, 17068, 35, 27, 0,
            3, 17068, 64, 61, 0, 7, 4065, 71, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 67
        },

        //Jensen Alnico P15-N
        {
            0, 600, 64, 64, 0, 2, 256, 64, 64, 0, 7, 414, 49, 43, 0, 7, 1832, 68, 58, 0,
            7, 4065, 46, 92, 0, 7, 8111, 52, 75, 0, 7, 11766, 43, 83, 0, 3, 15347, 64, 41, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 67
        },

        //Eminence Delta Demon
        {
            2, 68, 64, 64, 0, 7, 600, 55, 43, 0, 7, 1021, 72, 72, 0, 7, 1562, 68, 92, 0,
            7, 2519, 66, 80, 0, 7, 6218, 52, 66, 0, 7, 9513, 43, 81, 0, 3, 10580, 64, 66, 0,
            7, 12408, 58, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 67
        },

        //Celestion EVH12
        {
            2, 151, 64, 64, 0, 7, 1481, 56, 64, 0, 7, 2519, 66, 87, 0, 7, 3116, 66, 92, 0,
            7, 7293, 53, 87, 0, 7, 8554, 52, 87, 0, 7, 11157, 46, 59, 0, 3, 17068, 61, 30, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 67
        },

        //Eminence Copperhead

        {
            2, 301, 64, 64, 0, 7, 301, 58, 77, 0, 7, 1077, 68, 74, 0, 7, 2519, 71, 77, 0,
            7, 6558, 55, 87, 0, 7, 7293, 58, 37, 0, 7, 15347, 13, 75, 0, 3, 17068, 50, 43, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 67
        },

        //Mesa Boogie
        {
            2, 600, 64, 64, 0, 7, 128, 93, 52, 0, 7, 633, 58, 80, 0, 7, 1077, 52, 89, 0,
            7, 1647, 66, 89, 0, 7, 2037, 75, 86, 0, 7, 3466, 75, 90, 0, 7, 6218, 62, 52, 0,
            7, 11157, 58, 71, 0, 3, 1404, 64, 47, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 67
        },

        //Jazz-Chorus
        {
            4, 72, 0, 40, 0, 7, 72, 50, 43, 0, 7, 667, 38, 89, 0, 7, 3466, 77, 112, 0,
            7, 7293, 46, 93, 0, 7, 8111, 43, 105, 0, 7, 17068, 15, 40, 0, 3, 17068, 58, 55, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 67
        },

        //Vox-Bright
        {
            2, 1021, 64, 64, 0, 7, 243, 68, 72, 0, 7, 2657, 75, 41, 0, 7, 3466, 69, 96, 0,
            7, 4767, 74, 74, 0, 7, 6218, 44, 81, 0, 7, 8554, 52, 100, 0, 7, 13086, 46, 72, 0,
            7, 15347, 52, 62, 0, 3, 13800, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 67
        },

        //Marshall-I
        {
            2, 4287, 53, 64, 0, 7, 122, 80, 25, 0, 7, 633, 69, 86, 0, 7, 1021, 78, 59, 0,
            7, 1647, 75, 64, 0, 7, 2389, 86, 78, 0, 7, 3286, 95, 61, 0, 7, 6916, 61, 59, 0,
            7, 8554, 56, 84, 0, 7, 12408, 22, 18, 0, 3, 10032, 64, 61, 0, 0, 64, 64, 64, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 67
        }


    };


    if (npreset > (NUM_PRESETS -1))
        npreset = 0;
    for (int n = 0; n < 16; n++) {
        efx_Cabinet->changepar (n * 5 + 10, presets[npreset][n * 5]);
        efx_Cabinet->changepar (n * 5 + 11, presets[npreset][n * 5 + 1]);
        efx_Cabinet->changepar (n * 5 + 12, presets[npreset][n * 5 + 2]);
        efx_Cabinet->changepar (n * 5 + 13, presets[npreset][n * 5 + 3]);
        efx_Cabinet->changepar (n * 5 + 14, presets[npreset][n * 5 + 4]);

    }

    Cabinet_Preset = npreset;

    return (0);


};





void
RKR::EQ1_setpreset (int npreset)
{

    const int PRESET_SIZE = 12;
    const int NUM_PRESETS = 3;
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Plain
        {64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64},
        //Pop
        {78, 71, 64, 64, 64, 64, 64, 64, 71, 80, 64, 42},
        //Jazz
        {71, 68, 64, 64, 64, 64, 64, 64, 66, 69, 64, 40}
    };

    if (npreset >= NUM_PRESETS) {
        Fpre->ReadPreset(0,npreset-NUM_PRESETS+1);
        for (int n = 0; n < 10; n++)
            efx_EQ1->changepar (n * 5 + 12, pdata[n]);
        efx_EQ1->changepar (0, pdata[10]);
        for (int n = 0; n < 10; n++)
            efx_EQ1->changepar (n * 5 + 13, pdata[11]);
    } else {
        for (int n = 0; n < 10; n++)
            efx_EQ1->changepar (n * 5 + 12, presets[npreset][n]);
        efx_EQ1->changepar (0, presets[npreset][10]);
        for (int n = 0; n < 10; n++)
            efx_EQ1->changepar (n * 5 + 13, presets[npreset][11]);
    }
};



void
RKR::EQ2_setpreset (int npreset)
{


    const int PRESET_SIZE = 10;
    const int NUM_PRESETS = 3;
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Plain
        {72, 64, 64, 1077, 64, 64, 8111, 64, 64, 64},
        //Pop
        {72, 73, 45, 1077, 64, 64, 8111, 69, 38, 64},
        //Jazz
        {72, 71, 38, 1077, 64, 64, 10580, 69, 38, 64}
    };


    if (npreset >= NUM_PRESETS) {

        Fpre->ReadPreset(9,npreset-NUM_PRESETS+1);
        for (int n = 0; n < 3; n++) {
            efx_EQ2->changepar (n * 5 + 11, pdata[n * 3]);
            efx_EQ2->changepar (n * 5 + 12, pdata[n * 3 + 1]);
            efx_EQ2->changepar (n * 5 + 13, pdata[n * 3 + 2]);
        }
        efx_EQ2->changepar (0, pdata[9]);
    }

    else {
        for (int n = 0; n < 3; n++) {
            efx_EQ2->changepar (n * 5 + 11, presets[npreset][n * 3]);
            efx_EQ2->changepar (n * 5 + 12, presets[npreset][n * 3 + 1]);
            efx_EQ2->changepar (n * 5 + 13, presets[npreset][n * 3 + 2]);
        }
        efx_EQ2->changepar (0, presets[npreset][9]);
    }
};


void
RKR::add_metro()
{
    for(int i=0; i<period; i++) {

        efxoutl[i] +=m_ticks[i]*M_Metro_Vol;
        efxoutr[i] +=m_ticks[i]*M_Metro_Vol;

    }

}

void
RKR::Vol2_Efx ()
{
    memcpy(smpl,efxoutl, period * sizeof(float));
    memcpy(smpr,efxoutr, period * sizeof(float));
}


void
RKR::Vol3_Efx ()
{
    int i;
    float att=2.0f;

    for (i = 0; i < period; i++) {
        efxoutl[i] *= att;
        efxoutr[i] *= att;
    }

    Vol2_Efx();

}


void
RKR::Vol_Efx (int NumEffect, float volume)
{
    int i;
    float v1, v2;

    if (volume < 0.5f) {
        v1 = 1.0f;
        v2 = volume * 2.0f;
    } else {
        v1 = (1.0f - volume) * 2.0f;
        v2 = 1.0f;
    };


    if ((NumEffect == 8) || (NumEffect == 15))
        v2 *= v2;

    for (i = 0; i < period; i++) {
        efxoutl[i] = smpl[i] * v2 + efxoutl[i] * v1;
        efxoutr[i] = smpr[i] * v2 + efxoutr[i] * v1;
    };

    Vol2_Efx();

}


void
RKR::calculavol (int i)
{

    if (i == 1)
        Log_I_Gain = powf (Input_Gain * 2.0f, 4);
    if (i == 2)
        Log_M_Volume = powf (Master_Volume * 2.0f, 4);

}

int
RKR::checkforaux()
{
    int i;

    for(i=0; i<10; i++)
        if(efx_order[i]==35) {
            if (Vocoder_Bypass) return(1);
        }

    return(0);

}
void
RKR::Control_Gain (float *origl, float *origr)
{

    int i;
    float il_sum = 1e-12f;
    float ir_sum = 1e-12f;

    float a_sum = 1e-12f;

    float temp_sum;

    float tmp;







    if(upsample) {
        U_Resample->out(origl,origr,efxoutl,efxoutr,J_PERIOD,u_up);
        if((checkforaux()) || (ACI_Bypass)) A_Resample->mono_out(auxdata,auxresampled,J_PERIOD,u_up,period);
    } else if((checkforaux()) || (ACI_Bypass)) memcpy(auxresampled,auxdata,sizeof(float)*J_PERIOD);

    if(DC_Offset) {
        DC_Offsetl->filterout(efxoutl);
        DC_Offsetr->filterout(efxoutr);
    }



    for (i = 0; i <= period; i++) {
        efxoutl[i] *= Log_I_Gain;
        efxoutr[i] *= Log_I_Gain;
        tmp = fabsf(efxoutr[i]);
        if (tmp > ir_sum) ir_sum = tmp;
        tmp = fabsf(efxoutl[i]);
        if (tmp > il_sum) il_sum = tmp;


    }
    memcpy(smpl,efxoutl,sizeof(float)*period);
    memcpy(smpr,efxoutr,sizeof(float)*period);

    temp_sum = (float)CLAMP (rap2dB (il_sum), -48.0, 15.0);
    val_il_sum = .6f * old_il_sum + .4f * temp_sum;

    temp_sum = (float)CLAMP (rap2dB (ir_sum), -48.0, 15.0);
    val_ir_sum = .6f * old_ir_sum + .4f * temp_sum;

    val_sum = val_il_sum + val_ir_sum;


    if((ACI_Bypass) && (Aux_Source==0)) {
        temp_sum = 0.0;
        tmp = 0.0;
        for (i = 0; i <= period; i++) {
            tmp = fabsf(auxresampled[i]);
            if (tmp > a_sum) a_sum = tmp;
        }

        val_a_sum = .6f * old_a_sum + .4f * a_sum;
        old_a_sum = val_a_sum;
    }






}


void
RKR::Control_Volume (float *origl,float *origr)
{
    int i;
    float il_sum = 1e-12f;
    float ir_sum = 1e-12f;

    float temp_sum;
    float tmp;
    float Temp_M_Volume = 0.0f;

    if((flpos)&&(have_signal)) {
        if(db6booster) {
            for(i=0; i<period; i++) {
                efxoutl[i] *=.5f;
                efxoutr[i] *=.5f;
            }
        }

        efx_FLimiter->out(efxoutl, efxoutr);

        if(db6booster) {
            for(i=0; i<period; i++) {
                efxoutl[i] *=2.0f;
                efxoutr[i] *=2.0f;
            }
        }


    }

    memcpy(anall, efxoutl, sizeof(float)* period);
    memcpy(analr, efxoutr, sizeof(float)* period);



    if(upsample)
        D_Resample->out(anall,analr,efxoutl,efxoutr,period,u_down);


    if (OnCounter < t_periods) {
        Temp_M_Volume = Log_M_Volume / (float) (t_periods - OnCounter);
        OnCounter++;
    }

    else Temp_M_Volume = Log_M_Volume;

    for (i = 0; i <= period; i++) { //control volume

        efxoutl[i] *= Temp_M_Volume*booster;
        efxoutr[i] *= Temp_M_Volume*booster;


        if (Fraction_Bypass < 1.0f) {
            efxoutl[i]= (origl[i] * (1.0f - Fraction_Bypass) + efxoutl[i] * Fraction_Bypass);
            efxoutr[i]= (origr[i] * (1.0f - Fraction_Bypass) + efxoutr[i] * Fraction_Bypass);
        }

        tmp = fabsf (efxoutl[i]);
        if (tmp > il_sum) il_sum = tmp;
        tmp = fabsf (efxoutr[i]);
        if (tmp > ir_sum) ir_sum = tmp;

    }

    if ((!flpos) && (have_signal)) {
        if(db6booster) {
            for(i=0; i<period; i++) {
                efxoutl[i] *=.5f;
                efxoutr[i] *=.5f;
            }
        }

        efx_FLimiter->out(efxoutl, efxoutr);  //then limit final output

        if(db6booster) {
            for(i=0; i<period; i++) {
                efxoutl[i] *=2.0f;
                efxoutr[i] *=2.0f;
            }
        }


    }


    for (i = 0; i <= period; i++) {

        tmp = fabsf (efxoutl[i]);
        if (tmp > il_sum) il_sum = tmp;
        tmp = fabsf (efxoutr[i]);
        if (tmp > ir_sum) ir_sum = tmp;
    }

    temp_sum = (float) CLAMP(rap2dB (il_sum), -48, 15);
    val_vl_sum = .6f * old_vl_sum + .4f * temp_sum;
    temp_sum = (float) CLAMP(rap2dB (ir_sum), -48, 15);
    val_vr_sum = .6f * old_vr_sum + .4f * temp_sum;

    if ((il_sum+ir_sum) > 0.0004999f)  have_signal = 1;
    else  have_signal = 0;


}



void
RKR::cleanup_efx ()
{


    efx_EQ1->cleanup ();
    efx_Rev->cleanup ();
    efx_Distorsion->cleanup ();
    efx_Overdrive->cleanup ();
    efx_Compressor->cleanup ();
    efx_Echo->cleanup ();
    efx_Chorus->cleanup ();
    efx_Flanger->cleanup ();
    efx_Phaser->cleanup ();
    efx_EQ2->cleanup ();
    efx_WhaWha->cleanup ();
    efx_Alienwah->cleanup ();
    efx_Cabinet->cleanup ();
    efx_Pan->cleanup ();
    efx_Har->cleanup ();
    efx_MusDelay->cleanup ();
    efx_Gate->cleanup ();
    efx_NewDist->cleanup();
    efx_APhaser->cleanup();
    efx_Valve->cleanup();
    efx_DFlange->cleanup();
    efx_Ring->cleanup();
    efx_Exciter->cleanup();
    efx_MBDist->cleanup();
    efx_Arpie->cleanup();
    efx_Expander->cleanup();
    efx_Shuffle->cleanup();
    efx_Synthfilter->cleanup();
    efx_MBVvol->cleanup();
    efx_Convol->cleanup();
    efx_Looper->cleanup();
    efx_RyanWah->cleanup();
    efx_RBEcho->cleanup();
    efx_CoilCrafter->cleanup();
    efx_ShelfBoost->cleanup();
    efx_Vocoder->cleanup();
    efx_Sustainer->cleanup();
    efx_Sequence->cleanup();
    efx_Shifter->cleanup();
    efx_StompBox->cleanup();
    efx_Reverbtron->cleanup();
    efx_Echotron->cleanup();
    efx_StereoHarm->cleanup();
    efx_CompBand->cleanup();
    efx_Opticaltrem->cleanup();
    efx_Vibe->cleanup();
    RC->cleanup();
    efx_FLimiter->cleanup();
    efx_Infinity->cleanup();

};


void
RKR::Alg (float *inl1, float *inr1, float *origl, float *origr, void *)
{

    int i;
    int reco=0;
    int ponlast=0;
    efxoutl = inl1;
    efxoutr = inr1;

    if((t_timeout) && (Tap_Bypass)) TapTempo_Timeout(1);

    if (Bypass) {

        Control_Gain (origl, origr);

        if(Metro_Bypass) M_Metronome->metronomeout(m_ticks);

        if((Tap_Bypass) && (Tap_Selection == 4)) {
            beat->detect(efxoutl,efxoutr);
            int bt_tempo=lrintf(beat->get_tempo());
            if((bt_tempo>19) && (bt_tempo<360) && (bt_tempo != Tap_TempoSet)) {
                Tap_TempoSet=bt_tempo;
                Update_tempo();
                Tap_Display=1;
            }

        }


        if (Tuner_Bypass)
            efx_Tuner->schmittFloat (period, efxoutl, efxoutr);

        if (MIDIConverter_Bypass)
            efx_MIDIConverter->schmittFloat (period, efxoutl, efxoutr);


        if ((Harmonizer_Bypass) && (have_signal)) {
            if (efx_Har->mira) {
                if ((efx_Har->PMIDI) || (efx_Har->PSELECT)) {
                    RecNote->schmittFloat (efxoutl, efxoutr);
                    reco=1;
                    if ((reconota != -1) && (reconota != last)) {
                        if(RecNote->afreq > 0.0) {
                            RC->Vamos (0,efx_Har->Pinterval - 12);
                            ponlast = 1;
                        }
                    }
                }
            }
        }


        if ((StereoHarm_Bypass) && (have_signal)) {
            if (efx_StereoHarm->mira) {
                if ((efx_StereoHarm->PMIDI) || (efx_StereoHarm->PSELECT)) {
                    if(!reco) RecNote->schmittFloat (efxoutl, efxoutr);
                    reco=1;
                    if ((reconota != -1) && (reconota != last)) {
                        if(RecNote->afreq > 0.0) {
                            RC->Vamos (1,efx_StereoHarm->Pintervall - 12);
                            RC->Vamos (2,efx_StereoHarm->Pintervalr - 12);
                            ponlast = 1;
                        }
                    }
                }
            }
        }


        if((Ring_Bypass) && (efx_Ring->Pafreq)) {
            if(!reco) RecNote->schmittFloat (efxoutl, efxoutr);
            reco=1;
            if ((reconota != -1) && (reconota != last)) {
                if(RecNote->afreq > 0.0) {
                    efx_Ring->Pfreq=lrintf(RecNote->lafreq);
                    ponlast = 1;
                }
            }
        }

        if(ponlast) last=reconota;

        for (i = 0; i < 10; i++) {
            switch (efx_order[i]) {
            case 0:
                if (EQ1_Bypass) {
                    efx_EQ1->out (efxoutl, efxoutr);
                    Vol2_Efx ();
                }
                break;

            case 1:
                if (Compressor_Bypass) {
                    efx_Compressor->out (efxoutl, efxoutr);
                    Vol2_Efx ();
                }
                break;

            case 5:
                if (Chorus_Bypass) {
                    efx_Chorus->out (efxoutl, efxoutr);
                    Vol_Efx (5, efx_Chorus->outvolume);
                }
                break;

            case 7:
                if (Flanger_Bypass) {
                    efx_Flanger->out (efxoutl, efxoutr);
                    Vol_Efx (7, efx_Flanger->outvolume);
                }
                break;

            case 6:
                if (Phaser_Bypass) {
                    efx_Phaser->out (efxoutl, efxoutr);
                    Vol_Efx (6, efx_Phaser->outvolume);
                }
                break;

            case 2:
                if (Distorsion_Bypass) {
                    efx_Distorsion->out (efxoutl, efxoutr);
                    Vol_Efx (2, efx_Distorsion->outvolume);
                }
                break;

            case 3:
                if (Overdrive_Bypass) {
                    efx_Overdrive->out (efxoutl, efxoutr);
                    Vol_Efx (3, efx_Overdrive->outvolume);
                }
                break;

            case 4:
                if (Echo_Bypass) {
                    efx_Echo->out (efxoutl, efxoutr);
                    Vol_Efx (4, efx_Echo->outvolume);
                }
                break;
            case 8:
                if (Reverb_Bypass) {
                    efx_Rev->out (efxoutl, efxoutr);
                    Vol_Efx (8, efx_Rev->outvolume);
                }
                break;

            case 9:
                if (EQ2_Bypass) {
                    efx_EQ2->out (efxoutl, efxoutr);
                    Vol2_Efx ();
                }
                break;

            case 10:
                if (WhaWha_Bypass) {
                    efx_WhaWha->out (efxoutl, efxoutr);
                    Vol_Efx (10, efx_WhaWha->outvolume);
                }
                break;

            case 11:
                if (Alienwah_Bypass) {
                    efx_Alienwah->out (efxoutl, efxoutr);
                    Vol_Efx (11, efx_Alienwah->outvolume);
                }
                break;

            case 12:
                if (Cabinet_Bypass) {
                    efx_Cabinet->out (efxoutl, efxoutr);
                    Vol3_Efx ();
                }

                break;

            case 13:
                if (Pan_Bypass) {
                    efx_Pan->out (efxoutl, efxoutr);
                    Vol_Efx (13, efx_Pan->outvolume);
                }
                break;

            case 14:
                if (Harmonizer_Bypass) {
                    efx_Har->out (efxoutl, efxoutr);
                    Vol_Efx (14, efx_Har->outvolume);
                }
                break;

            case 15:
                if (MusDelay_Bypass) {
                    efx_MusDelay->out (efxoutl, efxoutr);
                    Vol_Efx (15, efx_MusDelay->outvolume);
                }
                break;

            case 16:
                if (Gate_Bypass) {
                    efx_Gate->out (efxoutl, efxoutr);
                    Vol2_Efx ();
                }
                break;

            case 17:
                if(NewDist_Bypass) {
                    efx_NewDist->out (efxoutl, efxoutr);
                    Vol_Efx(17,efx_NewDist->outvolume);
                }
                break;

            case 18:
                if (APhaser_Bypass) {
                    efx_APhaser->out (efxoutl, efxoutr);
                    Vol_Efx (18, efx_APhaser->outvolume);
                }
                break;

            case 19:
                if (Valve_Bypass) {
                    efx_Valve->out(efxoutl, efxoutr);
                    Vol_Efx (19, efx_Valve->outvolume);
                }
                break;

            case 20:
                if (DFlange_Bypass) {
                    efx_DFlange->out(efxoutl, efxoutr);
                    Vol2_Efx ();
                }
                break;

            case 21:
                if (Ring_Bypass) {
                    efx_Ring->out(efxoutl, efxoutr);
                    Vol_Efx (21,efx_Ring->outvolume);
                }
                break;

            case 22:
                if (Exciter_Bypass) {
                    efx_Exciter->out(efxoutl, efxoutr);
                    Vol2_Efx();
                }
                break;

            case 23:
                if (MBDist_Bypass) {
                    efx_MBDist->out(efxoutl, efxoutr);
                    Vol_Efx(23,efx_MBDist->outvolume);
                }
                break;

            case 24:
                if (Arpie_Bypass) {
                    efx_Arpie->out(efxoutl, efxoutr);
                    Vol_Efx(24,efx_Arpie->outvolume);
                }
                break;

            case 25:
                if (Expander_Bypass) {
                    efx_Expander->out(efxoutl, efxoutr);
                    Vol2_Efx();
                }
                break;

            case 26:
                if (Shuffle_Bypass) {
                    efx_Shuffle->out(efxoutl, efxoutr);
                    Vol_Efx(26,efx_Shuffle->outvolume);
                }
                break;

            case 27:
                if (Synthfilter_Bypass) {
                    efx_Synthfilter->out(efxoutl, efxoutr);
                    Vol_Efx(27,efx_Synthfilter->outvolume);
                }
                break;

            case 28:
                if (MBVvol_Bypass) {
                    efx_MBVvol->out(efxoutl, efxoutr);
                    Vol_Efx(28,efx_MBVvol->outvolume);
                }
                break;

            case 29:
                if (Convol_Bypass) {
                    efx_Convol->out(efxoutl, efxoutr);
                    Vol_Efx(29,efx_Convol->outvolume);
                }
                break;

            case 30:
                if (Looper_Bypass) {
                    efx_Looper->out(efxoutl, efxoutr);
                    Vol_Efx(30,efx_Looper->outvolume);
                }
                break;

            case 31:
                if (RyanWah_Bypass) {
                    efx_RyanWah->out(efxoutl, efxoutr);
                    Vol_Efx(31,efx_RyanWah->outvolume);
                }
                break;

            case 32:
                if (RBEcho_Bypass) {
                    efx_RBEcho->out(efxoutl, efxoutr);
                    Vol_Efx(32,efx_RBEcho->outvolume);
                }
                break;

            case 33:
                if (CoilCrafter_Bypass) {
                    efx_CoilCrafter->out(efxoutl, efxoutr);
                    Vol2_Efx();
                }
                break;

            case 34:
                if (ShelfBoost_Bypass) {
                    efx_ShelfBoost->out(efxoutl, efxoutr);
                    Vol2_Efx();
                }
                break;

            case 35:
                if (Vocoder_Bypass) {
                    efx_Vocoder->out(efxoutl, efxoutr);
                    Vol_Efx(35,efx_Vocoder->outvolume);
                }
                break;

            case 36:
                if (Sustainer_Bypass) {
                    efx_Sustainer->out(efxoutl, efxoutr);
                    Vol2_Efx();
                }
                break;

            case 37:
                if (Sequence_Bypass) {
                    efx_Sequence->out(efxoutl, efxoutr);
                    Vol_Efx(37,efx_Sequence->outvolume);
                }
                break;

            case 38:
                if (Shifter_Bypass) {
                    efx_Shifter->out(efxoutl, efxoutr);
                    Vol_Efx(38,efx_Shifter->outvolume);
                }
                break;

            case 39:
                if (StompBox_Bypass) {
                    efx_StompBox->out(efxoutl, efxoutr);
                    Vol2_Efx();
                }
                break;

            case 40:
                if (Reverbtron_Bypass) {
                    efx_Reverbtron->out(efxoutl, efxoutr);
                    Vol_Efx(40,efx_Reverbtron->outvolume);
                }
                break;

            case 41:
                if (Echotron_Bypass) {
                    efx_Echotron->out(efxoutl, efxoutr);
                    Vol_Efx(41,efx_Echotron->outvolume);
                }
                break;

            case 42:
                if (StereoHarm_Bypass) {
                    efx_StereoHarm->out(efxoutl, efxoutr);
                    Vol_Efx(42,efx_StereoHarm->outvolume);
                }
                break;

            case 43:
                if (CompBand_Bypass) {
                    efx_CompBand->out(efxoutl, efxoutr);
                    Vol_Efx(43,efx_CompBand->outvolume);
                }
                break;

            case 44:
                if (Opticaltrem_Bypass) {
                    efx_Opticaltrem->out(efxoutl, efxoutr);
                    Vol2_Efx();
                }
                break;

            case 45:
                if (Vibe_Bypass) {
                    efx_Vibe->out(efxoutl, efxoutr);
                    Vol_Efx(45,efx_Vibe->outvolume);
                }
                break;

            case 46:
                if (Infinity_Bypass) {
                    efx_Infinity->out(efxoutl, efxoutr);
                    Vol_Efx(46,efx_Infinity->outvolume);

                }



            }

        }

        if(Metro_Bypass) add_metro();

        Control_Volume (origl,origr);

    }

}

