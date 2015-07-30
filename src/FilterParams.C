/*
  ZynAddSubFX - a software synthesizer

  FilterParams.C - Parameters for filter
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
#include <stdlib.h>
#include "FilterParams.h"

FilterParams::FilterParams (unsigned char Ptype_, unsigned char Pfreq_,
                            unsigned char Pq_, double sample_rate, uint32_t ibufsz)
{

    // setpresettype("Pfilter");
    Dtype = Ptype_;
    Dfreq = Pfreq_;
    Dq = Pq_;
    SAMPLE_RATE = sample_rate;
    fSAMPLE_RATE = sample_rate;
    intermediate_bufsize = ibufsz;

    changed = false;
    defaults ();
};

FilterParams::~FilterParams ()
{
};


void
FilterParams::defaults ()
{
    Ptype = Dtype;
    Pfreq = Dfreq;
    Pq = Dq;

    Pstages = 0;
    Pfreqtrack = 64;
    Pgain = 64;
    Pcategory = 0;

    Pnumformants = 3;
    Pformantslowness = 64;
    for (int j = 0; j < FF_MAX_VOWELS; j++) {
        defaults (j);
    };

    Psequencesize = 3;
    for (int i = 0; i < FF_MAX_SEQUENCE; i++)
        Psequence[i].nvowel = (unsigned char) i % FF_MAX_VOWELS;

    Psequencestretch = 40;
    Psequencereversed = 0;
    Pcenterfreq = 64;		//1 kHz
    Poctavesfreq = 64;
    Pvowelclearness = 64;
};

void
FilterParams::defaults (int n)
{
    int j = n;
    for (int i = 0; i < FF_MAX_FORMANTS; i++) {
        Pvowels[j].formants[i].freq = (unsigned char) (RND*127.0);	//some random freqs
        Pvowels[j].formants[i].q = 64;
        Pvowels[j].formants[i].amp = 127;
    };
};


/*
 * Get the parameters from other FilterParams
 */

void
FilterParams::getfromFilterParams (FilterParams * pars)
{
    defaults ();

    if (pars == NULL)
        return;

    Ptype = pars->Ptype;
    Pfreq = pars->Pfreq;
    Pq = pars->Pq;

    Pstages = pars->Pstages;
    Pfreqtrack = pars->Pfreqtrack;
    Pgain = pars->Pgain;
    Pcategory = pars->Pcategory;

    Pnumformants = pars->Pnumformants;
    Pformantslowness = pars->Pformantslowness;
    for (int j = 0; j < FF_MAX_VOWELS; j++) {
        for (int i = 0; i < FF_MAX_FORMANTS; i++) {
            Pvowels[j].formants[i].freq = pars->Pvowels[j].formants[i].freq;
            Pvowels[j].formants[i].q = pars->Pvowels[j].formants[i].q;
            Pvowels[j].formants[i].amp = pars->Pvowels[j].formants[i].amp;
        };
    };

    Psequencesize = pars->Psequencesize;
    for (int i = 0; i < FF_MAX_SEQUENCE; i++)
        Psequence[i].nvowel = pars->Psequence[i].nvowel;

    Psequencestretch = pars->Psequencestretch;
    Psequencereversed = pars->Psequencereversed;
    Pcenterfreq = pars->Pcenterfreq;
    Poctavesfreq = pars->Poctavesfreq;
    Pvowelclearness = pars->Pvowelclearness;
};


/*
 * Parameter control
 */
float FilterParams::getfreq ()
{
    return (((float)Pfreq / 64.0f - 1.0f) * 5.0f);
};

float FilterParams::getq ()
{
    return (expf (powf ((float) Pq / 127.0f, 2) * logf (1000.0f)) - 0.9f);
};

float FilterParams::getfreqtracking (float notefreq)
{
    return (logf (notefreq / 440.0f) * ((float)Pfreqtrack - 64.0f) / (64.0f * LOG_2));
};

float FilterParams::getgain ()
{
    return (((float)Pgain / 64.0f - 1.0f) * 30.0f);	//-30..30dB
};

/*
 * Get the center frequency of the formant's graph
 */
float FilterParams::getcenterfreq ()
{
    return (10000.0f * powf (10.0f, -(1.0f - (float)Pcenterfreq / 127.0f) * 2.0f));
};

