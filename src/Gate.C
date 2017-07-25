// Based in gate_1410.c LADSPA Swh-plugins


/*
  rakarrack - a guitar effects software

 Gate.C  -  Noise Gate Effect
 Based on Steve Harris LADSPA gate.

  Copyright (C) 2008 Josep Andreu
  Author: Josep Andreu

 This program is free software; you can redistribute it and/or modify
 it under the terms of version 2 of the GNU General Public License
 as published by the Free Software Foundation.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License (version 2) for more details.

 You should have received a copy of the GNU General Public License
 (version2)  along with this program; if not, write to the Free Software
 Foundation,
 Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#include <math.h>
#include "Gate.h"


Gate::Gate (float * efxoutl_, float * efxoutr_, double samplerate, uint32_t intermediate_bufsize)
{

    efxoutl = efxoutl_;
    efxoutr = efxoutr_;

    interpbuf = new float[intermediate_bufsize];

    lpfl = new AnalogFilter (2, 22000, 1, 0, samplerate, interpbuf);
    lpfr = new AnalogFilter (2, 22000, 1, 0, samplerate, interpbuf);
    hpfl = new AnalogFilter (3, 20, 1, 0, samplerate, interpbuf);
    hpfr = new AnalogFilter (3, 20, 1, 0, samplerate, interpbuf);

    env = 0.0;
    gate = 0.0;
    fs = samplerate;
    state = CLOSED;
    hold_count = 0;

}

Gate::~Gate ()
{
    delete[] interpbuf;
    delete lpfl;
    delete lpfr;
    delete hpfl;
    delete hpfr;
}



void
Gate::cleanup ()
{
    lpfl->cleanup ();
    hpfl->cleanup ();
    lpfr->cleanup ();
    hpfr->cleanup ();
}




void
Gate::setlpf (int value)
{
    Plpf = value;
    float fr = (float)Plpf;
    lpfl->setfreq (fr);
    lpfr->setfreq (fr);
};

void
Gate::sethpf (int value)
{
    Phpf = value;
    float fr = (float)Phpf;
    hpfl->setfreq (fr);
    hpfr->setfreq (fr);
};


void
Gate::Gate_Change (int np, int value)
{

    switch (np) {

    case 1:
        Pthreshold = value;
        t_level = dB2rap ((float)Pthreshold);
        break;
    case 2:
        Prange = value;
        cut = dB2rap ((float)Prange);
        break;
    case 3:
        Pattack = value;
        a_rate = 1000.0f / ((float)Pattack * fs);
        break;
    case 4:
        Pdecay = value;
        d_rate = 1000.0f / ((float)Pdecay * fs);
        break;
    case 5:
        setlpf(value);
        break;
    case 6:
        sethpf(value);
        break;
    case 7:
        Phold = value;
        hold = (float)Phold;
        break;
    }
}

int
Gate::getpar (int np)
{

    switch (np)

    {
    case 1:
        return (Pthreshold);
        break;
    case 2:
        return (Prange);
        break;
    case 3:
        return (Pattack);
        break;
    case 4:
        return (Pdecay);
        break;
    case 5:
        return (Plpf);
        break;
    case 6:
        return (Phpf);
        break;
    case 7:
        return (Phold);
        break;

    }

    return (0);

}


void
Gate::Gate_Change_Preset (int npreset)
{

    const int PRESET_SIZE = 7;
    const int NUM_PRESETS = 3;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //0
        {0, 0, 1, 2, 6703, 76, 2},
        //-10
        {0, -10, 1, 2, 6703, 76, 2},
        //-20
        {0, -20, 1, 2, 6703, 76, 2}
    };

    if(npreset>NUM_PRESETS-1) {

        Fpre->ReadPreset(16,npreset-NUM_PRESETS+1,pdata);
        for (int n = 0; n < PRESET_SIZE; n++)
            Gate_Change(n + 1, pdata[n]);
    } else {
        for (int n = 0; n < PRESET_SIZE; n++)
            Gate_Change (n + 1, presets[npreset][n]);
    }

}



void
Gate::out (float *efxoutl, float *efxoutr, uint32_t period)
{
    unsigned i;
    float sum = 0.0f;


    lpfl->filterout (efxoutl,period);
    hpfl->filterout (efxoutl,period);
    lpfr->filterout (efxoutr,period);
    hpfr->filterout (efxoutr,period);


    for (i = 0; i < period; i++) {

        sum = fabsf (efxoutl[i]) + fabsf (efxoutr[i]);


        if (sum > env)
            env = sum;
        else
            env = sum * ENV_TR + env * (1.0f - ENV_TR);

        if (state == CLOSED) {
            if (env >= t_level)
                state = OPENING;
        } else if (state == OPENING) {
            gate += a_rate;
            if (gate >= 1.0) {
                gate = 1.0f;
                state = OPEN;
                hold_count = lrintf (hold * fs * 0.001f);
            }
        } else if (state == OPEN) {
            if (hold_count <= 0) {
                if (env < t_level) {
                    state = CLOSING;
                }
            } else
                hold_count--;

        } else if (state == CLOSING) {
            gate -= d_rate;
            if (env >= t_level)
                state = OPENING;
            else if (gate <= 0.0) {
                gate = 0.0;
                state = CLOSED;
            }
        }

        efxoutl[i] *= (cut * (1.0f - gate) + gate);
        efxoutr[i] *= (cut * (1.0f - gate) + gate);

    }
};
