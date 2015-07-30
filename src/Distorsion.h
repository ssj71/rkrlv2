/*
  ZynAddSubFX - a software synthesizer

  Distorsion.h - Distorsion Effect
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  Modified for rakarrack by Josep Andreu & Hernan Ordiales & Ryan Billing

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

#ifndef DISTORSION_H
#define DISTORSION_H

#include "global.h"
#include "AnalogFilter.h"
#include "Waveshaper.h"

class Distorsion
{
public:
    Distorsion (float * efxoutl_, float * efxoutr_, double samplerate,
    		uint32_t intermediate_bufsize, int wave_res, int wave_upq, int wave_dnq);
    ~Distorsion ();
    void out (float * smpsl, float * smpr, uint32_t period);
    void setpreset (int dgui, int npreset);
    void changepar (int npar, int value);
    int getpar (int npar);
    void cleanup ();
    void applyfilters (float * efxoutl, float * efxoutr, uint32_t period);

    int Ppreset;
    float *efxoutl;
    float *efxoutr;
    float *octoutl;
    float *octoutr;
    float outvolume;

private:
    //Parametrii
    int Pvolume;	//Volumul or E/R
    int Ppanning;	//Panning
    int Plrcross;	// L/R Mixing
    int Pdrive;		//the input amplification
    int Plevel;		//the ouput amplification
    int Ptype;		//Distorsion type
    int Pnegate;	//if the input is negated
    int Plpf;		//lowpass filter
    int Phpf;		//highpass filter
    int Pstereo;	//0=mono,1=stereo
    int Pprefiltering;	//if you want to do the filtering before the distorsion
    int Poctave;	//mix sub octave

    void setvolume (int Pvolume);
    void setpanning (int Ppanning);
    void setlrcross (int Plrcross);
    void setoctave (int Poctave);
    void setlpf (int Plpf);
    void sethpf (int Phpf);

    //Parametrii reali
    float panning, lrcross, octave_memoryl, togglel, octave_memoryr,toggler,octmix;
    AnalogFilter *lpfl, *lpfr, *hpfl, *hpfr, *blockDCl, *blockDCr, *DCl, *DCr;
    class Waveshaper *dwshapel, *dwshaper;
    class FPreset *Fpre;

    float * interpbuf;//buffer for filters

};




#endif
