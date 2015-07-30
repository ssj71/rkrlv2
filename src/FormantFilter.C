/*
  ZynAddSubFX - a software synthesizer

  FormantFilter.C - formant filters
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
#include "FormantFilter.h"

FormantFilter::FormantFilter (FilterParams * pars, float* interpbuf)
{
    numformants = pars->Pnumformants;
    for (unsigned int i = 0; i < numformants; i++)
        formant[i] = new AnalogFilter (4 /*BPF*/, 1000.0f, 10.0f, pars->Pstages, pars->fSAMPLE_RATE, interpbuf);
    cleanup ();
    inbuffer = new float[pars->intermediate_bufsize];
    tmpbuf = new float[pars->intermediate_bufsize];

    for (int j = 0; j < FF_MAX_VOWELS; j++)
        for (unsigned int i = 0; i < numformants; i++) {
            formantpar[j][i].freq =
                pars->getformantfreq (pars->Pvowels[j].formants[i].freq);
            formantpar[j][i].amp =
                pars->getformantamp (pars->Pvowels[j].formants[i].amp);
            formantpar[j][i].q =
                pars->getformantq (pars->Pvowels[j].formants[i].q);
        };
    for (int i = 0; i < FF_MAX_FORMANTS; i++)
        oldformantamp[i] = 1.0;
    for (unsigned int i = 0; i < numformants; i++) {
        currentformants[i].freq = 1000.0f;
        currentformants[i].amp = 1.0f;
        currentformants[i].q = 2.0f;
    };

    formantslowness = powf (1.0f - ((float)pars->Pformantslowness / 128.0f), 3.0f);

    sequencesize = pars->Psequencesize;
    if (sequencesize == 0)
        sequencesize = 1;
    for (unsigned int k = 0; k < sequencesize; k++)
        sequence[k].nvowel = pars->Psequence[k].nvowel;

    vowelclearness = powf (10.0f, ((float)pars->Pvowelclearness - 32.0f) / 48.0f);

    sequencestretch = powf (0.1f, ((float)pars->Psequencestretch - 32.0f) / 48.0f);
    if (pars->Psequencereversed)
        sequencestretch *= -1.0f;

    outgain = dB2rap (pars->getgain ());

    oldinput = -1.0f;
    Qfactor = 1.0f;
    oldQfactor = Qfactor;
    firsttime = 1;
};

FormantFilter::~FormantFilter ()
{
    for (unsigned int i = 0; i < numformants; i++)
        delete (formant[i]);
    delete (inbuffer);
    delete (tmpbuf);
};




void
FormantFilter::cleanup ()
{
    for (unsigned int i = 0; i < numformants; i++)
        formant[i]->cleanup ();
};

void
FormantFilter::setpos (float input)
{
    int p1, p2;

    if (firsttime != 0)
        slowinput = input;
    else
        slowinput = slowinput * (1.0f - formantslowness) + input * formantslowness;

    if ((fabsf (oldinput - input) < 0.001)
            && (fabsf (slowinput - input) < 0.001)
            && (fabsf (Qfactor - oldQfactor) < 0.001)) {
//      oldinput=input; daca setez asta, o sa faca probleme la schimbari foarte lente
        firsttime = 0;
        return;
    } else
        oldinput = input;


    float pos = fmodf (input * sequencestretch, 1.0f);
    if (pos < 0.0)
        pos += 1.0f;

    F2I (pos * (float)sequencesize, p2);
    p1 = p2 - 1;
    if (p1 < 0)
        p1 += sequencesize;

    pos = fmodf (pos * (float)sequencesize, 1.0f);
    if (pos < 0.0)
        pos = 0.0f;
    else if (pos > 1.0)
        pos = 1.0f;
    pos =
        (atanf ((pos * 2.0f - 1.0f) * vowelclearness) / atanf (vowelclearness) +
         1.0f) * 0.5f;

    p1 = sequence[p1].nvowel;
    p2 = sequence[p2].nvowel;

    if (firsttime != 0) {
        for (unsigned int i = 0; i < numformants; i++) {
            currentformants[i].freq =
                formantpar[p1][i].freq * (1.0f - pos) +
                formantpar[p2][i].freq * pos;
            currentformants[i].amp =
                formantpar[p1][i].amp * (1.0f - pos) + formantpar[p2][i].amp * pos;
            currentformants[i].q =
                formantpar[p1][i].q * (1.0f - pos) + formantpar[p2][i].q * pos;
            formant[i]->setfreq_and_q (currentformants[i].freq,
                                       currentformants[i].q * Qfactor);
            oldformantamp[i] = currentformants[i].amp;
        };
        firsttime = 0;
    } else {
        for (unsigned int i = 0; i < numformants; i++) {
            currentformants[i].freq =
                currentformants[i].freq * (1.0f - formantslowness) +
                (formantpar[p1][i].freq * (1.0f - pos) +
                 formantpar[p2][i].freq * pos) * formantslowness;

            currentformants[i].amp =
                currentformants[i].amp * (1.0f - formantslowness) +
                (formantpar[p1][i].amp * (1.0f - pos) +
                 formantpar[p2][i].amp * pos) * formantslowness;

            currentformants[i].q =
                currentformants[i].q * (1.0f - formantslowness) +
                (formantpar[p1][i].q * (1.0f - pos) +
                 formantpar[p2][i].q * pos) * formantslowness;

            formant[i]->setfreq_and_q (currentformants[i].freq,
                                       currentformants[i].q * Qfactor);
        };
    };

    oldQfactor = Qfactor;
};

void
FormantFilter::setfreq (float frequency)
{
    setpos (frequency);
};

void
FormantFilter::setq (float q_)
{
    Qfactor = q_;
    for (unsigned int i = 0; i < numformants; i++)
        formant[i]->setq (Qfactor * currentformants[i].q);
};

void
FormantFilter::setfreq_and_q (float frequency, float q_)
{
    Qfactor = q_;
    setpos (frequency);
};


void
FormantFilter::filterout (float * smp, uint32_t period)
{
    unsigned int i, j;
    for (i = 0; i < period; i++) {
        inbuffer[i] = smp[i];
        smp[i] = 0.0;
    };

    for (j = 0; j < numformants; j++) {
        for (i = 0; i < period; i++)
            tmpbuf[i] = inbuffer[i] * outgain;
        formant[j]->filterout (tmpbuf, period);

        if (ABOVE_AMPLITUDE_THRESHOLD
                (oldformantamp[j], currentformants[j].amp))
            for (i = 0; i < period; i++)
                smp[i] +=
                    tmpbuf[i] * INTERPOLATE_AMPLITUDE (oldformantamp[j],
                                                       currentformants[j].amp, i,
                                                       period);
        else
            for (i = 0; i < period; i++)
                smp[i] += tmpbuf[i] * currentformants[j].amp;
        oldformantamp[j] = currentformants[j].amp;
    };
};
