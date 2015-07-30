/*

  MBDist.h - Distorsion Effect

  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul
  ZynAddSubFX - a software synthesizer

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

#ifndef MBVVOL_H
#define MBVVOL_H

#include "global.h"
#include "EffectLFO.h"
#include "AnalogFilter.h"


class MBVvol
{
public:
    MBVvol (float * efxoutl_, float * efxoutr_, double sample_rate, uint32_t intermediate_bufsize);
    ~MBVvol ();
    void out (float * smpsl, float * smpr, uint32_t period);
    void setpreset (int npreset);
    void changepar (int npar, int value);
    int getpar (int npar);
    void cleanup ();

    int Ppreset;
    float outvolume;

    float *efxoutl;
    float *efxoutr;
    float *lowl;
    float *lowr;
    float *midll;
    float *midlr;
    float *midhl;
    float *midhr;
    float *highl;
    float *highr;

    uint32_t PERIOD;

private:

    void setvolume (int Pvolume);
    void setpanning (int Ppanning);
    void setCross1 (int value);
    void setCross2 (int value);
    void setCross3 (int value);
    void setCombi (int value);
    void updateVols(void);
    void setSource(float* ptr, float* ptrr, int val);


    //Parametrii
    int Pvolume;	//Volumul or E/R
    int Pcombi;
    int Cross1;
    int Cross2;
    int Cross3;
    int PsL, PsML, PsMH, PsH;//volume source per band

    //Parametrii reali

    float coeff;
    float lfo1l,lfo1r,lfo2l,lfo2r;
    float v1l,v1r,v2l,v2r;
    float d1,d2,d3,d4;
    float volL,volML,volMH,volH;
    float volLr,volMLr,volMHr,volHr;
    float *sourceL,*sourceML,*sourceMH,*sourceH;
    float *sourceLr,*sourceMLr,*sourceMHr,*sourceHr;
    float one, zero;
    AnalogFilter *lpf1l, *lpf1r, *hpf1l, *hpf1r;
    AnalogFilter *lpf2l, *lpf2r, *hpf2l, *hpf2r;
    AnalogFilter *lpf3l, *lpf3r, *hpf3l, *hpf3r;
    float* interpbuf; //buffer for filters

    EffectLFO* lfo1,*lfo2;

    class FPreset *Fpre;

};


#endif
