/*
  ZynAddSubFX - a software synthesizer

  SVFilter.C - Several state-variable filters
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

#include <math.h>
#include <stdio.h>
#include "SVFilter.h"

SVFilter::SVFilter (unsigned char Ftype, float Ffreq, float Fq,
                    unsigned char Fstages, double sample_rate, float *interpbuf)
{
    stages = Fstages;
    type = Ftype;
    freq = Ffreq;
    q = Fq;
    gain = 1.0f;
    outgain = 1.0f;
    needsinterpolation = 0;
    firsttime = 1;
    fSAMPLE_RATE = sample_rate;
    if (stages >= MAX_FILTER_STAGES)
        stages = MAX_FILTER_STAGES;
    cleanup ();
    setfreq_and_q (Ffreq, Fq);
    ismp = interpbuf;
};

SVFilter::~SVFilter ()
{
};

void
SVFilter::cleanup ()
{
    for (int i = 0; i < MAX_FILTER_STAGES + 1; i++) {
        st[i].low = 0.0;
        st[i].high = 0.0;
        st[i].band = 0.0;
        st[i].notch = 0.0;
    };
    oldabovenq = 0;
    abovenq = 0;
};

void
SVFilter::computefiltercoefs ()
{
    par.f = freq / fSAMPLE_RATE * 4.0f;
    if (par.f > 0.99999)
        par.f = 0.99999f;
    par.q = 1.0f - atanf (sqrtf (q)) * 2.0f / PI;
    par.q = powf (par.q, 1.0f / (float)(stages + 1));
    par.q_sqrt = sqrtf (par.q);
};


void
SVFilter::setfreq (float frequency)
{
    if (frequency < 0.1)
        frequency = 0.1f;
    float rap = freq / frequency;
    if (rap < 1.0)
        rap = 1.0f / rap;

    oldabovenq = abovenq;
    abovenq = frequency > (fSAMPLE_RATE / 2.0f - 500.0f);

    int nyquistthresh = (abovenq ^ oldabovenq);


    if ((rap > 3.0) || (nyquistthresh != 0)) {
        //if the frequency is changed fast, it needs interpolation (now, filter and coeficients backup)
        if (firsttime == 0)
            needsinterpolation = 1;
        ipar = par;
    };
    freq = frequency;
    computefiltercoefs ();
    firsttime = 0;

};

void
SVFilter::setfreq_and_q (float frequency, float q_)
{
    q = q_;
    setfreq (frequency);
};

void
SVFilter::setq (float q_)
{
    q = q_;
    computefiltercoefs ();
};

void
SVFilter::settype (int type_)
{
    type = type_;
    computefiltercoefs ();
};

void
SVFilter::setgain (float dBgain)
{
    gain = dB2rap (dBgain);
    computefiltercoefs ();
};

void
SVFilter::setstages (int stages_)
{
    if (stages_ >= MAX_FILTER_STAGES)
        stages_ = MAX_FILTER_STAGES - 1;
    stages = stages_;
    cleanup ();
    computefiltercoefs ();
};

void
SVFilter::singlefilterout (float * smp, fstage & x, parameters & par, uint32_t period)
{
    unsigned int i;
    float *out = NULL;
    switch (type) {
    case 0:
        out = &x.low;
        break;
    case 1:
        out = &x.high;
        break;
    case 2:
        out = &x.band;
        break;
    case 3:
        out = &x.notch;
        break;
    };

    for (i = 0; i < period; i++) {
        x.low = x.low + par.f * x.band;
        x.high = par.q_sqrt * smp[i] - x.low - par.q * x.band;
        x.band = par.f * x.high + x.band;
        x.notch = x.high + x.low;

        smp[i] = *out;
    };
};

void
SVFilter::filterout (float * smp, uint32_t period)
{
    unsigned int i;

    if (needsinterpolation != 0) {
        for (i = 0; i < period; i++)
            ismp[i] = smp[i];
        for (i = 0; i < stages + 1; i++)
            singlefilterout (ismp, st[i], ipar, period);
    };

    for (i = 0; i < stages + 1; i++)
        singlefilterout (smp, st[i], par, period);

    if (needsinterpolation != 0) {
        float fPERIOD = period;
        for (i = 0; i < period; i++) {
            float x = (float) i / fPERIOD;
            smp[i] = ismp[i] * (1.0f - x) + smp[i] * x;
        };
        needsinterpolation = 0;
    };

    for (i = 0; i < period; i++)
        smp[i] *= outgain;

};
