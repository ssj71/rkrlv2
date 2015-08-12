/*
  ZynAddSubFX - a software synthesizer

  Distorsion.C - Distorsion effect
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  Modified for rakarrack by Josep Andreu & Ryan Billing

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
#include "NewDist.h"

/*
 * Waveshape (this is called by OscilGen::waveshape and Distorsion::process)
 */

// does anyone else see how funny this class is? say it out loud. Perhaps this will become so popular they will start a new_dist colony ;)

NewDist::NewDist (float * efxoutl_, float * efxoutr_, double sample_rate, uint32_t intermediate_bufsize,
		int wave_res, int wave_upq, int wave_dnq)
{
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;

    octoutl = (float *) malloc (sizeof (float) * intermediate_bufsize);
    octoutr = (float *) malloc (sizeof (float) * intermediate_bufsize);

    unsigned int i;
    for(i=0;i<intermediate_bufsize;i++)
    {
    	octoutl[i] = octoutr[i] = 0;
    }

    interpbuf = new float[intermediate_bufsize];
    lpfl = new AnalogFilter (2, 22000, 1, 0, sample_rate, interpbuf);
    lpfr = new AnalogFilter (2, 22000, 1, 0, sample_rate, interpbuf);
    hpfl = new AnalogFilter (3, 20, 1, 0, sample_rate, interpbuf);
    hpfr = new AnalogFilter (3, 20, 1, 0, sample_rate, interpbuf);
    blockDCl = new AnalogFilter (2, 75.0f, 1, 0, sample_rate, interpbuf);
    blockDCr = new AnalogFilter (2, 75.0f, 1, 0, sample_rate, interpbuf);
    wshapel = new Waveshaper(sample_rate, wave_res, wave_upq, wave_dnq, intermediate_bufsize);
    wshaper = new Waveshaper(sample_rate, wave_res, wave_upq, wave_dnq, intermediate_bufsize);

    blockDCl->setfreq (75.0f);
    blockDCr->setfreq (75.0f);

    DCl = new AnalogFilter (3, 30, 1, 0, sample_rate, interpbuf);
    DCr = new AnalogFilter (3, 30, 1, 0, sample_rate, interpbuf);
    DCl->setfreq (30.0f);
    DCr->setfreq (30.0f);




    filterpars = new FilterParams (0, 64, 64, sample_rate, intermediate_bufsize);

    filterpars->Pcategory = 2;
    filterpars->Ptype = 0;
    filterpars->Pfreq = 72;
    filterpars->Pq = 76;
    filterpars->Pstages = 0;
    filterpars->Pgain = 76;



    filterl = new Filter (filterpars);
    filterr = new Filter (filterpars);

    Ppreset=0;
    setpreset (Ppreset);


    //default values
    Ppreset = 0;
    Pvolume = 50;
    Plrcross = 40;
    Pdrive = 1;
    Plevel = 32;
    Ptype = 0;
    Pnegate = 0;
    Plpf = 127;
    Phpf = 0;
    Prfreq = 64;
    Pprefiltering = 0;
    Poctave = 0;
    togglel = 1.0;
    octave_memoryl = -1.0;
    toggler = 1.0;
    octave_memoryr = -1.0;
    octmix=0.0;




    cleanup ();
};

NewDist::~NewDist ()
{
    free(octoutl);
    free(octoutr);

    delete[] interpbuf;
    delete lpfl;
    delete lpfr;
    delete hpfl;
    delete hpfr;
    delete blockDCl;
    delete blockDCr;
    delete wshapel;
    delete wshaper;

    delete DCl;
    delete DCr;

    delete filterpars;
    delete filterl;
    delete filterr;

};

/*
 * Cleanup the effect
 */
void
NewDist::cleanup ()
{
    lpfl->cleanup ();
    hpfl->cleanup ();
    lpfr->cleanup ();
    hpfr->cleanup ();
    blockDCr->cleanup ();
    blockDCl->cleanup ();
    DCl->cleanup();
    DCr->cleanup();


};


/*
 * Apply the filters
 */

void
NewDist::applyfilters (float * efxoutl, float * efxoutr, uint32_t period)
{
    lpfl->filterout(efxoutl, period);
    hpfl->filterout(efxoutl, period);
    lpfr->filterout(efxoutr, period);
    hpfr->filterout(efxoutr, period);

};


/*
 * Effect output
 */
