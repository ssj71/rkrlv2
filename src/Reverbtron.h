/*
  Reverbtron.h - Reverbtron Effect

  Author: Ryan Billing & Josep Andreu

  Adapted effect structure of ZynAddSubFX - a software synthesizer
  Author: Nasca Octavian Paul

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#ifndef REVERBTRON_H
#define REVERBTRON_H

#include "global.h"
#include "AnalogFilter.h"
#include "Resample.h"

class RvbFile
{
public:
	char Filename[128];
	float tdata[2000];
	float ftime[2000];
	int data_length;
	float maxtime;
	float maxdata;
};

class Reverbtron
{
public:
    Reverbtron (float * efxoutl_, float * efxoutr_, double sample_rate, uint32_t intermediate_bufsize,
    		int DS, int uq, int dq);
    ~Reverbtron ();
    void out (float * smpsl, float * smpr, uint32_t period);
    void setpreset (int npreset);
    void changepar (int npar, int value);
    int getpar (int npar);
    void cleanup ();
    int setfile (int value);
    void adjust(int DS, double sample_rate);
    RvbFile loadfile(char* filename);
    void applyfile(RvbFile file);

    int Ppreset;

    float *efxoutl;
    float *efxoutr;
    float outvolume;

    char Filename[128];
    RvbFile File;

private:

    void setvolume (int Pvolume);
    void setpanning (int Ppanning);
    void sethidamp (int Phidamp);
    void setlpf (int Plpf);
    void setfb(int value);
    void convert_time();
    RvbFile loaddefault();


    //Parametrii
    int Pvolume;	//This is master wet/dry mix like other FX...but I am finding it is not useful
    int Ppanning;	//Panning
    int Plrcross;	// L/R Mixing  // This is a mono effect, so lrcross and panning are pointless
    int Phidamp;
    int Plevel;		//This should only adjust the level of the IR effect, and not wet/dry mix
    int Plength;		//20... 1500// Set maximum number of IR points to calculate.
    int Puser;		//-64...64//Feedback.
    int Pstretch;		//-64 ... 64//For stretching reverb responses
    int Pidelay;         //0 ... 500ms// initial delay time
    int Filenum;
    int Psafe;
    int Pfb;		//-64 ... 64// amount of feedback
    int Pfade;
    int Pes;		//0 or 1// Add stereo spatialization
    int Prv;              //Shuffle
    int Plpf;
    int Pdiff;

    int imctr;
    int imax;
    int offset;
    int hoffset;
    int maxx_size;
    //int data_length;
    int error;// flag if error when loading file
    int Llength; //Plength but limited
    int avgtime;
    int hrtf_size;
    int hlength;
    int DS_state;
    int nPERIOD;
    int nSAMPLE_RATE;
    float nRATIO;


    int *time, *rndtime;
    double u_up;
    double u_down;
    float nfSAMPLE_RATE;

    //float fstretch, idelay, ffade, maxtime, maxdata, decay, diffusion;
    float fstretch, idelay, ffade, decay, diffusion;
    float lpanning, rpanning, hidamp, alpha_hidamp, convlength, oldl;
    //float *data, *lxn, *imdelay, *ftime, *tdata, *rnddata, *hrtf;
    float *data, *lxn, *imdelay, *rnddata, *hrtf;
    float *templ, *tempr;
    float level,fb, feedback,levpanl,levpanr;
    float roomsize;

    class Resample *U_Resample;
    class Resample *D_Resample;


    float* interpbuf; //buffer for filters
    class AnalogFilter *lpfl, *lpfr;	//filters

    class FPreset *Fpre;

};


#endif
