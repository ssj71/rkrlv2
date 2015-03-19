/*
  ZynAddSubFX - a software synthesizer

  Echo.h - Echo Effect
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

#ifndef MUSDELAY_H
#define MUSDELAY_H

#include "global.h"


class MusicDelay
{
public:
    MusicDelay (float * efxoutl_, float * efxoutr_, double sample_rate);
    ~MusicDelay ();
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
    void setpanning (int num, int Ppanning);
    void setdelay (int num, int Pdelay);
    void setgain (int num, int Pgain);
    void setlrdelay (int Plrdelay);
    void setlrcross (int Plrcross);
    void setfb (int num, int Pfb);
    void sethidamp (int Phidamp);
    void settempo (int Ptempo);
    void initdelays ();



    float fSAMPLE_RATE;
    //Parametrii
    int Pvolume;			//Volumul or E/R
    int Ppanning1;		//Panning
    int Ppanning2;
    int Pgain1;
    int Pgain2;
    int Pdelay1;
    int Pdelay2;
    int Plrdelay;			// L/R delay difference
    int Plrcross;			// L/R Mixing
    int Pfb1;			//Feed-back-ul
    int Pfb2;
    int Phidamp;
    int Ptempo;

    //Parametrii reali
    int dl1, dr1, dl2, dr2, delay1, delay2, lrdelay;
    int kl1, kr1, kl2, kr2;
    int maxx_delay;
    float panning1, panning2, lrcross, fb1, fb2, hidamp;
    float gain1, gain2;
    float *ldelay1, *rdelay1, *ldelay2, *rdelay2;
    float oldl1, oldr1, oldl2, oldr2;	//pt. lpf

    class FPreset *Fpre;

};


#endif