void
NewDist::out (float * smpsl, float * smpsr, uint32_t period)
{
    unsigned int i;
    float l, r, lout, rout;

    float inputvol = .5f;

    if (Pnegate != 0)
        inputvol *= -1.0f;


    if (Pprefiltering != 0)
        applyfilters (smpsl, smpsr, period);

    //no optimised, yet (no look table)


    wshapel->waveshapesmps (period, smpsl, Ptype, Pdrive, 2);
    wshaper->waveshapesmps (period, smpsr, Ptype, Pdrive, 2);




    memcpy(efxoutl,smpsl,period * sizeof(float));
    memcpy(efxoutr,smpsl,period * sizeof(float));





    if (octmix > 0.01f) {
        for (i = 0; i < period; i++) {
            lout = efxoutl[i];
            rout = efxoutr[i];

            if ( (octave_memoryl < 0.0f) && (lout > 0.0f) ) togglel *= -1.0f;
            octave_memoryl = lout;

            if ( (octave_memoryr < 0.0f) && (rout > 0.0f) ) toggler *= -1.0f;
            octave_memoryr = rout;

            octoutl[i] = lout *  togglel;
            octoutr[i] = rout *  toggler;

        }


        blockDCr->filterout (octoutr, period);
        blockDCl->filterout (octoutl, period);
    }



    filterl->filterout(smpsl, period);
    filterr->filterout(smpsr, period);



    if (Pprefiltering == 0)
        applyfilters (efxoutl, efxoutr, period);



    float level = dB2rap (60.0f * (float)Plevel / 127.0f - 40.0f);

    for (i = 0; i < period; i++) {
        lout = efxoutl[i];
        rout = efxoutr[i];

        l = lout * (1.0f - lrcross) + rout * lrcross;
        r = rout * (1.0f - lrcross) + lout * lrcross;

        if (octmix > 0.01f) {
            lout = l * (1.0f - octmix) + octoutl[i] * octmix;
            rout = r * (1.0f - octmix) + octoutr[i] * octmix;
        } else {
            lout = l;
            rout = r;
        }

        //efxoutl[i] = lout * level * panning;
        //efxoutr[i] = rout * level * ( 1.0f - panning);
        efxoutl[i] = lout * level * ( 1.0f - panning);
        efxoutr[i] = rout * level * panning;

    };

    DCr->filterout (efxoutr, period);
    DCl->filterout (efxoutl, period);


};


/*
 * Parameter control
 */
void
NewDist::setvolume (int Pvolume)
{
    this->Pvolume = Pvolume;

    outvolume = (float)Pvolume / 127.0f;
    if (Pvolume == 0)
        cleanup ();

};

void
NewDist::setpanning (int Ppanning)
{
    this->Ppanning = Ppanning;
    panning = ((float)Ppanning + 0.5f) / 127.0f;
};


void
NewDist::setlrcross (int Plrcross)
{
    this->Plrcross = Plrcross;
    lrcross = (float) Plrcross / 127.0f * 1.0f;
};

void
NewDist::setlpf (int value)
{
    Plpf = value;
    float fr =(float)Plpf;
    lpfl->setfreq (fr);
    lpfr->setfreq (fr);
};

void
NewDist::sethpf (int value)
{
    Phpf = value;
    float fr = (float)Phpf;
    hpfl->setfreq (fr);
    hpfr->setfreq (fr);
};


void
NewDist::setoctave (int Poctave)
{
    this->Poctave = Poctave;
    octmix = (float) (Poctave) / 127.0f;
};


void
NewDist::setpreset (int npreset)
{
    const int PRESET_SIZE = 11;
    const int NUM_PRESETS = 3;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //NewDist 1
        {0, 64, 64, 83, 65, 15, 0, 2437, 169, 68, 0},
        //NewDist 2
        {0, 64, 64, 95, 45, 6, 0, 3459, 209, 60, 1},
        //NewDist 3
        {0, 64, 64, 43, 77, 16, 0, 2983, 118, 83, 0}
    };

    if(npreset>NUM_PRESETS-1) {

        Fpre->ReadPreset(17,npreset-NUM_PRESETS+1, pdata);
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
NewDist::changepar (int npar, int value)
{
    switch (npar) {
    case 0:
        setvolume (value);
        break;
    case 1:
        setpanning (value);
        break;
    case 2:
        setlrcross (value);
        break;
    case 3:
        Pdrive = value;
        break;
    case 4:
        Plevel = value;
        break;
    case 5:
        Ptype = value;
        break;
    case 6:
        if (value > 1)
            value = 1;
        Pnegate = value;
        break;
    case 7:
        setlpf (value);
        break;
    case 8:
        sethpf (value);
        break;
    case 9:
        Prfreq = value;
        rfreq = expf (powf ((float)value / 127.0f, 0.5f) * logf (25000.0f)) + 40.0f;
        filterl->setfreq(rfreq);
        filterr->setfreq(rfreq);

        break;
    case 10:
        Pprefiltering = value;
        break;
    case 11:
        setoctave (value);
        break;


    };
};

int
NewDist::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
        return (Ppanning);
        break;
    case 2:
        return (Plrcross);
        break;
    case 3:
        return (Pdrive);
        break;
    case 4:
        return (Plevel);
        break;
    case 5:
        return (Ptype);
        break;
    case 6:
        return (Pnegate);
        break;
    case 7:
        return (Plpf);
        break;
    case 8:
        return (Phpf);
        break;
    case 9:
        return (Prfreq);
        break;
    case 10:
        return (Pprefiltering);
        break;
    case 11:
        return (Poctave);
        break;


    };
    return (0);			//in case of bogus parameter number
};

