
// Based in gate_1410.c LADSPA Swh-plugins

/*
  rakarrack - a guitar effects software

 Expander.h  -  Noise Gate Effect definitions
 Based on artscompressor.cc by Matthias Kretz <kretz@kde.org>
 Stefan Westerfeld <stefan@space.twc.de>

  Copyright (C) 2008-2010 Ryan Billing & Josep Andreu
  Author: Ryan Billing & Josep Andreu

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


#ifndef EXPANDER_H
#define EXPANDER_H

#include "global.h"
#include "AnalogFilter.h"

class Expander
{

public:

    Expander (float * efxoutl_, float * efxoutr_, double sample_rate, uint32_t intermediate_bufsize);
    ~Expander ();

    void out (float * smps_l, float * smps_r, uint32_t period);

    void Expander_Change (int np, int value);
    void Expander_Change_Preset (int npreset);
    void cleanup ();
    int getpar (int npar);

    float *efxoutl;
    float *efxoutr;



    // Compressor

    int Pthreshold;		// Threshold, -80 to 0dB
    int Pattack;			// attack time  (ms)  Set range from 10ms ... 2000ms
    int Pdecay;			// release time (ms)  Set range from 10ms ... 500ms
    int Pshape;			// Sharpness of transition from off to on.  Range is 0 ... 50
    int Plpf;
    int Phpf;
    int Plevel;
    int efollower;		// This is a mode allowing this object to be used as a "dynamics detector"
    // (envelope follower).  If efollower == 1, then efxoutl is a level to be used in place of an LFO
    // for filter modulation, etc. Then efxoutr is signal + envelope for things such as dynamic distortion.
    // Variable efollower is set to 0 by default in constructor.  Do not set this mode unless using this object
    // to control a parameter with signal dynamics.

private:

    void setlpf (int Plpf);
    void sethpf (int Phpf);


    float sgain;
    float sfactor;
    float tfactor;
    float tlevel;
    float a_rate;
    float d_rate;
    float env;
    float oldgain;
    float gain;
    float fs;
    float level;


    float* interpbuf; //buffer for filters
    AnalogFilter *lpfl, *lpfr, *hpfl, *hpfr;

    class FPreset *Fpre;

};

#endif
