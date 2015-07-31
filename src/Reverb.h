/*
  ZynAddSubFX - a software synthesizer

  Reverb.h - Reverberation effect
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  Modified for rakarrack by Josep Andreu

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

#ifndef REVERB_H
#define REVERB_H


#include "global.h"
#include "AnalogFilter.h"

class Reverb
{
public:
    Reverb (float * efxoutl_, float * efxoutr_, double samplerate, uint16_t intermediate_bufsize);
    ~Reverb ();
    void out (float * smps_l, float * smps_r, uint32_t period);
    void cleanup ();

    void setpreset (int npreset);
    void changepar (int npar, int value);
    int getpar (int npar);

    int Ppreset;
    float outvolume;		//this is the volume of effect and is public because need it in system effect. The out volume of su

    float *efxoutl;
    float *efxoutr;



private:

    void setvolume (int Pvolume);
    void setpan (int Ppan);
    void settime (int Ptime);
    void setlohidamp (int Plohidamp);
    void setidelay (int Pidelay);
    void setidelayfb (int Pidelayfb);
    void sethpf (int Phpf);
    void setlpf (int Plpf);
    void settype (int Ptype);
    void setroomsize (int Proomsize);
    void processmono (unsigned int ch, float * output, uint32_t period);

    float fSAMPLE_RATE;


    //Parametrii
    //Amount of the reverb,
    int Pvolume;

    //LefT/Right Panning
    int Ppan;

    //duration of reverb
    int Ptime;

    //Initial delay
    int Pidelay;

    //Initial delay feedback
    int Pidelayfb;

    //delay between ER/Reverbs
    int Prdelay;

    //EarlyReflections/Reverb Balance
    int Perbalance;

    //HighPassFilter
    int Plpf;

    //LowPassFilter
    int Phpf;

    //Low/HighFrequency Damping
    int Plohidamp;	// 0..63 lpf,64=off,65..127=hpf(TODO)

    //Reverb type
    int Ptype;

    //Room Size
    int Proomsize;

    //Parametrii 2
    int lohidamptype;		//0=disable,1=highdamp(lowpass),2=lowdamp(highpass)
    int idelaylen, rdelaylen;
    int idelayk;
    int comblen[REV_COMBS * 2];
    int aplen[REV_APS * 2];


    int combk[REV_COMBS * 2];
    int apk[REV_APS * 2];

    float lohifb, idelayfb, roomsize, rs;	//rs is used to "normalise" the volume according to the roomsize
    float rs_coeff;
    //parameter control
    float pan, erbalance;


    //Valorile interne

    float *comb[REV_COMBS * 2];

    float combfb[REV_COMBS * 2];	//feedback-ul fiecarui filtru "comb"
    float lpcomb[REV_COMBS * 2];	//pentru Filtrul LowPass

    float *ap[REV_APS * 2];
    float *inputbuf;
    float *idelay;

    float* interpbuf; //buffer for filters
    class AnalogFilter *lpf, *hpf;	//filters
    class FPreset *Fpre;
};




#endif
