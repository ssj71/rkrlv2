/*
  ZynAddSubFX - a software synthesizer

  Reverb.C - Reverberation effect
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

#include <math.h>
#include <stdlib.h>
#include "Reverb.h"

/*TODO: EarlyReflections,Prdelay,Perbalance */

Reverb::Reverb (float * efxoutl_, float * efxoutr_, double samplerate, uint16_t intermediate_bufsize)
{
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;
    inputbuf = new float[intermediate_bufsize];
    //filterpars=NULL;
    unsigned int i;
    for(i=0;i<intermediate_bufsize;i++)
    {
    	inputbuf[i] = 0;
    }


    //defaults
    Ppreset = 0;
    Pvolume = 48;
    Ppan = 64;
    Ptime = 64;
    Pidelay = 40;
    Pidelayfb = 0;
    Prdelay = 0;
    Plpf = 127;
    Phpf = 0;
    Perbalance = 64;
    Plohidamp = 80;
    Ptype = 1;
    Proomsize = 64;
    roomsize = 1.0f;
    rs = 1.0f;
    rs_coeff = rs / (float) REV_COMBS;

    fSAMPLE_RATE = samplerate;

    //max comb length
    unsigned int tmp = lrintf(220023.0*samplerate/44100.0);

    for (int i = 0; i < REV_COMBS * 2; i++) {
        comblen[i] = 800 + (int) (RND * 1400);
        combk[i] = 0;
        lpcomb[i] = 0;
        combfb[i] = -0.97f;
        comb[i] = new float[tmp];//set to make length so we don't need to reallocate ever
    };

    //max ap length
    tmp = lrintf(100023.0*samplerate/44100.0);

    for (int i = 0; i < REV_APS * 2; i++) {
        aplen[i] = 500 + (int) (RND * 500);
        apk[i] = 0;
        ap[i] = new float[tmp]; //set to max length
    };

    interpbuf = new float[intermediate_bufsize];
    lpf =  new AnalogFilter (2, 22000, 1, 0, samplerate, interpbuf);
    hpf =  new AnalogFilter (3, 20, 1, 0, samplerate, interpbuf);

    //max delay length
    tmp = lrintf(2.5*samplerate);
    idelay = new float[tmp]; //set to max length

    setpreset (Ppreset);
    cleanup ();			//do not call this before the comb initialisation
};


Reverb::~Reverb ()
{
	delete lpf;
	delete hpf;
	delete[] interpbuf;
	delete[] inputbuf;
    for (int i = 0; i < REV_COMBS * 2; i++) {
        delete[] comb[i];
    }
    for (int i = 0; i < REV_APS * 2; i++) {
        delete[] ap[i];
    }
    delete[] idelay;
};

/*
 * Cleanup the effect
 */
void
Reverb::cleanup ()
{
    int i, j;
    for (i = 0; i < REV_COMBS * 2; i++) {
        lpcomb[i] = 0.0;
        for (j = 0; j < comblen[i]; j++)
            comb[i][j] = 0.0;
    };

    for (i = 0; i < REV_APS * 2; i++)
        for (j = 0; j < aplen[i]; j++)
            ap[i][j] = 0.0;

    if (idelay != NULL)
        for (i = 0; i < idelaylen; i++)
            idelay[i] = 0.0;

    hpf->cleanup ();
    lpf->cleanup ();


};

/*
 * Process one channel; 0=left,1=right
 */
void
Reverb::processmono (unsigned int ch, float * output, uint32_t period)
{
    unsigned int i, j;
    float fbout, tmp;
    //TODO: implement the high part from lohidamp

    for (j = REV_COMBS * ch; j < REV_COMBS * (ch + 1); j++) {

        int ck = combk[j];
        int comblength = comblen[j];
        float lpcombj = lpcomb[j];

        for (i = 0; i < period; i++) {
            fbout = comb[j][ck] * combfb[j];
            fbout = fbout * (1.0f - lohifb) + (lpcombj * lohifb);
            lpcombj = fbout;

            comb[j][ck] = inputbuf[i] + fbout;
            output[i] += fbout;

            if ((++ck) >= comblength)
                ck = 0;
        };

        combk[j] = ck;
        lpcomb[j] = lpcombj;
    };

    for (j = REV_APS * ch; j < REV_APS * (1 + ch); j++) {
        int ak = apk[j];
        int aplength = aplen[j];
        for (i = 0; i < period; i++) {
            tmp = ap[j][ak];
            ap[j][ak] = 0.7f * tmp + output[i];
            output[i] = tmp - 0.7f * ap[j][ak];
            if ((++ak) >= aplength)
                ak = 0;
        };
        apk[j] = ak;
    };
};

