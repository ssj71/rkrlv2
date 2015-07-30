/*
  ZynAddSubFX - a software synthesizer

  EQ.h - EQ Effect
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

#ifndef EQ_H
#define EQ_H

#include "global.h"
#include "AnalogFilter.h"

class EQ
{
public:
    EQ (float * efxoutl_, float * efxoutr_, double samplerate, uint32_t intermediate_bufsize);
    ~EQ ();
    void out (float * smpsl, float * smpr, uint32_t period);
    void setpreset (int npreset);
    void changepar (int npar, int value);
    int getpar (int npar);
    void cleanup ();
    float getfreqresponse (float freq);
    void setvolume (int Pvolume);

    int Ppreset;
    int Pvolume;	//Volumul

    float outvolume;		//this is the volume of effect and is public because need it in system effect. The out volume of

    float *efxoutl;
    float *efxoutr;

    float * interpbuf;//buffer for filter

    struct {
        //parameters
        int Ptype, Pfreq, Pgain, Pq, Pstages;
        //internal values
        AnalogFilter *l, *r;
    } filter[MAX_EQ_BANDS];

};


#endif
