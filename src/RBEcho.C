/*
  ZynAddSubFX - a software synthesizer

  RBEcho.C - Echo effect
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  Modified for rakarrack by Josep Andreu
  Reverse Echo effect by Transmogrifox

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
#include "RBEcho.h"

RBEcho::RBEcho (float * efxoutl_, float * efxoutr_, double sample_rate)
{
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;
    fSAMPLE_RATE = sample_rate;

    //default values
    Ppreset = 0;
    Pvolume = 50;
    Ppanning = 64;
    Pdelay = 60;
    Plrdelay = 100;
    Plrcross = 100;
    Pfb = 40;
    Phidamp = 60;
    Psubdiv = 1;
    subdiv = 1.0f;
    reverse = ireverse = 0.0f;
    pingpong = 0.0f;
    ipingpong = 1.0f;

    lrdelay = 0;
    Srate_Attack_Coeff = 1.0f / (sample_rate * ATTACK);
    maxx_delay = 1 + sample_rate * MAX_DELAY;

    ldelay = new delayline(2.0f, 3, sample_rate);
    rdelay = new delayline(2.0f, 3, sample_rate);

    setpreset (Ppreset);
    cleanup ();
};

RBEcho::~RBEcho ()
{
	delete ldelay;
	delete rdelay;
};

/*
 * Cleanup the effect
 */
void
RBEcho::cleanup ()
{
    ldelay->cleanup();
    rdelay->cleanup();
    ldelay->set_averaging(0.25f);
    rdelay->set_averaging(0.25f);
    oldl = 0.0;
    oldr = 0.0;
};


/*
 * Initialize the delays
 */
void
RBEcho::initdelays ()
{
    oldl = 0.0;
    oldr = 0.0;

    if(Plrdelay>0) {
        ltime = delay + lrdelay;
        rtime = delay - lrdelay;
    } else {
        ltime = delay - lrdelay;
        rtime = delay + lrdelay;
    }

    if(ltime > 2.0f) ltime = 2.0f;
    if(ltime<0.01f) ltime = 0.01f;

    if(rtime > 2.0f) rtime = 2.0f;
    if(rtime<0.01f) rtime = 0.01f;


};

/*
 * Effect output
 */
void
RBEcho::out (float * smpsl, float * smpsr, uint32_t period)
{
    unsigned int i;
    float ldl, rdl;
    float avg, ldiff, rdiff, tmp;


    for (i = 0; i < period; i++) {

        //LowPass Filter
        ldl = lfeedback * hidamp + oldl * (1.0f - hidamp);
        rdl = rfeedback * hidamp + oldr * (1.0f - hidamp);
        oldl = ldl + DENORMAL_GUARD;
        oldr = rdl + DENORMAL_GUARD;

        ldl = ldelay->delay_simple((ldl + smpsl[i]), delay, 0, 1, 0);
        rdl = rdelay->delay_simple((rdl + smpsr[i]), delay, 0, 1, 0);


        if(Preverse) {
            rvl = ldelay->delay_simple(oldl, delay, 1, 0, 1)*ldelay->envelope();
            rvr = rdelay->delay_simple(oldr, delay, 1, 0, 1)*rdelay->envelope();
            ldl = ireverse*ldl + reverse*rvl;
            rdl = ireverse*rdl + reverse*rvr;

        }


        //lfeedback = lpanning * fb * ldl;
        //rfeedback = rpanning * fb * rdl;
        lfeedback = rpanning * fb * ldl;
        rfeedback = lpanning * fb * rdl;

        if(Pes) {
            ldl *= cosf(lrcross);
            rdl *= sinf(lrcross);

            avg = (ldl + rdl) * 0.5f;
            ldiff = ldl - avg;
            rdiff = rdl - avg;

            tmp = avg + ldiff * pes;
            ldl = 0.5 * tmp;

            tmp = avg + rdiff * pes;
            rdl = 0.5f * tmp;


        }
        //efxoutl[i] = (ipingpong*ldl + pingpong *ldelay->delay_simple(0.0f, ltime, 2, 0, 0)) * lpanning;
        //efxoutr[i] = (ipingpong*rdl + pingpong *rdelay->delay_simple(0.0f, rtime, 2, 0, 0)) * rpanning;
        efxoutl[i] = (ipingpong*ldl + pingpong *ldelay->delay_simple(0.0f, ltime, 2, 0, 0)) * rpanning;
        efxoutr[i] = (ipingpong*rdl + pingpong *rdelay->delay_simple(0.0f, rtime, 2, 0, 0)) * lpanning;

    };

};


