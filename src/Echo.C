/*
  ZynAddSubFX - a software synthesizer

  Echo.C - Echo effect
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  Modified for rakarrack by Josep Andreu
  Reverse Echo effect by Transmogrifox
  Echo Direct patch From: Arnout Engelen <arnouten@bzzt.net>
  delayline utility by Transmogrifox

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
#include "Echo.h"

Echo::Echo (float * efxoutl_, float * efxoutr_, double samplerate)
{
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;
    //default values
    Ppreset = 0;
    Pvolume = 50;
    Ppanning = 64;
    Pdelay = 60;
    Plrdelay = 100;
    Plrcross = 100;
    Pfb = 40;
    Phidamp = 60;

    lrdelay = 0;
    Srate_Attack_Coeff = 1.0f / (samplerate * ATTACK);
    maxx_delay = samplerate * MAX_DELAY;
    fade = samplerate / 5;    //1/5 SR fade time available

    ldelay = new delayline(2.0f, 1, samplerate);
    rdelay = new delayline(2.0f, 1, samplerate);

    setpreset (Ppreset);
    cleanup ();
};

Echo::~Echo ()
{
	delete ldelay;
	delete rdelay;
};

/*
 * Cleanup the effect
 */
void
Echo::cleanup ()
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
Echo::initdelays ()
{

    oldl = 0.0;
    oldr = 0.0;
    ltime = delay + lrdelay;
    rtime = delay - lrdelay;

    if(ltime > 2.0f) ltime = 2.0f;
    if(ltime<0.01f) ltime = 0.01f;

    if(rtime > 2.0f) rtime = 2.0f;
    if(rtime<0.01f) rtime = 0.01f;
};

/*
 * Effect output
 */
void
Echo::out (float * smpsl, float * smpsr, uint32_t period)
{
    unsigned int i;
    float l, r, ldl, rdl, ldlout, rdlout, rvl, rvr;

    for (i = 0; i < period; i++) {

        ldl = ldelay->delay_simple(oldl, ltime, 0, 1, 0);
        rdl = rdelay->delay_simple(oldr, rtime, 0, 1, 0);

        if(Preverse) {
            rvl = ldelay->delay_simple(oldl, ltime, 1, 0, 1)*ldelay->envelope();
            rvr = rdelay->delay_simple(oldr, rtime, 1, 0, 1)*rdelay->envelope();
            ldl = ireverse*ldl + reverse*rvl;
            rdl = ireverse*rdl + reverse*rvr;
        }

        l = ldl * (1.0f - lrcross) + rdl * lrcross;
        r = rdl * (1.0f - lrcross) + ldl * lrcross;
        ldl = l;
        rdl = r;

        ldlout = -ldl*fb;
        rdlout = -rdl*fb;
        if (!Pdirect) {
            l = ldl = smpsl[i] * panning + ldlout;
            r = rdl = smpsr[i] * (1.0f - panning) + rdlout;
        } else {
            ldl = smpsl[i] * panning + ldlout;
            rdl = smpsr[i] * (1.0f - panning) + rdlout;
        }

        efxoutl[i]= l;
        efxoutr[i]= r;

        //LowPass Filter
        oldl = ldl * hidamp + oldl * (1.0f - hidamp);
        oldr = rdl * hidamp + oldr * (1.0f - hidamp);
        oldl += DENORMAL_GUARD;
        oldr += DENORMAL_GUARD;

    };

};


/*
 * Parameter control
 */
void
Echo::setvolume (int Pvolume)
{
    this->Pvolume = Pvolume;
    outvolume = (float)Pvolume / 127.0f;

};

void
Echo::setpanning (int Ppanning)
{
    this->Ppanning = Ppanning;
    panning = ((float)Ppanning + 0.5f) / 127.0f;
};

void
Echo::setreverse (int Preverse)
{
    this->Preverse = Preverse;
    reverse = (float) Preverse / 127.0f;
    ireverse = 1.0f - reverse;
};

void
Echo::Tempo2Delay(int value)
{

    Pdelay = 60.0f/(float)value * 1000.0f;
    delay = (float)Pdelay / 1000.0f;
    if ((unsigned int) delay > (MAX_DELAY)) delay = MAX_DELAY;
    ldelay->set_averaging(10.0f);
    rdelay->set_averaging(10.0f);
    initdelays();
}

void
Echo::setdelay (int Pdelay)
{
    this->Pdelay = Pdelay;
    delay= ((float) Pdelay)/1000.0f;
    ldelay->set_averaging(0.05f);
    rdelay->set_averaging(0.05f);
    initdelays ();
};

void
Echo::setlrdelay (int Plrdelay)
{
    float tmp;
    this->Plrdelay = Plrdelay;
    tmp =
        (powf (2.0, fabsf ((float)Plrdelay - 64.0f) / 64.0f * 9.0f) -
         1.0f) / 1000.0f;
    if (Plrdelay < 64.0)
        tmp = -tmp;
    lrdelay = tmp;
    initdelays ();
};

void
Echo::setlrcross (int Plrcross)
{
    this->Plrcross = Plrcross;
    lrcross = (float)Plrcross / 127.0f * 1.0f;
};

void
Echo::setfb (int Pfb)
{
    this->Pfb = Pfb;
    fb = (float)Pfb / 128.0f;
};

void
Echo::sethidamp (int Phidamp)
{
    this->Phidamp = Phidamp;
    hidamp = 1.0f - (float)Phidamp / 127.0f;
};

void
Echo::setdirect (int Pdirect)
{
    if (Pdirect > 0)
        Pdirect = 1;
    this->Pdirect = Pdirect;
};

void
Echo::setpreset (int npreset)
{
    const int PRESET_SIZE = 9;
    const int NUM_PRESETS = 9;
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Echo 1
        {67, 64, 565, 64, 30, 59, 0, 127, 0},
        //Echo 2
        {67, 64, 357, 64, 30, 59, 0, 64, 0},
        //Echo 3
        {67, 75, 955, 64, 30, 59, 10, 0, 0},
        //Simple Echo
        {67, 60, 705, 64, 30, 0, 0, 0, 0},
        //Canyon
        {67, 60, 1610, 50, 30, 82, 48, 0, 0},
        //Panning Echo 1
        {67, 64, 705, 17, 0, 82, 24, 0, 0},
        //Panning Echo 2
        {81, 60, 737, 118, 100, 68, 18, 0, 0},
        //Panning Echo 3
        {81, 60, 472, 100, 127, 67, 36, 0, 0},
        //Feedback Echo
        {62, 64, 456, 64, 100, 90, 55, 0, 0}
    };


    for (int n = 0; n < PRESET_SIZE; n++)
        changepar (n, presets[npreset][n]);
    Ppreset = npreset;
};


void
Echo::changepar (int npar, int value)
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
        setdirect (value);
        break;
    };
};

int
Echo::getpar (int npar)
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
        return (Pdirect);
        break;

    };
    return (0);			//in case of bogus parameter number
};