/*
 * Effect output
 */
void
Reverb::out (float * smps_l, float * smps_r, uint32_t period)
{
    unsigned int i;

    for (i = 0; i < period; i++) {
        inputbuf[i] = (smps_l[i] + smps_r[i]) * .5f;
        //Initial delay r
        if (idelay != NULL) {
            float tmp = inputbuf[i] + idelay[idelayk] * idelayfb;
            inputbuf[i] = idelay[idelayk];
            idelay[idelayk] = tmp;
            idelayk++;
            if (idelayk >= idelaylen)
                idelayk = 0;
        };
    };


    lpf->filterout (inputbuf, period);
    hpf->filterout (inputbuf, period);

    processmono (0, efxoutl, period);	//left
    processmono (1, efxoutr, period);	//right



    float lvol = rs_coeff * pan * 2.0f;
    float rvol = rs_coeff * (1.0f - pan) * 2.0f;

    for (unsigned int i = 0; i < period; i++) {
        efxoutl[i] *= lvol;
        efxoutr[i] *= rvol;

    };
};


/*
 * Parameter control
 */
void
Reverb::setvolume (int Pvolume)
{
    this->Pvolume = Pvolume;
    outvolume = (float)Pvolume / 127.0f;
    if (Pvolume == 0)
        cleanup ();


};

void
Reverb::setpan (int Ppan)
{
    this->Ppan = Ppan;
    pan = (float) Ppan / 127.0f;
};

void
Reverb::settime (int Ptime)
{
    int i;
    float t;
    this->Ptime = Ptime;
    t = powf (60.0f, (float) Ptime / 127.0f) - 0.97f;

    for (i = 0; i < REV_COMBS * 2; i++) {
        combfb[i] =
            -expf ((float) comblen[i] / fSAMPLE_RATE * logf (0.001f) /
                   t);
        //the feedback is negative because it removes the DC
    };
};

void
Reverb::setlohidamp (int Plohidamp)
{
    float x;

    if (Plohidamp < 64)
        Plohidamp = 64;		//remove this when the high part from lohidamp will be added

    this->Plohidamp = Plohidamp;
    if (Plohidamp == 64) {
        lohidamptype = 0;
        lohifb = 0.0;
    } else {
        if (Plohidamp < 64)
            lohidamptype = 1;
        if (Plohidamp > 64)
            lohidamptype = 2;
        x = fabsf ((float) (Plohidamp - 64) / 64.1f);
        lohifb = x * x;
    };
};

void
Reverb::setidelay (int Pidelay)
{
    float delay;
    this->Pidelay = Pidelay;
    delay = powf (50.0f * (float)Pidelay / 127.0f, 2.0f) - 1.0f;

    idelaylen = lrintf (fSAMPLE_RATE * delay / 1000.0f);
    if (idelaylen > 1) {
        idelayk = 0;
        for (int i = 0; i < idelaylen; i++)
            idelay[i] = 0.0;
    };
};

void
Reverb::setidelayfb (int Pidelayfb)
{
    this->Pidelayfb = Pidelayfb;
    idelayfb = (float)Pidelayfb / 128.0f;
};

void
Reverb::sethpf (int value)
{
    Phpf = value;
    float fr = (float)Phpf;
    hpf->setfreq (fr);

};

void
Reverb::setlpf (int value)
{
    Plpf = value;
    float fr = (float)Plpf;
    lpf->setfreq (fr);

};

void
Reverb::settype (int Ptype)
{
    const int NUM_TYPES = 2;
    int combtunings[NUM_TYPES][REV_COMBS] = {
        //this is unused (for random)
        {0, 0, 0, 0, 0, 0, 0, 0},
        //Freeverb by Jezar at Dreampoint
        {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617}
    };
    int aptunings[NUM_TYPES][REV_APS] = {
        //this is unused (for random)
        {0, 0, 0, 0},
        //Freeverb by Jezar at Dreampoint
        {225, 341, 441, 556}
    };

    if (Ptype >= NUM_TYPES)
        Ptype = NUM_TYPES - 1;
    this->Ptype = Ptype;

    float tmp;
    for (int i = 0; i < REV_COMBS * 2; i++) {
        if (Ptype == 0)
            tmp = 800.0f + (float)(RND*1400.0f);
        else
            tmp = (float)combtunings[Ptype][i % REV_COMBS];
        tmp *= roomsize;
        if (i > REV_COMBS)
            tmp += 23.0f;
        tmp *= fSAMPLE_RATE / 44100.0f;	//adjust the combs according to the samplerate
        if (tmp < 10)
            tmp = 10;

        comblen[i] = lrintf(tmp);
        combk[i] = 0;
        lpcomb[i] = 0;
    };

    for (int i = 0; i < REV_APS * 2; i++) {
        if (Ptype == 0)
            tmp = 500.0f + (float)(RND*500.0f);
        else
            tmp = (float)aptunings[Ptype][i % REV_APS];
        tmp *= roomsize;
        if (i > REV_APS)
            tmp += 23.0f;
        tmp *= fSAMPLE_RATE / 44100.0f;	//adjust the combs according to the samplerate
        if (tmp < 10)
            tmp = 10;
        aplen[i] = lrintf(tmp);
        apk[i] = 0;
    };
    settime (Ptime);
    cleanup ();
};

