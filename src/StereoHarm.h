/*
  rakarrack - a guitar effects software

  StereoHarm.h  -  Stereo Harmonizer definitions
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

#ifndef STEREOHARM_H
#define STEREOHARM_H

#include "global.h"
#include "smbPitchShift.h"
#include "Resample.h"


class StereoHarm
{

public:
    StereoHarm (float *efxoutl_, float *efxoutr_, long int Quality, int DS, int uq, int dq, uint32_t intermediate_bufsize, double sample_rate);
    ~StereoHarm ();
    void out (float *smpsl, float *smpsr, uint32_t period);
    void setpreset (int npreset);
    void changepar (int npar, int value);
    int getpar (int npar);
    void cleanup ();
    void adjust(int DS, uint32_t period);


    int Ppreset;
    int Pintervall;
    int Pintervalr;

    int PMIDI;
    int PSELECT;
    int mira;
    int DS_state;
    unsigned int nPERIOD;
    float nRATIO;
    int nSAMPLE_RATE;
    unsigned int SAMPLE_RATE;
    long window;

    long int hq;

    double u_up;
    double u_down;
    float nfSAMPLE_RATE;


    float *efxoutl;
    float *efxoutr;
    float *outil,*outir;
    float *outol,*outor;
    float *templ, *tempr;

    float outvolume;

    float r_ratiol;
    float r_ratior;


private:

    int Pvolume;
    int Plrcross;
    int Pgainl;
    int Pgainr;
    int Pchromel;
    int Pchromer;
    int Pnote;
    int Ptype;


    float gainl,gainr;
    float intervall;
    float intervalr;
    float chromel;
    float chromer;
    float lrcross;
    void setvolume (int Pvolume);
    void setinterval (int chan, int value);
    void setchrome(int chan, int value);
    void setgain (int chan, int value);
    void setMIDI (int PMIDI);
    void setlrcross(int value);

    class Resample *U_Resample;
    class Resample *D_Resample;

    PitchShifter *PSl, *PSr;

    class FPreset *Fpre;
};

#endif
