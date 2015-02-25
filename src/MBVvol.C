/*
  MBVvol.C - Vary Band Volumen effect

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
#include "MBVvol.h"

/*
 * Waveshape (this is called by OscilGen::waveshape and Distorsion::process)
 */



MBVvol::MBVvol (float * efxoutl_, float * efxoutr_)
{
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;

    lowl = (float *) malloc (sizeof (float) * period);
    lowr = (float *) malloc (sizeof (float) * period);
    midll = (float *) malloc (sizeof (float) * period);
    midlr = (float *) malloc (sizeof (float) * period);
    midhl = (float *) malloc (sizeof (float) * period);
    midhr = (float *) malloc (sizeof (float) * period);
    highl = (float *) malloc (sizeof (float) * period);
    highr = (float *) malloc (sizeof (float) * period);


    lpf1l = new AnalogFilter (2, 500.0f, .7071f, 0);
    lpf1r = new AnalogFilter (2, 500.0f, .7071f, 0);
    hpf1l = new AnalogFilter (3, 500.0f, .7071f, 0);
    hpf1r = new AnalogFilter (3, 500.0f, .7071f, 0);
    lpf2l = new AnalogFilter (2, 2500.0f, .7071f, 0);
    lpf2r = new AnalogFilter (2, 2500.0f, .7071f, 0);
    hpf2l = new AnalogFilter (3, 2500.0f, .7071f, 0);
    hpf2r = new AnalogFilter (3, 2500.0f, .7071f, 0);
    lpf3l = new AnalogFilter (2, 5000.0f, .7071f, 0);
    lpf3r = new AnalogFilter (2, 5000.0f, .7071f, 0);
    hpf3l = new AnalogFilter (3, 5000.0f, .7071f, 0);
    hpf3r = new AnalogFilter (3, 5000.0f, .7071f, 0);


    //default values
    Ppreset = 0;
    Pvolume = 50;
    coeff = 1.0 / (float) period;
    volL=volLr=volML=volMLr=volMH=volMHr=volH=volHr=2.0f;

    setpreset (Ppreset);
    cleanup ();
};

MBVvol::~MBVvol ()
{
};

/*
 * Cleanup the effect
 */
void
MBVvol::cleanup ()
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

};
/*
 * Effect output
 */
void
MBVvol::out (float * smpsl, float * smpsr)
{
    int i;


    memcpy(lowl,smpsl,sizeof(float) * period);
    memcpy(midll,smpsl,sizeof(float) * period);
    memcpy(midhl,smpsl,sizeof(float) * period);
    memcpy(highl,smpsl,sizeof(float) * period);

    lpf1l->filterout(lowl);
    hpf1l->filterout(midll);
    lpf2l->filterout(midll);
    hpf2l->filterout(midhl);
    lpf3l->filterout(midhl);
    hpf3l->filterout(highl);

    memcpy(lowr,smpsr,sizeof(float) * period);
    memcpy(midlr,smpsr,sizeof(float) * period);
    memcpy(midhr,smpsr,sizeof(float) * period);
    memcpy(highr,smpsr,sizeof(float) * period);

    lpf1r->filterout(lowr);
    hpf1r->filterout(midlr);
    lpf2r->filterout(midlr);
    hpf2r->filterout(midhr);
    lpf3r->filterout(midhr);
    hpf3r->filterout(highr);

    lfo1.effectlfoout (&lfo1l, &lfo1r);
    lfo2.effectlfoout (&lfo2l, &lfo2r);

    d1=(lfo1l-v1l)*coeff;
    d2=(lfo1r-v1r)*coeff;
    d3=(lfo2l-v2l)*coeff;
    d4=(lfo2r-v2r)*coeff;

    for (i = 0; i < period; i++) {

        setCombi(Pcombi);

        efxoutl[i]=lowl[i]*volL+midll[i]*volML+midhl[i]*volMH+highl[i]*volH;
        efxoutr[i]=lowr[i]*volLr+midlr[i]*volMLr+midhr[i]*volMHr+highr[i]*volHr;
    }

};


/*
 * Parameter control
 */
void
MBVvol::setvolume (int value)
{
    Pvolume = value;
    outvolume = (float)Pvolume / 127.0f;
};

