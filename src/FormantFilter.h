/*
  ZynAddSubFX - a software synthesizer

  FormantFilter.h - formant filter
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

#ifndef FORMANT_FILTER_H
#define FORMANT_FILTER_H

#include "global.h"
#include "Filter_.h"
#include "AnalogFilter.h"
#include "FilterParams.h"

class FormantFilter:public Filter_
{
public:
    FormantFilter (class FilterParams * pars, float* interpbuf);//interpbuff MUST be an array greater or equal to period
    ~FormantFilter ();
    void filterout (float * smp, uint32_t period);
    void setfreq (float frequency);
    void setfreq_and_q (float frequency, float q_);
    void setq (float q_);

    void cleanup ();
private:

    void setpos (float input);


    struct {
        float freq, amp, q;	//frequency,amplitude,Q
    } formantpar[FF_MAX_VOWELS][FF_MAX_FORMANTS],
    currentformants[FF_MAX_FORMANTS];

    struct {
        unsigned char nvowel;
    } sequence[FF_MAX_SEQUENCE];

    unsigned int sequencesize, numformants, firsttime;

    float oldformantamp[FF_MAX_FORMANTS];
    float oldinput, slowinput;
    float Qfactor, formantslowness, oldQfactor;
    float vowelclearness, sequencestretch;

    float *inbuffer, *tmpbuf;

    AnalogFilter * formant[FF_MAX_FORMANTS];



};


#endif
