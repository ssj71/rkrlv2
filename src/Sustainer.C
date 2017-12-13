/*
  Rakarrack Guitar FX

  Sustainer.C - Simple compressor/sustainer effect with easy interface, minimal controls
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Sustainer.h"

Sustainer::Sustainer (float * efxoutl_, float * efxoutr_, double sample_rate)
{
	float cSAMPLE_RATE = 1/sample_rate;
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;

    Pvolume = 64;
    Psustain = 64;
    fsustain = 0.5f;
    level = 0.5f;

    float tmp = 0.01f;  //10 ms decay time on peak detectorS
    prls = 1.0f - (cSAMPLE_RATE/(cSAMPLE_RATE + tmp));

    tmp = 0.05f; //50 ms att/rel on compressor
    calpha =  cSAMPLE_RATE/(cSAMPLE_RATE + tmp);
    cbeta = 1.0f - calpha;
    cthresh = 0.25f;
    cratio = 0.25f;

    timer = 0;
    hold = (int) (sample_rate*0.0125);  //12.5ms
    cleanup ();
};

Sustainer::~Sustainer ()
{

};

/*
 * Cleanup the effect
 */
void
Sustainer::cleanup ()
{
    compeak = 0.0f;
    compenv = 0.0f;
    oldcompenv = 0.0f;
    cpthresh = cthresh; //dynamic threshold
};




/*
 * Effect output
 */
void
Sustainer::out (float * smpsl, float * smpsr, uint32_t period)
{
    unsigned int i;
    float auxtempl = 0.0f;
    float auxtempr = 0.0f;
    float auxcombi = 0.0f;

    for (i = 0; i<period; i++) {  //apply compression to auxresampled
        auxtempl = input * smpsl[i];
        auxtempr = input * smpsr[i];
        auxcombi = 0.5f * (auxtempl + auxtempr);
        if(fabs(auxcombi) > compeak) {
            compeak = fabs(auxcombi);   //First do peak detection on the signal
            timer = 0;
        }
        if(timer>hold) {
            compeak *= prls;
            timer--;
        }
        timer++;
        compenv = cbeta * oldcompenv + calpha * compeak;       //Next average into envelope follower
        oldcompenv = compenv;

        if(compenv > cpthresh) {                              //if envelope of signal exceeds thresh, then compress
            compg = cpthresh + cpthresh*(compenv - cpthresh)/compenv;
            cpthresh = cthresh + cratio*(compg - cpthresh);   //cpthresh changes dynamically
            tmpgain = compg/compenv;
        } else {
            tmpgain = 1.0f;
        }

        if(compenv < cpthresh) cpthresh = compenv;
        if(cpthresh < cthresh) cpthresh = cthresh;

        smpsl[i] = auxtempl * tmpgain * level;
        smpsr[i] = auxtempr * tmpgain * level;
    };
    //End compression
};


/*
 * Parameter control
 */


void
Sustainer::setpreset (int npreset)
{
    const int PRESET_SIZE = 2;
    const int NUM_PRESETS = 3;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Moderate
        {79, 54},
        //Extreme
        {16, 127},
        //Mild
        {120, 15},

    };

    if(npreset>NUM_PRESETS-1) {
        Fpre->ReadPreset(36,npreset-NUM_PRESETS+1, pdata);
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, pdata[n]);
    } else {
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, presets[npreset][n]);
    }
    Ppreset = npreset;
};


void
Sustainer::changepar (int npar, int value)
{
    switch (npar) {
    case 0:
        Pvolume = value;
        level = dB2rap(-30.0f * (1.0f - ((float) Pvolume/127.0f)));
        break;
    case 1:
        Psustain = value;
        fsustain =  (float) Psustain/127.0f;
        cratio = 1.25f - fsustain;
        input = dB2rap (42.0f * fsustain - 6.0f);
        cthresh = 0.25 + fsustain;
        break;

    };
};

int
Sustainer::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
        return (Psustain);
        break;
    };
    return (0);			//in case of bogus parameter number
};

