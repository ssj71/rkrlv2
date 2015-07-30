/*

  CompBand.h - 4 Bands Compressor

  Using Compressor and AnalogFilters by other authors.

  Based on artscompressor.cc by Matthias Kretz <kretz@kde.org>
  Stefan Westerfeld <stefan@space.twc.de>
  Modified by Ryan Billing & Josep Andreu

  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul
  ZynAddSubFX - a software synthesizer

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

#ifndef COMPBANDL_H
#define COMPBANDL_H

#include "Compressor.h"
#include "AnalogFilter.h"


class CompBand
{
public:
    CompBand (float * efxoutl_, float * efxoutr_, double sample_rate, uint32_t intermediate_bufsize);
    ~CompBand ();
    void out (float * smpsl, float * smpr, uint32_t period);
    void setpreset (int npreset);
    void changepar (int npar, int value);
    int getpar (int npar);
    void cleanup ();

    int Ppreset;
    float outvolume;
    float level;

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


private:

    void setvolume (int Pvolume);
    void setlevel(int value);
    void setratio (int ch, int value);
    void setthres (int ch, int value);
    void setCross1 (int value);
    void setCross2 (int value);
    void setCross3 (int value);


    //Parametrii
    int Pvolume;	//Volumul or E/R
    int Plevel;
    int PLratio;
    int PMLratio;
    int PMHratio;
    int PHratio;
    int PLthres;
    int PMLthres;
    int PMHthres;
    int PHthres;


    int Cross1;
    int Cross2;
    int Cross3;

    //Parametrii reali

    AnalogFilter *lpf1l, *lpf1r, *hpf1l, *hpf1r;
    AnalogFilter *lpf2l, *lpf2r, *hpf2l, *hpf2r;
    AnalogFilter *lpf3l, *lpf3r, *hpf3l, *hpf3r;
    float * interpbuf;//buffer for filters

    Compressor *CL,*CML,*CMH,*CH;

    class FPreset *Fpre;

};

#endif
