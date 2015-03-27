/*
  Rakarrack Audio FX

  Dual_Flange.h - Super Flanger
  Copyright (C) 2010 Ryan Billing
  Authors:
  Ryan Billing (a.k.a Transmogrifox)  --  Signal Processing
  Copyright (C) 2010 Ryan Billing

  Nasca Octavian Paul -- Remnants of ZynAddSubFX Echo.h structure and utility routines common to ZynSubFX source
  Copyright (C) 2002-2005 Nasca Octavian Paul

  Higher intensity flanging accomplished by picking two points out of the delay line to create a wider notch filter.

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

#ifndef DUAL_FLANGE_H
#define DUAL_FLANGE_H

#include "global.h"
#include "EffectLFO.h"
#include "delayline.h"

class Dflange
{
public:
    Dflange (float * efxoutl_, float * efxoutr_, double sample_rate);
    ~Dflange ();
    void out (float * smpsl, float * smpr, uint32_t period);
    void setpreset (int npreset);
    void changepar (int npar, int value);
    int getpar (int npar);
    void cleanup ();

    int Ppreset;

    float *efxoutl;
    float *efxoutr;

    uint32_t PERIOD;

private:
    //Parameters
    int Pwetdry;		// 0 //Wet/Dry mix.  Range -64 to 64
    int Ppanning;		// 1 //Panning.  Range -64 to 64
    int Plrcross;		// 2 //L/R Mixing.  Range 0 to 127
    int Pdepth;		// 3 //Max delay deviation expressed as frequency of lowest frequency notch.  Min = 20, Max = 4000
    int Pwidth;		// 4 //LFO amplitude.  Range 0 to 16000 (Hz)
    int Poffset;		// 5 //Offset of notch 1 to notch 2.  Range 0 to 100 (percent)
    int Pfb;		// 6 //Feedback parameter.  Range -64 to 64
    int Phidamp;		// 7 //Lowpass filter delay line.  Range 20 to 20000 (Hz)
    int Psubtract;	// 8 //Subtract wet/dry instead of add.  Nonzero is true
    int Pzero;		// 9 //Enable through-zero flanging,   Nonzero is true
    // 10 //LFO Speed
    // 11 //LFO stereo diff
    // 12 //LFO type
    // 13 //LFO Randomness
    int Pintense;		// 14 //Intense Mode

    float wet, dry;		//Wet/Dry mix.
    float lpan, rpan;		//Panning.
    float flrcross, frlcross;	// L/R Mixing.
    float fdepth;		//Max delay deviation expressed as frequency of lowest frequency notch.  Min = 20, Max = 15000
    float fwidth;		//LFO amplitude.
    float foffset;		// Offset of notch 1 to notch 2.  Range 0 to 1.0
    float ffb;			//Feedback parameter.  Range -0.99 to 0.99
    float fhidamp;		//Lowpass filter delay line.  Range 20 to 20000 (Hz)
    float fsubtract;		//Subtract wet/dry instead of add.  Nonzero is true
    float fzero;		//Enable through-zero flanging
    float logmax;
    EffectLFO *lfo;		//lfo Flanger

    //Internally used variables
    int maxx_delay;
    int kl, kr, zl, zr;
    int zcenter;

    float l, r, ldl, rdl, zdr, zdl;
    float rflange0, rflange1, lflange0, lflange1, oldrflange0, oldrflange1, oldlflange0, oldlflange1;
    float period_const, base, ibase;
    float *ldelay, *rdelay, *zldelay, *zrdelay;
    float oldl, oldr;		//pt. lpf
    float rsA, rsB, lsA, lsB;	//Audio sample at given delay

    delayline *ldelayline0, *rdelayline0, *ldelayline1, *rdelayline1;
    class FPreset *Fpre;

    float fSAMPLE_RATE;


};


#endif
