/*
  ZynAddSubFX - a software synthesizer

  RBFilter.C - Several state-variable filters
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  Modified for rakarrack by Ryan Billing

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
#include "RBFilter.h"

RBFilter::RBFilter (int Ftype, float Ffreq, float Fq,
                    int Fstages, double sample_rate, float* interpbuf)
{
    stages = Fstages;
    type = Ftype;
    freq = Ffreq;
    q = Fq;
    qmode = 0;
    gain = 1.0f;
    outgain = 1.0f;
    needsinterpolation = 0;
    firsttime = 1;
    en_mix = 0;
    oldq = 0.0f;
    oldsq = 0.0f;
    oldf = 0.0f;
    hpg = lpg = bpg = 0.0f;
    fSAMPLE_RATE = sample_rate;
    if (stages >= MAX_FILTER_STAGES)
        stages = MAX_FILTER_STAGES;
    cleanup ();
    setfreq_and_q (Ffreq, Fq);
    float cSAMPLE_RATE = 1/sample_rate;
    a_smooth_tc = cSAMPLE_RATE/(cSAMPLE_RATE + 0.01f);  //10ms time constant for averaging coefficients
    b_smooth_tc = 1.0f - a_smooth_tc;
    ismp = interpbuf;
};

RBFilter::~RBFilter ()
{
};

void
RBFilter::cleanup ()
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
RBFilter::computefiltercoefs ()
{
    par.f = 2.0f * sinf(PI*freq / fSAMPLE_RATE);
    if (par.f > 0.99999f)
        par.f = 0.99999f;
    par.q = 1.0f - atanf (sqrtf (q)) * 2.0f / PI;
    par.q = powf (par.q, 1.0f / (float)(stages + 1));
    par.q_sqrt = sqrtf (par.q);

};

void
RBFilter::computefiltercoefs_hiQ ()  //potentially unstable at some settings, but better sound
{
    par.f = 2.0f * sinf(PI*freq / fSAMPLE_RATE);
    if (par.f > 0.99999f)
        par.f = 0.99999f;
    if(q<0.5f) q = 0.5f;
    par.q = 1.0f/q;
    par.q = powf (par.q, 1.0f / (float)(stages + 1));
    par.q_sqrt = 1.0f;

};

void
RBFilter::directmod (float lfo)  //potentially unstable at some settings, but better sound
{
    par.f = fabsf(lfo);  //cannot be less than 0
    if (par.f > 0.99999f)
        par.f = 0.99999f;
    //note range on input LFO should be scaled assuming the following:
    //A value of 1.0f sets a resonant frequency of SAMPLE_RATE/6.0
    //A value greater than 1 makes the filter go unstable, thus the limit.
    //Call the filter on the same sample multiple times for cheap and dirty oversampling
};
void
RBFilter::setmode (int mode)
{
    if (mode) qmode = 1;
    else qmode = 0;
}

void
RBFilter::setfreq (float frequency)
{
    if (frequency > (fSAMPLE_RATE / 2.0f - 500.0f)) frequency = fSAMPLE_RATE / 2.0f - 500.0f;
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

    if(!qmode) computefiltercoefs ();
    else computefiltercoefs_hiQ ();
    firsttime = 0;

};

void
RBFilter::setfreq_and_q (float frequency, float q_)
{
    q = q_;
    setfreq (frequency);
};

void
RBFilter::setq (float q_)
{
    q = q_;
    if(!qmode) computefiltercoefs ();
    else computefiltercoefs_hiQ ();
};

void
RBFilter::settype (int type_)
{
    type = type_;
    if(!qmode) computefiltercoefs ();
    else computefiltercoefs_hiQ ();
};

void
RBFilter::setgain (float dBgain)
{
    gain = dB2rap (dBgain);
    if(!qmode) computefiltercoefs ();
    else computefiltercoefs_hiQ ();
};

void
RBFilter::setstages (int stages_)
{
    if (stages_ >= MAX_FILTER_STAGES)
        stages_ = MAX_FILTER_STAGES - 1;
    stages = stages_;
    cleanup ();
    if(!qmode) computefiltercoefs ();
    else computefiltercoefs_hiQ ();
};
void
RBFilter::setmix (int mix, float lpmix, float bpmix, float hpmix)
{
    if(mix) {
        en_mix = 1;
    } else {
        en_mix = 0;
    }

    lpg = lpmix;
    bpg = bpmix;
    hpg = hpmix;
};


void
RBFilter::singlefilterout (float * smp, fstage & x, parameters & par, uint32_t period)
{
    iper = 1.0f/(float)period;
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

    float tmpq, tmpsq, tmpf, qdiff, sqdiff, fdiff;
    qdiff = (par.q - oldq)*iper;
    sqdiff = (par.q_sqrt - oldsq)*iper;
    fdiff = (par.f - oldf)*iper;
    tmpq = oldq;
    tmpsq = oldsq;
    tmpf = oldf;

    for (i = 0; i < period; i++) {
        tmpq += qdiff;
        tmpsq += sqdiff;
        tmpf += fdiff;   //Modulation interpolation

        x.low = x.low + tmpf * x.band;
        x.high = tmpsq * smp[i] - x.low - tmpq * x.band;
        //x.high = smp[i] - x.low - tmpq * x.band;
        x.band = tmpf * x.high + x.band;

        if(en_mix) {
            smp[i] = lpg * x.low + hpg * x.high + bpg * x.band;
        } else {
            x.notch = x.high + x.low;
            smp[i] = *out;
        }
    };

    oldf = par.f;
    oldq = par.q;
    oldsq = par.q_sqrt;


};

void
RBFilter::filterout (float * smp, uint32_t period)
{
    unsigned int i;

    if (needsinterpolation != 0) {
        for (i = 0; i < period; i++)
            ismp[i] = smp[i];
        for (i = 0; i < stages + 1; i++)
            singlefilterout (ismp, st[i], ipar, period);

        needsinterpolation = 0;
    };

    for (i = 0; i < stages + 1; i++)
        singlefilterout (smp, st[i], par, period);


    for (i = 0; i < period; i++)
        smp[i] *= outgain;

};

float
RBFilter::filterout_s (float smp)
{
    unsigned int i;

    if (needsinterpolation != 0) {
        for (i = 0; i < stages + 1; i++)
            smp=singlefilterout_s (smp, st[i], ipar);
        needsinterpolation = 0;
    };

    for (i = 0; i < stages + 1; i++)
        smp=singlefilterout_s (smp, st[i], par);


    return(smp *= outgain);

};

float inline
RBFilter::singlefilterout_s (float smp, fstage & x, parameters & par)
{
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


    oldq = b_smooth_tc*oldq + a_smooth_tc*par.q;
    oldsq = b_smooth_tc*oldsq + a_smooth_tc*par.q_sqrt;
    oldf = b_smooth_tc*oldf + a_smooth_tc*par.f;   //modulation interpolation

    x.low = x.low + oldf * x.band;
    x.high = oldsq * smp - x.low - oldq * x.band;
    x.band = oldf * x.high + x.band;

    if(en_mix) {
        smp = lpg * x.low + hpg * x.high + bpg * x.band;
    } else {
        x.notch = x.high + x.low;
        smp = *out;
    }

    oldf = par.f;
    oldq = par.q;
    oldsq = par.q_sqrt;

    return(smp);
};


