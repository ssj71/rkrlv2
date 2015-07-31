/*
  Rakarrack Guitar Effects Processor

  InfiniT : Infinitely rising & falling filter effect
  Author:  Ryan Billing (aka Transmogrifox)
  Copyright (C) 2010

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

#ifndef INFINITY_H
#define INFINITY_H
#include "global.h"
#include "RBFilter.h"

#define NUM_INF_BANDS 8

class Infinity
{
public:
    Infinity (float * efxoutl_, float * efxoutr_, double sample_rate, uint32_t intermediate_bufsize);
    ~Infinity ();
    void out (float * smpsl, float * smpsr, uint32_t period);

    void setpreset (int npreset);
    void changepar (int npar, int value);
    int getpar (int npar);
    void cleanup ();

    int Ppreset;
    float outvolume;

    float *efxoutl;
    float *efxoutr;


private:

    void setvolume (int Pvolume);
    void adjustfreqs();
    void oscillator();
    void reinitfilter ();
    void setq ();

    int Pvolume;		//For wet/dry mix // "Wet/Dry"
    int Pb[NUM_INF_BANDS];          //Pbx -64 to 64// "Band" each label "1" through "8" adjusts mix of each filter band
    int Pq;              //-64 to 64// "Res." filter resonance
    int Pstartfreq;         //0 to 127// "Start" where the filter starts
    int Pendfreq;	       //0 to 127// "End" when the filter ends
    int Prate;           //BPM// "Tempo" how fast it sweeps
    int Pstdf;           //filter bands offset L&R
    int Psubdiv;
    int Pautopan;
    int Preverse;
    int Pstages;        //filter stages

    struct filterstate {
        float sinp;
        float cosp;      //sinusoid oscillator state variables
        float ramp;      //frequency oscillator
        float level;
        float lfo;
        float vol;
    } rbandstate[NUM_INF_BANDS], lbandstate[NUM_INF_BANDS];

    struct phasevars {
        float yn1[MAX_PHASER_STAGES];
        float xn1[MAX_PHASER_STAGES];
        float gain;
    } rphaser[NUM_INF_BANDS], lphaser[NUM_INF_BANDS];
    float phaserfb;
    float phaser(phasevars *pstruct, float fxn, int j);
    /*
    fconst = freq* 2*pi/fs;
    rampconst = 2*freq/fs;
     */
    float cfconst, fconst;  //sine wave oscillator frequency constant // 2*pi*freq/fs
    float rampconst, irampconst, crampconst, cirampconst;  //ramp oscillator constant// 2*freq/SR
    float alpha, beta;
    float fstart, fend;  //range of filter sweep
    float linconst, logconst;
    float minlevel, maxlevel;
    float frmin, frmax;
    float qq;
    float stdiff;
    float volmaster;
    float autopan, msin, dsin, mcos, mconst;
    float ratescale;
    int tflag;

    float fSAMPLE_RATE;

    float* interpbuf; //buffer for filters
    class RBFilter *filterl[NUM_INF_BANDS], *filterr[NUM_INF_BANDS];

    class FPreset *Fpre;


};

#endif
