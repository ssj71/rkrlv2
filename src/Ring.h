/*
  ZynAddSubFX - a software synthesizer
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Based on Ring Modulator Steve Harris LADSPA plugin.
  Ring.h - Distorsion Effect

  Modified and adapted to rakarrack by Josep Andreu

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

#ifndef RING_H
#define RING_H

#include "global.h"


class Ring
{
public:
    Ring (float * efxoutl_, float * efxoutr_, double sample_rate);
    ~Ring ();
    void out (float * smpsl, float * smpr, uint32_t period);
    void setpreset (int npreset);
    void changepar (int npar, int value);
    int getpar (int npar);
    void setscale();
    void cleanup ();
    void Create_Tables(float sample_rate);

    int Ppreset;
    int Pafreq;
    int Pfreq;

    float outvolume;

    float *efxoutl;
    float *efxoutr;

private:

    void setvolume (int Pvolume);
    void setpanning (int Ppanning);
    void setlrcross (int Plrcross);


    //Parametrii
    int Pvolume;	//Volumul or E/R
    int Ppanning;	//Panning
    int Plrcross;	// L/R Mixing
    int Pinput;
    int Plevel;		//the ouput amplification
    int Pdepthp;
    int Psin;		//highpass filter
    int Ptri;
    int Psaw;
    int Psqu;
    int Pstereo;	//0=mono,1=stereo


    //Parametrii reali
    unsigned int offset;
    float panning, lrcross;
    float *sin_tbl, *tri_tbl, *saw_tbl, *squ_tbl;
    float sin,tri,saw,squ,scale,depth, idepth;

    class FPreset *Fpre;

    unsigned int SAMPLE_RATE;
};


#endif
