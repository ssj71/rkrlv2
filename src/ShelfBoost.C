/*
  Rakarrack   Audio FX software
  ShelfBoost.C - Tone Booster
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ShelfBoost.h"



ShelfBoost::ShelfBoost (float * efxoutl_, float * efxoutr_, double sample_rate, uint32_t intermediate_bufsize)
{
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;


    //default values
    Ppreset = 0;
    Pvolume = 50;
    Pstereo = 0;

    interpbuf = new float[intermediate_bufsize];
    RB1l =  new AnalogFilter(7,3200.0f,0.5f,0,sample_rate, interpbuf);
    RB1r =  new AnalogFilter(7,3200.0f,0.5f,0,sample_rate, interpbuf);


    cleanup ();

    setpreset (Ppreset);
};

ShelfBoost::~ShelfBoost ()
{
	delete RB1l;
	delete RB1r;
	delete[] interpbuf;
};

/*
 * Cleanup the effect
 */
void
ShelfBoost::cleanup ()
{

    RB1l->cleanup();
    RB1r->cleanup();

};


/*
 * Effect output
 */
void
ShelfBoost::out (float * smpsl, float * smpsr, uint32_t period)
{
    unsigned int i;


    RB1l->filterout(smpsl,period);
    if(Pstereo) RB1r->filterout(smpsr,period);


    for(i=0; i<period; i++) {
        smpsl[i]*=outvolume*u_gain;
        if(Pstereo) smpsr[i]*=outvolume*u_gain;
    }

    if(!Pstereo) memcpy(smpsr,smpsl,sizeof(float)*period);




};


/*
 * Parameter control
 */
void
ShelfBoost::setvolume (int value)
{
    Pvolume = value;
    outvolume = (float)Pvolume / 127.0f;

};

void
ShelfBoost::setpreset (int npreset)
{
    const int PRESET_SIZE = 5;
    const int NUM_PRESETS = 4;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Trebble
        {127, 64, 16000, 1, 24},
        //Mid
        {127, 64, 4400, 1, 24},
        //Bass
        {127, 64, 220, 1, 24},
        //Distortion 1
        {6, 40, 12600, 1, 127}

    };

    if(npreset>NUM_PRESETS-1) {
        Fpre->ReadPreset(34,npreset-NUM_PRESETS+1,pdata);
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, pdata[n]);
    } else {
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, presets[npreset][n]);
    }
    Ppreset = npreset;
    cleanup ();

};


void
ShelfBoost::changepar (int npar, int value)
{
    switch (npar) {
    case 0:
        setvolume (value);
        break;
    case 1:
        Pq1 = value;
        q1 = powf (30.0f, ((float)value - 64.0f) / 64.0f);
        RB1l->setq(q1);
        RB1r->setq(q1);
        break;
    case 2:
        Pfreq1 = value;
        freq1 = (float) value;
        RB1l->setfreq(freq1);
        RB1r->setfreq(freq1);
        break;
    case 3:
        Pstereo = value;
        break;
    case 4:
        Plevel = value;
        gain = .375f * (float)value;
        u_gain = 1.0f / gain;
        RB1l->setgain(gain);
        RB1r->setgain(gain);
        break;

    };
};

int
ShelfBoost::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
        return (Pq1);
        break;
    case 2:
        return (Pfreq1);
        break;
    case 3:
        return (Pstereo);
        break;
    case 4:
        return (Plevel);
        break;

    };
    return (0);			//in case of bogus parameter number
};

