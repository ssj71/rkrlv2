/*

  Shuffle.h - Stereo Shuffle

  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul
  ZynAddSubFX - a software synthesizer

  Modified for rakarrack by Josep Andreu & Hernan Ordiales

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

#ifndef SHUFFLE_H
#define SHUFFLE_H

#include "global.h"
#include "AnalogFilter.h"


class Shuffle
{
public:
    Shuffle (float * efxoutl_, float * efxoutr_, double sample_rate, uint32_t intermediate_bufsize);
    ~Shuffle ();
    void out (float * smpsl, float * smpr, uint32_t period);
    void setpreset (int npreset);
    void changepar (int npar, int value);
    int getpar (int npar);
    void cleanup ();

    int Ppreset;
    float outvolume;

    float *efxoutl;
    float *efxoutr;
    float *inputl;
    float *inputr;


private:

    void setvolume (int Pvolume);
    void setCross1 (int value);
    void setCross2 (int value);
    void setCross3 (int value);
    void setCross4 (int value);
    void setGainL (int value);
    void setGainML (int value);
    void setGainMH (int value);
    void setGainH (int value);

    //Parametrii
    int Pvolume;	//Volumul or E/R
    int PvolL;
    int PvolML;
    int PvolMH;
    int PvolH;
    int PQ;
    int E;

    int Cross1;		//lowpass filter
    int Cross2;		//highpass filter
    int Cross3;
    int Cross4;


    //Parametrii reali
    float tmp;
    float volL,volML,volMH,volH;
    AnalogFilter  *lr, *hr;
    AnalogFilter  *mlr,*mhr;
    float* interpbuf; //buffer for filters

    class FPreset *Fpre;

};


#endif
