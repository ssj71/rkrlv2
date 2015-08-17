/*
  Rakarrack Guitar FX

  Sustainer.h - Simple compressor/sustainer effect with easy interface, minimal controls
  Copyright (C) 2010 Ryan Billing
  Author: Ryan Billing

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 3 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#ifndef SUSTAINER_H
#define SUSTAINER_H

#include "global.h"

class Sustainer
{
public:
    Sustainer (float * efxoutl_, float * efxoutr_m, double sample_rate);
    ~Sustainer ();
    void cleanup ();

    void out (float * smpsl, float * smpr, uint32_t period);
    void changepar (int npar, int value);
    int getpar (int npar);
    void setpreset (int npreset);

    int Ppreset;

    float *efxoutl;
    float *efxoutr;

private:
    //Parametrii
    int Pvolume;	//Output Level
    int Psustain;	//Compression amount

    int timer, hold;
    float level, fsustain, input,tmpgain;
    float prls, compeak, compg, compenv, oldcompenv, calpha, cbeta, cthresh, cratio, cpthresh;

    class FPreset *Fpre;
};


#endif


