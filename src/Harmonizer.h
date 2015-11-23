/*
  rakarrack - a guitar effects software

  Harmonizer.h  -  Harmonizer definitions
  Copyright (C) 2008 Josep Andreu
  Author: Josep Andreu

  Using Stephan M. Bernsee smbPitchShifter engine.

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

#ifndef HARMONIZER_H
#define HARMONIZER_H

#include "global.h"
#include "AnalogFilter.h"
#include "smbPitchShift.h"
#include "Resample.h"


class Harmonizer
{

public:
    Harmonizer (float *efxoutl_, float *efxoutr_, long int Quality, int DS, int uq, int dq, uint16_t intermediate_bufsize, double sample_rate);
    ~Harmonizer ();
    void out (float *smpsl, float *smpsr, uint32_t period);
    void setpreset (int npreset);
    void changepar (int npar, int value);
    int getpar (int npar);
    void cleanup ();
    void applyfilters (float * efxoutl, uint32_t period);
    void adjust(int DS, uint32_t period);


    int Ppreset;
    int Pinterval;
    int PMIDI;
    int PSELECT;
    int mira;
    int DS_state;
    int nPERIOD;
    int nSAMPLE_RATE;
    long window;

    long int hq;

    double u_up;
    double u_down;
    float nfSAMPLE_RATE;



    float *efxoutl;
    float *efxoutr;
    float *outi;
    float *outo;
    float *templ, *tempr;

    float outvolume;

    float r_ratio;

private:

    int Pvolume;
    int Pgain;
    int Ppan;
    int Pnote;
    int Ptype;

    int fPfreq;
    int fPgain;
    int fPq;

    float panning;
    float gain;
    float interval;

    void setvolume (int Pvolume);
    void setpanning (int Ppan);
    void setinterval (int Pinterval);
    void setgain (int Pgain);
    void setMIDI (int PMIDI);
    void fsetfreq (int value);
    void fsetgain (int value);
    void fsetq (int value);


    AnalogFilter *pl;
    float* interpbuf; //buffer for filters

    class Resample *U_Resample;
    class Resample *D_Resample;

    PitchShifter *PS;
    class FPreset *Fpre;

    unsigned int SAMPLE_RATE;
    bool DS_init;
};

#endif
