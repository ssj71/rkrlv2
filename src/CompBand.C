/*

  CompBand.C - 4 Bands Compressor

  Using Compressor and AnalogFilters by other authors.

  Based on artscompressor.cc by Matthias Kretz <kretz@kde.org>
  Stefan Westerfeld <stefan@space.twc.de>

  Modified by Ryan Billing & Josep Andreu

  ZynAddSubFX - a software synthesizer
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

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
#include "CompBand.h"

/*
 * Waveshape (this is called by OscilGen::waveshape and Distorsion::process)
 */



CompBand::CompBand (float * efxoutl_, float * efxoutr_, double sample_rate, uint32_t intermediate_bufsize)
{
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;

    lowl = (float *) malloc (sizeof (float) * intermediate_bufsize);
    lowr = (float *) malloc (sizeof (float) * intermediate_bufsize);
    midll = (float *) malloc (sizeof (float) * intermediate_bufsize);
    midlr = (float *) malloc (sizeof (float) * intermediate_bufsize);
    midhl = (float *) malloc (sizeof (float) * intermediate_bufsize);
    midhr = (float *) malloc (sizeof (float) * intermediate_bufsize);
    highl = (float *) malloc (sizeof (float) * intermediate_bufsize);
    highr = (float *) malloc (sizeof (float) * intermediate_bufsize);


    interpbuf = new float[intermediate_bufsize];
    lpf1l = new AnalogFilter (2, 500.0f,.7071f, 0, sample_rate, interpbuf);
    lpf1r = new AnalogFilter (2, 500.0f,.7071f, 0, sample_rate, interpbuf);
    hpf1l = new AnalogFilter (3, 500.0f,.7071f, 0, sample_rate, interpbuf);
    hpf1r = new AnalogFilter (3, 500.0f,.7071f, 0, sample_rate, interpbuf);
    lpf2l = new AnalogFilter (2, 2500.0f,.7071f, 0, sample_rate, interpbuf);
    lpf2r = new AnalogFilter (2, 2500.0f,.7071f, 0, sample_rate, interpbuf);
    hpf2l = new AnalogFilter (3, 2500.0f,.7071f, 0, sample_rate, interpbuf);
    hpf2r = new AnalogFilter (3, 2500.0f,.7071f, 0, sample_rate, interpbuf);
    lpf3l = new AnalogFilter (2, 5000.0f,.7071f, 0, sample_rate, interpbuf);
    lpf3r = new AnalogFilter (2, 5000.0f,.7071f, 0, sample_rate, interpbuf);
    hpf3l = new AnalogFilter (3, 5000.0f,.7071f, 0, sample_rate, interpbuf);
    hpf3r = new AnalogFilter (3, 5000.0f,.7071f, 0, sample_rate, interpbuf);


    CL = new Compressor(efxoutl,efxoutr, sample_rate);
    CML = new Compressor(efxoutl,efxoutr, sample_rate);
    CMH = new Compressor(efxoutl,efxoutr, sample_rate);
    CH = new Compressor(efxoutl,efxoutr, sample_rate);

    CL->Compressor_Change_Preset(0,5);
    CML->Compressor_Change_Preset(0,5);
    CMH->Compressor_Change_Preset(0,5);
    CH->Compressor_Change_Preset(0,5);


    //default values
    Ppreset = 0;
    Pvolume = 50;

    setpreset (Ppreset);
    cleanup ();
};

CompBand::~CompBand ()
{
	free(lowl);
	free(lowr);
	free(midll);
	free(midlr);
	free(midhl);
	free(midhr);
	free(highl);
	free(highr);

    delete lpf1l;
    delete lpf1r;
    delete hpf1l;
    delete hpf1r;
    delete lpf2l;
    delete lpf2r;
    delete hpf2l;
    delete hpf2r;
    delete lpf3l;
    delete lpf3r;
    delete hpf3l;
    delete hpf3r;
    delete[] interpbuf;

    delete CL;
    delete CML;
    delete CMH;
    delete CH;
};

/*
 * Cleanup the effect
 */
void
CompBand::cleanup ()
{
    lpf1l->cleanup ();
    hpf1l->cleanup ();
    lpf1r->cleanup ();
    hpf1r->cleanup ();
    lpf2l->cleanup ();
    hpf2l->cleanup ();
    lpf2r->cleanup ();
    hpf2r->cleanup ();
    lpf3l->cleanup ();
    hpf3l->cleanup ();
    lpf3r->cleanup ();
    hpf3r->cleanup ();
    CL->cleanup();
    CML->cleanup();
    CMH->cleanup();
    CH->cleanup();

};
/*
 * Effect output
 */
