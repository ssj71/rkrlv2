/*
  ZynAddSubFX - a software synthesizer

  DynamicFilter.h - "WahWah" effect and others
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

#ifndef DYNAMICFILTER_H
#define DYNAMICFILTER_H

#include "global.h"
#include "EffectLFO.h"
#include "Filter.h"

class DynamicFilter
{
public:
    DynamicFilter (float * efxoutl_, float * efxoutr_, double sample_rate, uint32_t intermediate_bufsize);
    ~DynamicFilter ();
    void out (float * smpsl, float * smpsr, uint32_t period);

    void setpreset (int npreset);
    void changepar (int npar, int value);
    int getpar (int npar);
    void cleanup ();


    int Ppreset;
    float *efxoutl;
    float *efxoutr;
    float outvolume; 

    uint32_t PERIOD;

private:
    //Parametrii DynamicFilter
    EffectLFO* lfo;		//lfo-ul DynamicFilter
    int Pvolume;
    int Ppanning;
    int Pdepth;		//the depth of the lfo of the DynamicFilter
    int Pampsns;	//how the filter varies according to the input amplitude
    int Pampsnsinv;	//if the filter freq is lowered if the input amplitude rises
    int Pampsmooth;	//how smooth the input amplitude changes the filter
    int Pmode;      //settings of filters

    //Control Parametrii
    void setvolume (int Pvolume);
    void setpanning (int Ppanning);
    void setdepth (int Pdepth);
    void setampsns (int Pampsns);

    void reinitfilter ();

    //Valorile interne

    float panning, depth, ampsns, ampsmooth;
    float ms1, ms2, ms3, ms4;	//mean squares

    class Filter *filterl, *filterr;
    class FilterParams *filterpars;
    class FPreset *Fpre;

};

#endif
