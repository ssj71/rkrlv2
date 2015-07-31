/*
  ZynAddSubFX - a software synthesizer

  Filter.C - Filters, uses analog,formant,etc. filters
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

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

#include <math.h>
#include <stdio.h>

#include "Filter.h"

Filter::Filter (FilterParams * pars)
{
    unsigned char Ftype = pars->Ptype;
    unsigned char Fstages = pars->Pstages;

    category = pars->Pcategory;

    interpbuf = new float[pars->intermediate_bufsize];

    switch (category) {
    case 1:
        filter = new FormantFilter (pars, interpbuf);
        break;
    case 2:
        filter = new SVFilter(Ftype, 1000.0f, pars->getq (), Fstages, pars->fSAMPLE_RATE, interpbuf);
        filter->outgain = dB2rap (pars->getgain ());
        if (filter->outgain > 1.0f)
            filter->outgain = sqrtf (filter->outgain);
        break;
    default:
        filter = new AnalogFilter (Ftype, 1000.0f, pars->getq (), Fstages, pars->fSAMPLE_RATE, interpbuf);
        if ((Ftype >= 6) && (Ftype <= 8))
            filter->setgain (pars->getgain ());
        else
            filter->outgain = dB2rap (pars->getgain ());
        break;
    };
};

Filter::~Filter ()
{
	delete[] interpbuf;
};

void
Filter::filterout (float * smp, uint32_t period)
{
    filter->filterout (smp, period);
};

void
Filter::setfreq (float frequency)
{
    filter->setfreq (frequency);
};

void
Filter::setfreq_and_q (float frequency, float q_)
{
    filter->setfreq_and_q (frequency, q_);
};

void
Filter::setq (float q_)
{
    filter->setq (q_);
};

float Filter::getrealfreq (float freqpitch)
{
    if ((category == 0) || (category == 2))
        return (powf (2.0f, freqpitch + 9.96578428f));	//log2(1000)=9.95748
    else
        return (freqpitch);
};
