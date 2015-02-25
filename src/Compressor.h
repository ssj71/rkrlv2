/*
  rakarrack - a guitar effects software

 Compressor.h  -  Compressor Effect definitions
 Based on artscompressor.cc by Matthias Kretz <kretz@kde.org>
 Stefan Westerfeld <stefan@space.twc.de>

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
 (version2)  along with this program; if not, write to the Free Software
 Foundation,
 Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/


#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include "FPreset.h"

class Compressor
{

public:

    Compressor (float * efxoutl_, float * efxoutr_, double samplerate);
    ~Compressor ();

    void out (float * smps_l, float * smps_r, uint32_t period);

    void Compressor_Change (int np, int value);
    void Compressor_Change_Preset (int dgui,int npreset);
    int getpar (int npar);
    void cleanup ();

    float *efxoutl;
    float *efxoutr;



    // Compressor

    int tatt;			// attack time  (ms)
    int trel;			// release time (ms)
    int tratio;
    int toutput;
    int tthreshold;
    int a_out;
    int stereo;
    int tknee;
    int peak;
    int clipping;
    int limit;

private:

    float rvolume;
    float lvolume;
    float rvolume_db;
    float lvolume_db;
    float thres_db;		// threshold
    float knee;
    float thres_mx;
    float kpct;
    float ratio;			// ratio
    float kratio;			// ratio maximum for knee region
    float eratio;			// dynamic ratio
    float makeup;			// make-up gain
    float makeuplin;

    float outlevel;
    float att, attr, attl;
    float rel, relr, rell;
    float relcnst, attconst;
    int ltimer, rtimer, hold;

    float rgain;
    float rgain_old;

    float lgain;
    float lgain_old;

    float lgain_t;
    float rgain_t;

    float coeff_kratio;
    float coeff_ratio;
    float coeff_knee;
    float coeff_kk;
    float lpeak;
    float rpeak;

    class FPreset *Fpre;

    float cSAMPLE_RATE;

};

#endif