/*
 * Parameter control
 */
void
RBEcho::setvolume (int Pvolume)
{
    this->Pvolume = Pvolume;
    outvolume = (float)Pvolume / 127.0f;

};

void
RBEcho::setpanning (int Ppanning)
{
    this->Ppanning = Ppanning;
    lpanning = ((float)Ppanning) / 64.0f;
    rpanning = 2.0f - lpanning;
    lpanning = 10.0f * powf(lpanning, 4);
    rpanning = 10.0f * powf(rpanning, 4);
    lpanning = 1.0f - 1.0f/(lpanning + 1.0f);
    rpanning = 1.0f - 1.0f/(rpanning + 1.0f);
    lpanning *= 1.1f;
    rpanning *= 1.1f;
};

void
RBEcho::setreverse (int Preverse)
{
    this->Preverse = Preverse;
    reverse = (float) Preverse / 127.0f;
    ireverse = 1.0f - reverse;
};

void
RBEcho::setdelay (int Pdelay)
{
    this->Pdelay = Pdelay;
    fdelay= 60.0f/((float) Pdelay);
    if (fdelay < 0.01f) fdelay = 0.01f;
    if (fdelay > (float) MAX_DELAY) fdelay = (float) MAX_DELAY;  //Constrains 10ms ... MAX_DELAY
    delay = subdiv * fdelay;
    initdelays ();
};

void
RBEcho::setlrdelay (int Plrdelay)
{
    float tmp;
    this->Plrdelay = Plrdelay;
    lrdelay = delay * fabs(((float)Plrdelay - 64.0f) / 65.0f);

    tmp = fabs( ((float) Plrdelay - 64.0f)/32.0f);
    pingpong = 1.0f - 1.0f/(5.0f*tmp*tmp + 1.0f);
    pingpong *= 1.05159f;
    ipingpong = 1.0f - pingpong;
    initdelays ();
};

void
RBEcho::setlrcross (int Plrcross)
{
    this->Plrcross = Plrcross;
    lrcross = D_PI * (float)Plrcross / 128.0f;

};

void
RBEcho::setfb (int Pfb)
{
    this->Pfb = Pfb;
    fb = (float)Pfb / 128.0f;
};

void
RBEcho::sethidamp (int Phidamp)
{
    this->Phidamp = Phidamp;
    hidamp = f_exp(-D_PI * 500.0f * ((float) Phidamp)/fSAMPLE_RATE);
};

void
RBEcho::setpreset (int npreset)
{
    const int PRESET_SIZE = 10;
    const int NUM_PRESETS = 3;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Echo 1
        {64, 64, 90, 64, 64, 64, 64, 0, 1, 96},
        //Echo 2
        {64, 64, 90, 64, 64, 64, 64, 0, 2 ,96},
        //Echo 3
        {64, 64, 90, 64, 64, 64, 64, 0, 3 ,96}
    };

    if(npreset>NUM_PRESETS-1) {
        Fpre->ReadPreset(32,npreset-NUM_PRESETS+1,pdata);
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, pdata[n]);
    } else {
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, presets[npreset][n]);
    }
    Ppreset = npreset;
};


void
RBEcho::changepar (int npar, int value)
{
    switch (npar) {
    case 0:
        setvolume (value);
        break;
    case 1:
        setpanning (value);
        break;
    case 2:
        setdelay (value);
        break;
    case 3:
        setlrdelay (value);
        break;
    case 4:
        setlrcross (value);
        break;
    case 5:
        setfb (value);
        break;
    case 6:
        sethidamp (value);
        break;
    case 7:
        setreverse (value);
        break;
    case 8:
        Psubdiv = value;
        subdiv = 1.0f/((float)(value + 1));
        delay = subdiv * fdelay;
        initdelays ();
        break;
    case 9:
        Pes = value;
        pes = 8.0f * (float)Pes / 127.0f;
        break;
    };
};

int
RBEcho::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
        return (Ppanning);
        break;
    case 2:
        return (Pdelay);
        break;
    case 3:
        return (Plrdelay);
        break;
    case 4:
        return (Plrcross);
        break;
    case 5:
        return (Pfb);
        break;
    case 6:
        return (Phidamp);
        break;
    case 7:
        return (Preverse);
        break;
    case 8:
        return (Psubdiv);
        break;
    case 9:
        return (Pes);
        break;

    };
    return (0);			//in case of bogus parameter number
};