void
Reverb::setroomsize (int Proomsize)
{
    if (Proomsize == 0)
        Proomsize = 64;		//this is because the older versions consider roomsize=0
    this->Proomsize = Proomsize;
    roomsize = ((float)Proomsize - 64.0f) / 64.0f;
    if (roomsize > 0.0)
        roomsize *= 2.0f;
    roomsize = powf (10.0f, roomsize);
    rs = sqrtf (roomsize);
    rs_coeff = rs / (float) REV_COMBS;
    settype (Ptype);
};

void
Reverb::setpreset (int npreset)
{
    const int PRESET_SIZE = 12;
    const int NUM_PRESETS = 13;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Cathedral1
        {80, 64, 63, 24, 0, 0, 0, 4002, 27, 83, 1, 64},
        //Cathedral2
        {80, 64, 69, 35, 0, 0, 0, 25040, 21, 71, 0, 64},
        //Cathedral3
        {80, 64, 69, 24, 0, 0, 0, 25040, 2417, 78, 1, 85},
        //Hall1
        {90, 64, 51, 10, 0, 0, 0, 25040, 81, 78, 1, 64},
        //Hall2
        {90, 64, 53, 20, 0, 0, 0, 25040, 2417, 71, 1, 64},
        //Room1
        {100, 64, 33, 0, 0, 0, 0, 25040, 21, 106, 0, 30},
        //Room2
        {100, 64, 21, 26, 0, 0, 0, 1223, 21, 77, 1, 45},
        //Basement
        {110, 64, 14, 0, 0, 0, 0, 25040, 27, 71, 0, 25},
        //Tunnel
        {85, 80, 84, 20, 42, 0, 0, 652, 21, 78, 1, 105},
        //Echoed1
        {95, 64, 26, 60, 71, 0, 0, 14722, 21, 64, 1, 64},
        //Echoed2
        {90, 64, 40, 88, 71, 0, 0, 14722, 21, 88, 1, 64},
        //VeryLong1
        {90, 64, 93, 15, 0, 0, 0, 14722, 21, 77, 0, 95},
        //VeryLong2
        {90, 64, 111, 30, 0, 0, 0, 14722, 5058, 74, 1, 80}
    };


    if(npreset>NUM_PRESETS-1) {

        Fpre->ReadPreset(8,npreset-NUM_PRESETS+1,pdata);
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, pdata[n]);
    } else {
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, presets[npreset][n]);
    }
    Ppreset = npreset;
};


void
Reverb::changepar (int npar, int value)
{
    switch (npar) {
    case 0:
        setvolume (value);
        break;
    case 1:
        setpan (value);
        break;
    case 2:
        settime (value);
        break;
    case 3:
        setidelay (value);
        break;
    case 4:
        setidelayfb (value);
        break;
//      case 5: setrdelay(value);
//              break;
//      case 6: seterbalance(value);
//              break;
    case 7:
        setlpf (value);
        break;
    case 8:
        sethpf (value);
        break;
    case 9:
        setlohidamp (value);
        break;
    case 10:
        settype (value);
        break;
    case 11:
        setroomsize (value);
        break;
    };
};

int
Reverb::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
        return (Ppan);
        break;
    case 2:
        return (Ptime);
        break;
    case 3:
        return (Pidelay);
        break;
    case 4:
        return (Pidelayfb);
        break;
//      case 5: return(Prdelay);
//              break;
//      case 6: return(Perbalance);
//              break;
    case 7:
        return (Plpf);
        break;
    case 8:
        return (Phpf);
        break;
    case 9:
        return (Plohidamp);
        break;
    case 10:
        return (Ptype);
        break;
    case 11:
        return (Proomsize);
        break;
    };
    return (0);			//in case of bogus "parameter"
};
