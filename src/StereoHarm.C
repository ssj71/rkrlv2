/*
  rakarrack - a guitar effects software

  StereoHarm.C  -  Stereo Harmonizer
  Copyright (C) 2008 Josep Andreu
  Author:  Josep Andreu

  Using Stephan M. Bernsee smbPitchShifter engine.

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


#include "StereoHarm.h"



StereoHarm::StereoHarm (float *efxoutl_, float *efxoutr_, long int Quality, int DS, int uq, int dq, uint32_t intermediate_bufsize, double sample_rate)
{ 
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;
    hq = Quality;
    SAMPLE_RATE = (unsigned int)sample_rate;
    adjust(DS, intermediate_bufsize);
    nPERIOD = lrintf(intermediate_bufsize*nRATIO);

    templ = (float *) malloc (sizeof (float) * intermediate_bufsize);
    tempr = (float *) malloc (sizeof (float) * intermediate_bufsize);

    outil = (float *) malloc (sizeof (float) * nPERIOD);
    outir = (float *) malloc (sizeof (float) * nPERIOD);

    outol = (float *) malloc (sizeof (float) * nPERIOD);
    outor = (float *) malloc (sizeof (float) * nPERIOD);

    memset (outil, 0, sizeof (float) * nPERIOD);
    memset (outir, 0, sizeof (float) * nPERIOD);

    memset (outol, 0, sizeof (float) * nPERIOD);
    memset (outor, 0, sizeof (float) * nPERIOD);

    U_Resample = new Resample(dq);
    D_Resample = new Resample(uq);


    chromel=0.0;
    chromer=0.0;


    PSl = new PitchShifter (window, hq, nfSAMPLE_RATE);
    PSl->ratio = 1.0f;
    PSr = new PitchShifter (window, hq, nfSAMPLE_RATE);
    PSr->ratio = 1.0f;

    Ppreset = 0;
    PMIDI = 0;
    mira = 0;
    setpreset (Ppreset);


    cleanup ();

};



StereoHarm::~StereoHarm ()
{
	free(templ);
	free(tempr);
	free(outil);
	free(outir);
	free(outol);
	free(outor);
	delete U_Resample;
	delete D_Resample;
	delete PSl;
	delete PSr;
};

void
StereoHarm::cleanup ()
{
    mira = 0;
    chromel=0;
    chromer=0;
    memset(outil, 0, sizeof(float)*nPERIOD);
    memset(outir, 0, sizeof(float)*nPERIOD);
    memset(outol, 0, sizeof(float)*nPERIOD);
    memset(outor, 0, sizeof(float)*nPERIOD);

};


void
StereoHarm::out (float *smpsl, float *smpsr, uint32_t period)
{

    unsigned int i;

    nPERIOD = lrintf(period*nRATIO);
    u_up= (double)nPERIOD / (double)period;
    u_down= (double)period / (double)nPERIOD;

    if(DS_state != 0) {
//        memcpy(templ, smpsl,sizeof(float)*period);
//        memcpy(tempr, smpsr,sizeof(float)*period);
        U_Resample->out(smpsl,smpsr,templ,tempr,period,u_up);
    }


    for (i = 0; i < nPERIOD; i++) {


        outil[i] = templ[i] * (1.0f - lrcross) + tempr[i] * lrcross;
        if (outil[i] > 1.0)
            outil[i] = 1.0f;
        if (outil[i] < -1.0)
            outil[i] = -1.0f;

        outir[i] = tempr[i] * (1.0f - lrcross) + templ[i] * lrcross;
        if (outir[i] > 1.0)
            outir[i] = 1.0f;
        if (outir[i] < -1.0)
            outir[i] = -1.0f;

    }

    if ((PMIDI) || (PSELECT)) {
        PSl->ratio = r_ratiol;
        PSr->ratio = r_ratior;
    }

    if (PSl->ratio != 1.0f) {
        PSl->smbPitchShift (PSl->ratio, nPERIOD, window, hq, nfSAMPLE_RATE, outil, outol);
    } else
        memcpy(outol,outil,sizeof(float)*nPERIOD);


    if (PSr->ratio != 1.0f) {
        PSr->smbPitchShift (PSr->ratio, nPERIOD, window, hq, nfSAMPLE_RATE, outir, outor);
    } else
        memcpy(outor,outir,sizeof(float)*nPERIOD);


    if(DS_state != 0) {
        D_Resample->out(outol,outor,templ,tempr,nPERIOD,u_down);
    } else {
        memcpy(templ, outol,sizeof(float)*period);
        memcpy(tempr, outor,sizeof(float)*period);

    }


    //for (i = 0; i < period; i++) {
        //efxoutl[i] = templ[i] * gainl;
        //efxoutr[i] = tempr[i] * gainr;
        
    //}
    for (i = 0; i < period; i++) {
        efxoutl[i] = templ[i] * gainl * (1.0f - lrcross) + tempr[i] * gainr * lrcross;
        efxoutr[i] = tempr[i] * gainr * (1.0f - lrcross) + templ[i] * gainl * lrcross;
    }

};


void
StereoHarm::setvolume (int value)
{
    this->Pvolume = value;
    outvolume = (float)Pvolume / 127.0f;
};



void
StereoHarm::setgain (int chan, int value)
{

    switch(chan) {
    case 0:
        Pgainl = value;
        gainl = (float)Pgainl / 127.0f;
        gainl *=2.0;
        break;
    case 1:
        Pgainr = value;
        gainr = (float)Pgainr / 127.0f;
        gainr *=2.0;
        break;

    }
};


void
StereoHarm::setinterval (int chan, int value)
{

    switch(chan) {
    case 0:
        Pintervall = value;
        intervall = (float)Pintervall - 12.0f;
        PSl->ratio = powf(2.0f,intervall / 12.0f)+chromel;
        if (Pintervall % 12 == 0)
            mira = 0;
        else
            mira = 1;
        break;

    case 1:
        Pintervalr = value;
        intervalr = (float)Pintervalr - 12.0f;
        PSr->ratio = powf(2.0f,intervalr / 12.0f)+chromer;
        if (Pintervalr % 12 == 0)
            mira = 0;
        else
            mira = 1;
        break;
    }
};



void
StereoHarm::setchrome (int chan, int value)
{

    float max,min;
    max = 0.0;
    min = 0.0;

    switch(chan) {
    case 0:
        max = powf(2.0f,(intervall+1.0f) / 12.0f);
        min = powf(2.0f,(intervall-1.0f) / 12.0f);
        break;
    case 1:
        max = powf(2.0f,(intervalr+1.0f) / 12.0f);
        min = powf(2.0f,(intervalr-1.0f) / 12.0f);
        break;
    }

    if (max > 2.0) max=2.0f;
    if (min < 0.5) min=0.5f;


    switch(chan) {
    case 0:
        Pchromel=value;
        chromel=(max-min)/4000.0f*(float)value;
        PSl->ratio=powf(2.0f,intervall/12.0f)+chromel;
        break;
    case 1:
        Pchromer=value;
        chromer=(max-min)/4000.0f*(float)value;
        PSr->ratio=powf(2.0f,intervalr/12.0f)+chromer;
        break;
    }

}

void
StereoHarm::setMIDI (int value)
{

    this->PMIDI = value;
}


void
StereoHarm::adjust(int DS, uint32_t period)
{

    DS_state=DS;
    float fSAMPLE_RATE = SAMPLE_RATE;


    switch(DS) {

    case 0:
        //nPERIOD = period;
        nRATIO = 1;
        nSAMPLE_RATE = SAMPLE_RATE;
        nfSAMPLE_RATE = fSAMPLE_RATE;
        window = 2048;
        break;

    case 1:
        //nPERIOD = lrintf(fPERIOD*96000.0f/fSAMPLE_RATE);
        nRATIO = 96000.0f/fSAMPLE_RATE;
        nSAMPLE_RATE = 96000;
        nfSAMPLE_RATE = 96000.0f;
        window = 2048;
        break;


    case 2:
        //nPERIOD = lrintf(fPERIOD*48000.0f/fSAMPLE_RATE);
        nRATIO = 48000.0f/fSAMPLE_RATE;
        nSAMPLE_RATE = 48000;
        nfSAMPLE_RATE = 48000.0f;
        window = 2048;
        break;

    case 3:
        //nPERIOD = lrintf(fPERIOD*44100.0f/fSAMPLE_RATE);
        nRATIO = 44100.0f/fSAMPLE_RATE;
        nSAMPLE_RATE = 44100;
        nfSAMPLE_RATE = 44100.0f;
        window = 2048;
        break;

    case 4:
        //nPERIOD = lrintf(fPERIOD*32000.0f/fSAMPLE_RATE);
        nRATIO = 32000.0f/fSAMPLE_RATE;
        nSAMPLE_RATE = 32000;
        nfSAMPLE_RATE = 32000.0f;
        window = 2048;
        break;

    case 5:
        //nPERIOD = lrintf(fPERIOD*22050.0f/fSAMPLE_RATE);
        nRATIO = 22050.0f/fSAMPLE_RATE;
        nSAMPLE_RATE = 22050;
        nfSAMPLE_RATE = 22050.0f;
        window = 1024;
        break;

    case 6:
        //nPERIOD = lrintf(fPERIOD*16000.0f/fSAMPLE_RATE);
        nRATIO = 16000.0f/fSAMPLE_RATE;
        nSAMPLE_RATE = 16000;
        nfSAMPLE_RATE = 16000.0f;
        window = 1024;
        break;

    case 7:
        //nPERIOD = lrintf(fPERIOD*12000.0f/fSAMPLE_RATE);
        nRATIO = 12000.0f/fSAMPLE_RATE;
        nSAMPLE_RATE = 12000;
        nfSAMPLE_RATE = 12000.0f;
        window = 512;
        break;

    case 8:
        //nPERIOD = lrintf(fPERIOD*8000.0f/fSAMPLE_RATE);
        nRATIO = 8000.0f/fSAMPLE_RATE;
        nSAMPLE_RATE = 8000;
        nfSAMPLE_RATE = 8000.0f;
        window = 512;
        break;

    case 9:
        //nPERIOD = lrintf(fPERIOD*4000.0f/fSAMPLE_RATE);
        nRATIO = 4000.0f/fSAMPLE_RATE;
        nSAMPLE_RATE = 4000;
        nfSAMPLE_RATE = 4000.0f;
        window = 256;
        break;
    }



    u_up= (double)nPERIOD / (double)period;
    u_down= (double)period / (double)nPERIOD;
}


void
StereoHarm::setlrcross (int value)
{
    Plrcross = value;
    lrcross = (float)Plrcross / 127.0f;

};



void
StereoHarm::setpreset (int npreset)
{
    const int PRESET_SIZE = 12;
    const int NUM_PRESETS = 4;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Plain
        {64, 64, 12, 0, 64, 12, 0, 0, 0, 0, 0, 64},
        //Octavador
        {64, 64, 0, 0, 64, 0, 0, 0, 0, 0, 0, 64},
        //Chorus
        {64, 64, 12, 80, 64, 12, -80, 0, 0, 0, 0, 64},
        //Chorus
        {64, 64, 12, 280, 64, 12, -280, 0, 0, 0, 0, 64}

    };

    cleanup();
    if(npreset>NUM_PRESETS-1) {
        Fpre->ReadPreset(42,npreset-NUM_PRESETS+1,pdata);
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, pdata[n]);
    } else {
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, presets[npreset][n]);
    }

    Ppreset = npreset;


};



void
StereoHarm::changepar (int npar, int value)
{

    switch (npar) {
    case 0:
        setvolume (value);
        break;
    case 1:
        setgain (0,value);
        break;
    case 2:
        setinterval(0,value);
        break;
    case 3:
        setchrome(0, value);
        break;
    case 4:
        setgain (1,value);
        break;
    case 5:
        setinterval(1,value);
        break;
    case 6:
        setchrome(1, value);
        break;
    case 7:
        PSELECT = value;;
        break;
    case 8:
        Pnote = value;
        break;
    case 9:
        Ptype = value;
        if (Ptype==0) {
            setchrome(0, Pchromel);
            setchrome(1, Pchromer);
        }
        break;
    case 10:
        setMIDI (value);
        break;
    case 11:
        setlrcross(value);
        break;


    }


};


int
StereoHarm::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
        return (Pgainl);
        break;
    case 2:
        return (Pintervall);
        break;
    case 3:
        return (Pchromel);
        break;
    case 4:
        return (Pgainr);
        break;
    case 5:
        return (Pintervalr);
        break;
    case 6:
        return (Pchromer);
        break;
    case 7:
        return (PSELECT);
        break;
    case 8:
        return (Pnote);
        break;
    case 9:
        return (Ptype);
        break;
    case 10:
        return (PMIDI);
        break;
    case 11:
        return(Plrcross);
        break;
    default:
        return (0);

    }

};