void
CompBand::out (float * smpsl, float * smpsr, uint32_t period)
{
    unsigned int i;


    memcpy(lowl,smpsl,sizeof(float) * period);
    memcpy(midll,smpsl,sizeof(float) * period);
    memcpy(midhl,smpsl,sizeof(float) * period);
    memcpy(highl,smpsl,sizeof(float) * period);

    lpf1l->filterout(lowl, period);
    hpf1l->filterout(midll, period);
    lpf2l->filterout(midll, period);
    hpf2l->filterout(midhl, period);
    lpf3l->filterout(midhl, period);
    hpf3l->filterout(highl, period);

    memcpy(lowr,smpsr,sizeof(float) * period);
    memcpy(midlr,smpsr,sizeof(float) * period);
    memcpy(midhr,smpsr,sizeof(float) * period);
    memcpy(highr,smpsr,sizeof(float) * period);

    lpf1r->filterout(lowr, period);
    hpf1r->filterout(midlr, period);
    lpf2r->filterout(midlr, period);
    hpf2r->filterout(midhr, period);
    lpf3r->filterout(midhr, period);
    hpf3r->filterout(highr, period);


    CL->out(lowl,lowr, period);
    CML->out(midll,midlr, period);
    CMH->out(midhl,midhr, period);
    CH->out(highl,highr, period);


    for (i = 0; i < period; i++) {
        efxoutl[i]=(lowl[i]+midll[i]+midhl[i]+highl[i])*level;
        efxoutr[i]=(lowr[i]+midlr[i]+midhr[i]+highr[i])*level;
    }



};


/*
 * Parameter control
 */
void
CompBand::setvolume (int value)
{
    Pvolume = value;
    outvolume = (float)Pvolume / 128.0f;

};


void
CompBand::setlevel (int value)
{
    Plevel = value;
    level = dB2rap (60.0f * (float)value / 127.0f - 36.0f);


};



void
CompBand::setratio(int ch, int value)
{

    switch(ch) {
    case 0:
        CL->Compressor_Change(2,value);
        break;
    case 1:
        CML->Compressor_Change(2,value);
        break;
    case 2:
        CMH->Compressor_Change(2,value);
        break;
    case 3:
        CH->Compressor_Change(2,value);
        break;
    }
}


void
CompBand::setthres(int ch, int value)
{

    switch(ch) {
    case 0:
        CL->Compressor_Change(1,value);
        break;
    case 1:
        CML->Compressor_Change(1,value);
        break;
    case 2:
        CMH->Compressor_Change(1,value);
        break;
    case 3:
        CH->Compressor_Change(1,value);
        break;
    }
}




void
CompBand::setCross1 (int value)
{
    Cross1 = value;
    lpf1l->setfreq ((float)value);
    lpf1r->setfreq ((float)value);
    hpf1l->setfreq ((float)value);
    hpf1r->setfreq ((float)value);

};

void
CompBand::setCross2 (int value)
{
    Cross2 = value;
    hpf2l->setfreq ((float)value);
    hpf2r->setfreq ((float)value);
    lpf2l->setfreq ((float)value);
    lpf2r->setfreq ((float)value);

};

void
CompBand::setCross3 (int value)
{
    Cross3 = value;
    hpf3l->setfreq ((float)value);
    hpf3r->setfreq ((float)value);
    lpf3l->setfreq ((float)value);
    lpf3r->setfreq ((float)value);

};


void
CompBand::setpreset (int npreset)
{
    const int PRESET_SIZE = 13;
    const int NUM_PRESETS = 3;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Good Start
        {0, 16, 16, 16, 16, 0, 0, 0, 0, 1000, 5000, 10000, 48},

        //Loudness
        {0, 16, 2, 2, 4, -16, 24, 24, -8, 140, 1000, 5000, 48},

        //Loudness 2
        {64, 16, 2, 2, 2, -32, 24, 24, 24, 100, 1000, 5000, 48}

    };

    if(npreset>NUM_PRESETS-1) {
        Fpre->ReadPreset(43,npreset-NUM_PRESETS+1,pdata);
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
CompBand::changepar (int npar, int value)
{
    switch (npar) {
    case 0:
        setvolume (value);
        break;
    case 1:
        PLratio = value;
        setratio(0,value);
        break;
    case 2:
        PMLratio = value;
        setratio(1,value);
        break;
    case 3:
        PMHratio = value;
        setratio(2,value);
        break;
    case 4:
        PHratio = value;
        setratio(3,value);
        break;
    case 5:
        PLthres = value;
        setthres(0,value);
        break;
    case 6:
        PMLthres = value;
        setthres(1,value);
        break;
    case 7:
        PMHthres = value;
        setthres(2,value);
        break;
    case 8:
        PHthres = value;
        setthres(3,value);
        break;
    case 9:
        setCross1 (value);
        break;
    case 10:
        setCross2 (value);
        break;
    case 11:
        setCross3(value);
        break;
    case 12:
        setlevel(value);
        break;


    };
};

int
CompBand::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
        return (PLratio);
        break;
    case 2:
        return (PMLratio);
        break;
    case 3:
        return (PMHratio);
        break;
    case 4:
        return (PHratio);
        break;
    case 5:
        return (PLthres);
        break;
    case 6:
        return (PMLthres);
        break;
    case 7:
        return (PMHthres);
        break;
    case 8:
        return (PHthres);
        break;
    case 9:
        return (Cross1);
        break;
    case 10:
        return (Cross2);
        break;
    case 11:
        return (Cross3);
        break;
    case 12:
        return (Plevel);
        break;
    };
    return (0);			//in case of bogus parameter number
};