void
MBVvol::setCombi(int value)
{


    v1l+=d1;
    v1r+=d2;
    v2l+=d3;
    v2r+=d4;


    switch(value) {
    case 0:
        volL=v1l;
        volLr=v1r;
        volML=v1l;
        volMLr=v1r;
        volMH=v2l;
        volMHr=v2r;
        volH =v2l;
        volHr=v2r;
        break;
    case 1:
        volL=v1l;
        volLr=v1r;
        volML=v2l;
        volMLr=v2r;
        volMH=v2l;
        volMHr=v2r;
        volH =v1l;
        volHr=v1r;
        break;
    case 2:
        volL=v1l;
        volLr=v1r;
        volML=v2l;
        volMLr=v2r;
        volMH=v1l;
        volMHr=v1r;
        volH =v2l;
        volHr=v2r;
        break;
    case 3:
        volL=1.0f;
        volLr=1.0f;
        volML=v1l;
        volMLr=v1r;
        volMH=v1l;
        volMHr=v1r;
        volH =1.0f;
        volHr=1.0f;
        break;
    case 4:
        volL=1.0f;
        volLr=1.0f;
        volML=v1l;
        volMLr=v1r;
        volMH=v2l;
        volMHr=v2r;
        volH =1.0f;
        volHr=1.0f;
        break;
    case 5:
        volL=0.0f;
        volLr=0.0f;
        volML=v1l;
        volMLr=v1r;
        volMH=v1l;
        volMHr=v1r;
        volH =0.0f;
        volHr=0.0f;
        break;
    case 6:
        volL=0.0f;
        volLr=0.0f;
        volML=v1l;
        volMLr=v1r;
        volMH=v2l;
        volMHr=v2r;
        volH =0.0f;
        volHr=0.0f;
        break;
    case 7:
        volL=v1l;
        volLr=v1r;
        volML=1.0f;
        volMLr=1.0f;
        volMH=1.0f;
        volMHr=1.0f;
        volH =v1l;
        volHr=v1r;
        break;
    case 8:
        volL=v1l;
        volLr=v1r;
        volML=1.0f;
        volMLr=1.0f;
        volMH=1.0f;
        volMHr=1.0f;
        volH =v2l;
        volHr=v2r;
        break;
    case 9:
        volL=v1l;
        volLr=v1r;
        volML=0.0f;
        volMLr=0.0f;
        volMH=0.0f;
        volMHr=0.0f;
        volH =v1l;
        volHr=v1r;
        break;
    case 10:
        volL=v1l;
        volLr=v1r;
        volML=0.0f;
        volMLr=0.0f;
        volMH=0.0f;
        volMHr=0.0f;
        volH =v2l;
        volHr=v2r;
        break;

    }


}

void
MBVvol::setCross1 (int value)
{
    Cross1 = value;
    lpf1l->setfreq ((float)value);
    lpf1r->setfreq ((float)value);
    hpf1l->setfreq ((float)value);
    hpf1r->setfreq ((float)value);

};

void
MBVvol::setCross2 (int value)
{
    Cross2 = value;
    hpf2l->setfreq ((float)value);
    hpf2r->setfreq ((float)value);
    lpf2l->setfreq ((float)value);
    lpf2r->setfreq ((float)value);

};

void
MBVvol::setCross3 (int value)
{
    Cross3 = value;
    hpf3l->setfreq ((float)value);
    hpf3r->setfreq ((float)value);
    lpf3l->setfreq ((float)value);
    lpf3r->setfreq ((float)value);

};


void
MBVvol::setpreset (int npreset)
{
    const int PRESET_SIZE = 11;
    const int NUM_PRESETS = 3;
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Vary1
        {0, 40, 0, 64, 80, 0, 0, 500, 2500, 5000, 0},
        //Vary2
        {0, 80, 0, 64, 40, 0, 0, 120, 600, 2300, 1},
        //Vary3
        {0, 120, 0, 64, 40, 0, 0, 800, 2300, 5200, 2}
    };

    if(npreset>NUM_PRESETS-1) {
        Fpre->ReadPreset(28,npreset-NUM_PRESETS+1);
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
MBVvol::changepar (int npar, int value)
{
    switch (npar) {
    case 0:
        setvolume (value);
        break;
    case 1:
        lfo1.Pfreq = value;
        lfo1.updateparams ();
        break;
    case 2:
        lfo1.PLFOtype = value;
        lfo1.updateparams ();
        break;
    case 3:
        lfo1.Pstereo = value;
        lfo1.updateparams ();
        break;
    case 4:
        lfo2.Pfreq = value;
        lfo2.updateparams ();
        break;
    case 5:
        lfo2.PLFOtype = value;
        lfo2.updateparams ();
        break;
    case 6:
        lfo2.Pstereo = value;
        lfo2.updateparams ();
        break;
    case 7:
        setCross1 (value);
        break;
    case 8:
        setCross2 (value);
        break;
    case 9:
        setCross3(value);
        break;
    case 10:
        Pcombi=value;
        break;
    };
};

int
MBVvol::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
        return (lfo1.Pfreq);
        break;
    case 2:
        return (lfo1.PLFOtype);
        break;
    case 3:
        return (lfo1.Pstereo);
        break;
    case 4:
        return (lfo2.Pfreq);
        break;
    case 5:
        return (lfo2.PLFOtype);
        break;
    case 6:
        return (lfo2.Pstereo);
        break;
    case 7:
        return (Cross1);
        break;
    case 8:
        return (Cross2);
        break;
    case 9:
        return (Cross3);
        break;
    case 10:
        return (Pcombi);
        break;

    };
    return (0);			//in case of bogus parameter number
};

