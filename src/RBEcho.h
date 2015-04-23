/*
  ZynAddSubFX - a software synthesizer

  Echo.h - Echo Effect
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  Modified for rakarrack by Josep Andreu

  Reverse Echo by Transmogrifox

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

#ifndef RBECHO_H
#define RBECHO_H

#include "global.h"
#include "delayline.h"

class RBEcho
{
public:
    RBEcho (float * efxoutl_, float * efxoutr_, double sample_rate);
    ~RBEcho ();
    void out (float * smpsl, float * smpr, uint32_t period);
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
    void setpanning (int Ppanning);
    void setdelay (int Pdelay);
    void setlrdelay (int Plrdelay);
    void setlrcross (int Plrcross);
    void setfb (int Pfb);
    void sethidamp (int Phidamp);
    void setreverse (int Preverse);
    void initdelays ();

    //Parametrii
    int Pvolume;	// E/R
    int Ppanning;	//Panning
    int Pdelay;
    int Plrdelay;	// L/R delay difference
    int Plrcross;	// L/R Mixing
    int Pfb;		//Feed-back
    int Phidamp;
    int Preverse;
    int Psubdiv;
    int Pes;

    int maxx_delay;

    float delay, lrdelay, ltime, rtime;
    float fdelay, subdiv, pes, pingpong, ipingpong;
    float rvl, rvr;
    float rpanning, lpanning, lrcross, fb, hidamp, reverse, ireverse, lfeedback, rfeedback;
    float oldl, oldr;		//pt. lpf
    float  Srate_Attack_Coeff;
    float fSAMPLE_RATE;

    class delayline *ldelay, *rdelay;
    class FPreset *Fpre;
};


#endif
