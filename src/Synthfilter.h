/*
  ZynAddSubFX - a software synthesizer
  Synthfilter.h - Synthesizer filter effect
  Copyright (C) 2010 Ryan Billing
  Based on Analog Phaser  derived from
  Phaser.h/.C
  Copyright (C) 2002-2005 Nasca Octavian Paul

  Authors: Nasca Octavian Paul, Ryan Billing, Josep Andreu

  Modified for rakarrack by Josep Andreu

  Further modified for rakarrack by Ryan Billing (Transmogrifox) to model Analog Phaser behavior 2009

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

#ifndef SYNTHFILTER_H
#define SYNTHFILTER_H
#include "global.h"
#include "EffectLFO.h"


class Synthfilter
{
public:
    Synthfilter (float * efxoutl_, float * efxoutr_, double sample_rate);
    ~Synthfilter ();
    void out (float * smpsl, float * smpsr, uint32_t period);
    void setpreset (int npreset);
    void changepar (int npar, int value);
    int getpar (int npar);
    void cleanup ();

    int Ppreset;
    float outvolume;

    float *efxoutl;
    float *efxoutr;

    uint32_t PERIOD;

private:

    //Control parameters
    void setvolume (int Pvolume);
    void setdistortion (int Pdistortion);
    void setwidth (int Pwidth);
    void setfb (int Pfb);
    void setdepth (int Pdepth);

    //Phaser parameters
    int Pvolume;			 //0//Used in Process.C to set wet/dry mix
    int Pdistortion;		 //1//0...127//Model distortion
    //2//Tempo//LFO frequency
    //3//0...127//LFO Random
    //4//0...max types//LFO Type
    //5//0...127//LFO stereo offset
    int Pwidth;			 //6//0...127//Phaser width (LFO amplitude)
    int Pfb;			 //7//-64...64//feedback
    int Plpstages;	         //8//0...12//Number of first-order Low-Pass stages
    int Phpstages;		 //9//0...12//Number of first-order High-Pass stages
    int Poutsub;			 //10//0 or 1//subtract the output instead of the adding it
    int Pdepth;			 //11//0...127//Depth of phaser sweep
    int Penvelope;		 //12//-64...64//envelope sensitivity
    int Pattack;			 //13//0...1000ms//Attack Time
    int Prelease;			 //14//0...500ms//Release Time
    int Pbandwidth;		 //15//0...127//Separate high pass & low pass



    //Internal Variables
    float distortion, fb, width, env, envdelta, sns, att, rls, fbl, fbr, depth, bandgain;
    float *lyn1, *ryn1, *lx1hp, *ly1hp, *rx1hp, *ry1hp;
    float oldlgain, oldrgain, inv_period;

    float delta;
    float Rmin;	// 2N5457 typical on resistance at Vgs = 0
    float Rmax;	// Resistor parallel to FET
    float C, Clp, Chp;	        // Capacitor
    EffectLFO* lfo;	         //Filter modulator

    class FPreset *Fpre;

};

#endif
