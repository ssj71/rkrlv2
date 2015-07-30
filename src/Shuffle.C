/*
  Shuffle.C - Distorsion effect

  ZynAddSubFX - a software synthesizer
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Distorsion.h"
#include "Shuffle.h"

/*
 * Waveshape (this is called by OscilGen::waveshape and Distorsion::process)
 */



Shuffle::Shuffle (float * efxoutl_, float * efxoutr_, double sample_rate, uint32_t intermediate_bufsize)
{
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;

    inputl = (float *) malloc (sizeof (float) * intermediate_bufsize);
    inputr = (float *) malloc (sizeof (float) * intermediate_bufsize);


    interpbuf = new float[intermediate_bufsize];
    lr = new AnalogFilter (6, 300.0f, .3f, 0, sample_rate,interpbuf);
    hr = new AnalogFilter (6, 8000.0f,.3f, 0, sample_rate,interpbuf);
    mlr = new AnalogFilter (6, 1200.0f,.3f, 0, sample_rate,interpbuf);
    mhr = new AnalogFilter (6, 2400.0f,.3f, 0, sample_rate,interpbuf);


    //default values
    Ppreset = 0;
    Pvolume = 50;
    PvolL = 0;
    PvolML = 0;
    PvolMH = 0;
    PvolH = 0;
    E=0;
    setpreset (Ppreset);
    cleanup ();
};

Shuffle::~Shuffle ()
{
	free(inputl);
	free(inputr);
	delete[] interpbuf;
	delete lr;
	delete hr;
	delete mlr;
	delete mhr;

};

/*
 * Cleanup the effect
 */
void
Shuffle::cleanup ()
{
    lr->cleanup ();
    hr->cleanup ();
    mlr->cleanup ();
    mhr->cleanup ();

};
/*
 * Effect output
 */
void
Shuffle::out (float * smpsl, float * smpsr, uint32_t period)
{
    unsigned int i;

    for (i = 0; i < period; i++) {

        inputl[i] = smpsl[i] + smpsr[i];
        inputr[i] = smpsl[i] - smpsr[i];
    }

    if(E) {

        lr->filterout(inputr, period);
        mlr->filterout(inputr, period);
        mhr->filterout(inputr, period);
        hr->filterout(inputr, period);
    } else {
        lr->filterout(inputl, period);
        mlr->filterout(inputl, period);
        mhr->filterout(inputl, period);
        hr->filterout(inputl, period);
    }


    for (i = 0; i < period; i++) {
        efxoutl[i]=(inputl[i]+inputr[i]-smpsl[i])*.333333f;
        efxoutr[i]=(inputl[i]-inputr[i]-smpsr[i])*.333333f;

    }


};


/*
 * Parameter control
 */
void
Shuffle::setvolume (int value)
{
    Pvolume = value;
    outvolume = (float)Pvolume / 128.0f;
};

void
Shuffle::setCross1 (int value)
{
    Cross1 = value;
    lr->setfreq ((float)value);

};

void
Shuffle::setCross2 (int value)
{
    Cross2 = value;
    mlr->setfreq ((float)value);

};


void
Shuffle::setCross3 (int value)
{
    Cross3 = value;
    mhr->setfreq ((float)value);

};

void
Shuffle::setCross4 (int value)
{
    Cross4 = value;
    hr->setfreq ((float)value);

};

void
Shuffle::setGainL(int value)
{
    PvolL = value+64;
    volL = 30.0f * ((float)PvolL - 64.0f) / 64.0f;
    lr->setgain(volL);
}

void
Shuffle::setGainML(int value)
{
    PvolML = value+64;
    volML = 30.0f * ((float)PvolML - 64.0f) / 64.0f;;
    mlr->setgain(volML);
}

void
Shuffle::setGainMH(int value)
{
    PvolMH = value+64;
    volMH = 30.0f * ((float)PvolMH - 64.0f) / 64.0f;;
    mhr->setgain(volMH);
}

void
Shuffle::setGainH(int value)
{
    PvolH = value+64;
    volH = 30.0f * ((float)PvolH - 64.0f) / 64.0f;;
    hr->setgain(volH);
}


void
Shuffle::setpreset (int npreset)
{
    const int PRESET_SIZE = 11;
    const int NUM_PRESETS = 4;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Shuffle 1
        {64, 10, 0, 0, 0,600, 1200,2000, 6000,-14, 1},
        //Shuffle 2
        {64, 0, 0, 0, 0, 120, 1000,2400, 8000,-7, 1},
        //Shuffle 3
        {64, 0, 0, 0, 0, 60, 1800, 3700, 12000, 7, 0},
        //Remover
        {0, 17, 0, 7, 5, 600, 1200, 2000, 13865, -45, 1}
    };
    if(npreset>NUM_PRESETS-1) {
        Fpre->ReadPreset(26,npreset-NUM_PRESETS+1,pdata);
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
Shuffle::changepar (int npar, int value)
{
    switch (npar) {
    case 0:
        setvolume (value);
        break;
    case 1:
        setGainL(value);
        break;
    case 2:
        setGainML(value);
        break;
    case 3:
        setGainMH(value);
        break;
    case 4:
        setGainH(value);
        break;
    case 5:
        setCross1 (value);
        break;
    case 6:
        setCross2 (value);
        break;
    case 7:
        setCross3 (value);
        break;
    case 8:
        setCross4 (value);
        break;
    case 9:
        PQ = value;
        value +=64;
        tmp = powf (30.0f, ((float)value - 64.0f) / 64.0f);
        lr->setq(tmp);
        mlr->setq(tmp);
        mhr->setq(tmp);
        hr->setq(tmp);
        break;
    case 10:
        E=value;
        break;
    };
};

int
Shuffle::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
        return (PvolL-64);
        break;
    case 2:
        return (PvolML-64);
        break;
    case 3:
        return (PvolMH-64);
        break;
    case 4:
        return (PvolH-64);
        break;
    case 5:
        return (Cross1);
        break;
    case 6:
        return (Cross2);
        break;
    case 7:
        return (Cross3);
        break;
    case 8:
        return (Cross4);
        break;
    case 9:
        return (PQ);
        break;
    case 10:
        return (E);

    };
    return (0);			//in case of bogus parameter number
};

