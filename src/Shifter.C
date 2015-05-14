/*
  rakarrack - a guitar effects software

  Shifter.C  -  Shifter
  Copyright (C) 2008-2010 Josep Andreu
  Author: Josep Andreu

  Using Stephan M. Bernsee smbPtichShifter engine.

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


#include "Shifter.h"



Shifter::Shifter (float *efxoutl_, float *efxoutr_, long int Quality, int DS, int uq, int dq,
				  double sample_rate, uint32_t intermediate_bufsize)
{



    efxoutl = efxoutl_;
    efxoutr = efxoutr_;
    hq = Quality;
    adjust(DS,sample_rate);

    //temp value till period actually known
    nPERIOD = intermediate_bufsize*nRATIO;

    templ = (float *) malloc (sizeof (float) * intermediate_bufsize);
    tempr = (float *) malloc (sizeof (float) * intermediate_bufsize);

    outi = (float *) malloc (sizeof (float) * nPERIOD);
    outo = (float *) malloc (sizeof (float) * nPERIOD);

    U_Resample = new Resample(dq);
    D_Resample = new Resample(uq);

    PS = new PitchShifter (window, hq, nfSAMPLE_RATE);
    PS->ratio = 1.0f;

    state = IDLE;
    env = 0.0f;
    tune = 0.0f;
    Pupdown = 0;
    Pinterval = 0;
    Ppreset = 0;
    setpreset (Ppreset);
    cleanup ();

};



Shifter::~Shifter ()
{
	free(templ);
	free(tempr);
	free(outi);
	free(outo);
	delete PS;
};

void
Shifter::cleanup ()
{
    state = IDLE;
    memset(outi, 0, sizeof(float)*nPERIOD);
    memset(outo, 0, sizeof(float)*nPERIOD);
};

void
Shifter::adjust(int DS, double SAMPLE_RATE)
{

    DS_state=DS;

    switch(DS) {

    case 0:
        nRATIO = 1;
        nSAMPLE_RATE = SAMPLE_RATE;
        nfSAMPLE_RATE = SAMPLE_RATE;
        window = 2048;
        break;

    case 1:
        nRATIO = 96000.0f/SAMPLE_RATE;
        nSAMPLE_RATE = 96000;
        nfSAMPLE_RATE = 96000.0f;
        window = 2048;
        break;


    case 2:
        nRATIO = 48000.0f/SAMPLE_RATE;
        nSAMPLE_RATE = 48000;
        nfSAMPLE_RATE = 48000.0f;
        window = 2048;
        break;

    case 3:
        nRATIO = 44100.0f/SAMPLE_RATE;
        nSAMPLE_RATE = 44100;
        nfSAMPLE_RATE = 44100.0f;
        window = 2048;
        break;

    case 4:
        nRATIO = 32000.0f/SAMPLE_RATE;
        nSAMPLE_RATE = 32000;
        nfSAMPLE_RATE = 32000.0f;
        window = 2048;
        break;

    case 5:
        nRATIO = 22050.0f/SAMPLE_RATE;
        nSAMPLE_RATE = 22050;
        nfSAMPLE_RATE = 22050.0f;
        window = 1024;
        break;

    case 6:
        nRATIO = 16000.0f/SAMPLE_RATE;
        nSAMPLE_RATE = 16000;
        nfSAMPLE_RATE = 16000.0f;
        window = 1024;
        break;

    case 7:
        nRATIO = 12000.0f/SAMPLE_RATE;
        nSAMPLE_RATE = 12000;
        nfSAMPLE_RATE = 12000.0f;
        window = 512;
        break;

    case 8:
        nRATIO = 8000.0f/SAMPLE_RATE;
        nSAMPLE_RATE = 8000;
        nfSAMPLE_RATE = 8000.0f;
        window = 512;
        break;

    case 9:
        nRATIO = 4000.0f/SAMPLE_RATE;
        nSAMPLE_RATE = 4000;
        nfSAMPLE_RATE = 4000.0f;
        window = 256;
        break;
    }
}


void
Shifter::out (float *smpsl, float *smpsr, uint32_t period)
{

    int i;
    float sum;
    float use;

    //This should probably be moved to a separate function so it doesn't need to recalculate every time
    nPERIOD = lrintf((float)period*nRATIO);
    u_up= (double)nPERIOD / (double)period;
    u_down= (double)period / (double)nPERIOD;

    if(DS_state != 0) {
        memcpy(templ, smpsl,sizeof(float)*period);
        memcpy(tempr, smpsr,sizeof(float)*period);
        U_Resample->out(templ,tempr,smpsl,smpsr,period,u_up);
    }

    for (i=0; i < nPERIOD; i++) {
        if((Pmode == 0) || (Pmode ==2)) {
            sum = fabsf(smpsl[i])+fabsf(smpsr[i]);
            if (sum>env) env = sum;
            else env=sum*ENV_TR+env*(1.0f-ENV_TR);

            if (env <= tz_level) {
                state=IDLE;
                tune = 0.0;
            }

            if ((state == IDLE) && (env >= t_level)) state=UP;

            if (state==UP) {
                tune +=a_rate;
                if (tune >=1.0f) state = WAIT;
            }

            if (state==WAIT) {
                tune = 1.0f;
                if (env<td_level)
                    state=DOWN;
            }

            if (state==DOWN) {
                tune -= d_rate;
                if(tune<=0.0) {
                    tune = 0.0;
                    state=IDLE;
                }
            }
        }
        outi[i] = (smpsl[i] + smpsr[i])*.5;
        if (outi[i] > 1.0)
            outi[i] = 1.0f;
        if (outi[i] < -1.0)
            outi[i] = -1.0f;

    }


    if (Pmode == 1) use = whammy;
    else use = tune;
    if ((Pmode == 0) && (Pinterval == 0)) use = tune * whammy;
    if (Pmode == 2) use = 1.0f - tune;

    if(Pupdown)
        PS->ratio = 1.0f-(1.0f-range)*use;
    else
        PS->ratio = 1.0f+((range-1.0f)*use);


    PS->smbPitchShift (PS->ratio, nPERIOD, window, hq, nfSAMPLE_RATE, outi, outo);

    for (i = 0; i < nPERIOD; i++) {
        templ[i] = outo[i] * gain * panning;
        tempr[i] = outo[i] * gain * (1.0f - panning);
    }


    if(DS_state != 0) {
        D_Resample->out(templ,tempr,efxoutl,efxoutr,nPERIOD,u_down);

    } else {
        memcpy(efxoutl, templ,sizeof(float)*period);
        memcpy(efxoutr, tempr,sizeof(float)*period);
    }

};



void
Shifter::setvolume (int value)
{
    this->Pvolume = value;
    outvolume = (float)Pvolume / 127.0f;
};



void
Shifter::setpanning (int value)
{
    this->Ppan = value;
    panning = (float)Ppan / 127.0f;
};



void
Shifter::setgain (int value)
{
    this->Pgain = value;
    gain = (float)Pgain / 127.0f;
    gain *=2.0f;
};


void
Shifter::setinterval (int value)
{
    interval = (float) value;
    if ((Pmode == 0) && ( Pinterval == 0)) interval = 1.0f;
    if(Pupdown) interval *=-1.0f;
    range = powf (2.0f, interval / 12.0f);

};



void
Shifter::setpreset (int npreset)
{
    const int PRESET_SIZE = 10;
    const int NUM_PRESETS = 5;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Fast
        {0, 64, 64, 200, 200, -20, 2, 0, 0, 0},
        //Slowup
        {0, 64, 64, 900, 200, -20, 2, 0, 0, 0},
        //Slowdown
        {0, 64, 64, 900, 200, -20, 3, 1, 0, 0},
        //Chorus
        {64, 64, 64, 0, 0, -20, 1, 0, 1, 22},
        //Trig Chorus
        {64, 64, 64, 250, 100, -10, 0, 0, 0, 25}
    };

    if(npreset>NUM_PRESETS-1) {
        Fpre->ReadPreset(38,npreset-NUM_PRESETS+1, pdata);
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, pdata[n]);
    } else {
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, presets[npreset][n]);
    }
    Ppreset = npreset;


};



void
Shifter::changepar (int npar, int value)
{

    switch (npar) {
    case 0:
        setvolume (value);
        break;
    case 1:
        setpanning (value);
        break;
    case 2:
        setgain (value);
        break;
    case 3:
        Pattack = value;
        a_rate = 1000.0f / ((float)Pattack * nfSAMPLE_RATE);
        break;
    case 4:
        Pdecay = value;
        d_rate = 1000.0f / ((float)Pdecay * nfSAMPLE_RATE);
        break;
    case 5:
        Pthreshold = value;
        t_level = dB2rap ((float)Pthreshold);
        td_level = t_level*.75f;
        tz_level = t_level*.5f;
        break;
    case 6:
        Pinterval = value;
        setinterval(Pinterval);
        break;
    case 7:
        Pupdown = value;
        setinterval(Pinterval);
        break;
    case 8:
        Pmode = value;
        break;
    case 9:
        Pwhammy = value;
        whammy = (float) value / 127.0f;
        break;

    }


};


int
Shifter::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
        return (Ppan);
        break;
    case 2:
        return (Pgain);
        break;
    case 3:
        return (Pattack);
        break;
    case 4:
        return (Pdecay);
        break;
    case 5:
        return (Pthreshold);
        break;
    case 6:
        return (Pinterval);
        break;
    case 7:
        return (Pupdown);
        break;
    case 8:
        return (Pmode);
        break;
    case 9:
        return (Pwhammy);
    }

    return(0);
};
