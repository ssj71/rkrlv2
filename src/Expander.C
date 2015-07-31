
/*
  rakarrack - a guitar effects software

 Expander.C  -  Noise Gate Effect

  Copyright (C) 2010 Ryan Billing & Josep Andreu
  Author: Ryan Billing & Josep Andreu
  Adapted from swh-plugins Noise Gate by Steve Harris

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
#include "Expander.h"


Expander::Expander (float * efxoutl_, float * efxoutr_, double sample_rate, uint32_t intermediate_bufsize)
{

    efxoutl = efxoutl_;
    efxoutr = efxoutr_;


    interpbuf = new float[intermediate_bufsize];
    lpfl = new AnalogFilter (2, 22000, 1, 0, sample_rate, interpbuf);
    lpfr = new AnalogFilter (2, 22000, 1, 0, sample_rate, interpbuf);
    hpfl = new AnalogFilter (3, 20, 1, 0, sample_rate, interpbuf);
    hpfr = new AnalogFilter (3, 20, 1, 0, sample_rate, interpbuf);

    env = 0.0;
    oldgain = 0.0;
    efollower = 0;
    fs = sample_rate;

    Expander_Change_Preset(0);


}

Expander::~Expander ()
{
	delete[] interpbuf;
	delete lpfl;
	delete lpfr;
	delete hpfl;
	delete hpfr;
}



void
Expander::cleanup ()
{
    lpfl->cleanup ();
    hpfl->cleanup ();
    lpfr->cleanup ();
    hpfr->cleanup ();
    oldgain = 0.0f;

}




void
Expander::setlpf (int value)
{
    Plpf = value;
    float fr = (float)Plpf;
    lpfl->setfreq (fr);
    lpfr->setfreq (fr);
};

void
Expander::sethpf (int value)
{
    Phpf = value;
    float fr = (float)Phpf;
    hpfl->setfreq (fr);
    hpfr->setfreq (fr);
};


void
Expander::Expander_Change (int np, int value)
{

    switch (np) {

    case 1:
        Pthreshold = value;
        tfactor = dB2rap (-((float) Pthreshold));
        tlevel = 1.0f/tfactor;
        break;
    case 2:
        Pshape = value;
        sfactor = dB2rap ((float)Pshape/2);
        sgain = expf(-sfactor);
        break;
    case 3:
        Pattack = value;
        a_rate = 1000.0f/((float)Pattack * fs);
        break;
    case 4:
        Pdecay = value;
        d_rate = 1000.0f/((float)Pdecay * fs);
        break;
    case 5:
        setlpf(value);
        break;
    case 6:
        sethpf(value);
        break;
    case 7:
        Plevel = value;
        level = dB2rap((float) value/6.0f);
        break;

    }


}

int
Expander::getpar (int np)
{

    switch (np)

    {
    case 1:
        return (Pthreshold);
        break;
    case 2:
        return (Pshape);
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
        return (Plevel);
        break;
    }

    return (0);

}


void
Expander::Expander_Change_Preset (int npreset)
{

    const int PRESET_SIZE = 7;
    const int NUM_PRESETS = 3;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {

        //Noise Gate
        {-50, 20, 50, 50, 3134, 76, 0},
        //Boost Gate
        {-55, 30, 50, 50, 1441, 157, 50},
        //Treble swell
        {-30, 9, 950, 25, 6703, 526, 90}
    };

    if(npreset>NUM_PRESETS-1) {
        Fpre->ReadPreset(25,npreset-NUM_PRESETS+1,pdata);
        for (int n = 0; n < PRESET_SIZE; n++)
            Expander_Change (n+1, pdata[n]);
    } else {
        for (int n = 0; n < PRESET_SIZE; n++)
            Expander_Change (n + 1, presets[npreset][n]);
    }

}



void
Expander::out (float *efxoutl, float *efxoutr, uint32_t period)
{


    unsigned int i;
    float delta = 0.0f;
    float expenv = 0.0f;


    lpfl->filterout (efxoutl, period);
    hpfl->filterout (efxoutl, period);
    lpfr->filterout (efxoutr, period);
    hpfr->filterout (efxoutr, period);


    for (i = 0; i < period; i++) {

        delta = 0.5f*(fabsf (efxoutl[i]) + fabsf (efxoutr[i])) - env;    //envelope follower from Compressor.C
        if (delta > 0.0)
            env += a_rate * delta;
        else
            env += d_rate * delta;

        //End envelope power detection

        if (env > tlevel) env = tlevel;
        expenv = sgain * (expf(env*sfactor*tfactor) - 1.0f);		//Envelope waveshaping

        gain = (1.0f - d_rate) * oldgain + d_rate * expenv;
        oldgain = gain;				//smooth it out a little bit

        if(efollower) {
            efxoutl[i] = gain;
            efxoutr[i] += gain;
        } else {
            efxoutl[i] *= gain*level;
            efxoutr[i] *= gain*level;
        }

    }



};
