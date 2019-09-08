/*
  rakarrack - a guitar effects software

 Vibe.h  -  Vibe Effect definitions

  Copyright (C) 2008-2010 Ryan Billing
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


#ifndef Vibe_H
#define Vibe_H

#include "global.h"
#include "EffectLFO.h"

class Vibe
{

public:

    Vibe (float * efxoutl_, float * efxoutr_, double sample_rate);
    ~Vibe ();

    void out (float * smpsl, float * smpsr, uint32_t period);
    void setvolume(int value);
    void setpanning(int value);
    void setpreset (int npreset);
    void changepar (int npar, int value);
    int getpar (int npar);
    void cleanup ();

    float outvolume;
    float *efxoutl;
    float *efxoutr;
    uint32_t PERIOD;

private:
    int Pwidth;
    int Pfb;
    int Plrcross;
    int Pdepth;
    int Ppanning;
    int Pvolume;
    int Pstereo;

    float fwidth;
    float fdepth;
    float rpanning, lpanning;
    float flrcross, fcross;
    float fb;
    EffectLFO* lfo;

    float Ra, Rb, b, dTC, dRCl, dRCr, lampTC, ilampTC, minTC, alphal, alphar, stepl, stepr, oldstepl, oldstepr;
    float fbr, fbl;
    float dalphal, dalphar;
    float lstep,rstep;
    float cperiod;
    float gl, oldgl;
    float gr, oldgr;

    float cSAMPLE_RATE;
    float fSAMPLE_RATE;

    class fparams
    {
    public:
        fparams () {
            clear ();
            n0 = n1 = d0 = d1 = 0;
        }
        void clear () {
            x1 = y1 = 0;
        }
        //filter state
        float x1;
        float y1;
        //filter coefficients
        float n0;
        float n1;
        float d0;
        float d1;
    } vc[8], vcvo[8], ecvc[8], vevo[8], bootstrap[8];

    float vibefilter(float data, fparams *ftype, int stage) const;
    void init_vibes();
    void modulate(float ldrl, float ldrr);
    float bjt_shape(float data) const;

    float R1;
    float Rv;
    float C2;
    float C1[8];
    float beta;  //transistor forward gain.
    float gain, k;
    float oldcvolt[8] ;
    float en1[8], en0[8], ed1[8], ed0[8];
    float cn1[8], cn0[8], cd1[8], cd0[8];
    float ecn1[8], ecn0[8], ecd1[8], ecd0[8];
    float on1[8], on0[8], od1[8], od0[8];

    class FPreset *Fpre;


};

#endif