/*
 * Get the number of octave that the formant functions applies to
 */
float FilterParams::getoctavesfreq ()
{
    return (0.25f + 10.0f * (float)Poctavesfreq / 127.0f);
};

/*
 * Get the frequency from x, where x is [0..1]
 */
float FilterParams::getfreqx (float x)
{
    if (x > 1.0)
        x = 1.0f;
    float
    octf = powf (2.0f, getoctavesfreq ());
    return (getcenterfreq () / sqrtf (octf) * powf (octf, x));
};

/*
 * Get the x coordinate from frequency (used by the UI)
 */
float FilterParams::getfreqpos (float freq)
{
    return ((logf (freq) -
             logf (getfreqx (0.0))) / logf (2.0f) / getoctavesfreq ());
};


/*
 * Get the freq. response of the formant filter
 */
void
FilterParams::formantfilterH (int nvowel, int nfreqs, float * freqs)
{
    float c[3], d[3];
    float filter_freq, filter_q, filter_amp;
    float omega, sn, cs, alpha;

    for (int i = 0; i < nfreqs; i++)
        freqs[i] = 0.0;

    //for each formant...
    for (int nformant = 0; nformant < Pnumformants; nformant++) {
        //compute formant parameters(frequency,amplitude,etc.)
        filter_freq = getformantfreq (Pvowels[nvowel].formants[nformant].freq);
        filter_q = getformantq (Pvowels[nvowel].formants[nformant].q) * getq ();
        if (Pstages > 0)
            filter_q =
                (filter_q > 1.0 ? powf (filter_q, 1.0f / ((float)Pstages + 1)) : filter_q);

        filter_amp = getformantamp (Pvowels[nvowel].formants[nformant].amp);


        if (filter_freq <= (SAMPLE_RATE / 2 - 100.0)) {
            omega = 2.0f * PI * filter_freq / fSAMPLE_RATE;
            sn = sinf (omega);
            cs = cosf (omega);
            alpha = sn / (2.0f * filter_q);
            float tmp = 1.0f + alpha;
            c[0] = alpha / tmp * sqrtf (filter_q + 1.0f);
            c[1] = 0;
            c[2] = -alpha / tmp * sqrtf (filter_q + 1.0f);
            d[1] = -2.0f * cs / tmp * (-1.0f);
            d[2] = (1.0f - alpha) / tmp * (-1.0f);
        } else
            continue;


        for (int i = 0; i < nfreqs; i++) {
            float freq = getfreqx ((float)i / (float) nfreqs);
            if (freq > SAMPLE_RATE / 2) {
                for (int tmp = i; tmp < nfreqs; tmp++)
                    freqs[tmp] = 0.0;
                break;
            };
            float fr = freq / fSAMPLE_RATE * PI * 2.0f;
            float x = c[0], y = 0.0f;
            for (int n = 1; n < 3; n++) {
                x += cosf ((float)n * fr) * c[n];
                y -= sinf ((float)n * fr) * c[n];
            };
            float h = x * x + y * y;
            x = 1.0f;
            y = 0.0;
            for (int n = 1; n < 3; n++) {
                x -= cosf ((float)n * fr) * d[n];
                y += sinf ((float)n * fr) * d[n];
            };
            h = h / (x * x + y * y);

            freqs[i] += powf (h, ((float)Pstages + 1.0f) / 2.0f) * filter_amp;
        };
    };
    for (int i = 0; i < nfreqs; i++) {
        if (freqs[i] > 0.000000001f)
            freqs[i] = rap2dB (freqs[i]) + getgain ();
        else
            freqs[i] = -90.0f;
    };

};

/*
 * Transforms a parameter to the real value
 */
float FilterParams::getformantfreq (unsigned char freq)
{
    float
    result = getfreqx ((float)freq / 127.0f);
    return (result);
};

float FilterParams::getformantamp (unsigned char amp)
{
    float
    result = powf (0.1f, (1.0f - (float)amp / 127.0f) * 4.0f);
    return (result);
};

float FilterParams::getformantq (unsigned char q)
{
    //temp
    float
    result = powf (25.0f, ((float)q - 32.0f) / 64.0f);
    return (result);
};
