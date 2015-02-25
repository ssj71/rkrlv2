/*
  rakarrack - a guitar effects software

 pan.C  -  Auto/Pan -  Extra Stereo
  Copyright (C) 2008-2010 Josep Andreu
  Author: Josep Andreu

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


#include "Pan.h"



Pan::Pan (float *efxoutl_, float *efxoutr_, double sample_rate)
{

    efxoutl = efxoutl_;
    efxoutr = efxoutr_;

    lfo = new EffectLFO(sample_rate);

    Ppreset = 0;
    PERIOD = 256; //make our best guess for the initializing
    setpreset (Ppreset);

    lfo->effectlfoout (&lfol, &lfor);

    cleanup ();


};



Pan::~Pan ()
{
	delete lfo;
};

void
Pan::cleanup ()
{
};




void
Pan::out (float *smpsl, float *smpsr, uint32_t period)
{

    unsigned int i;
    float fPERIOD = period;
    float avg, ldiff, rdiff, tmp;
    float pp;
    float coeff_PERIOD = 1.0 / fPERIOD;
    float fi,P_i;



    if (PextraON) {

        for (i = 0; i < period; i++)

        {

            avg = (smpsl[i] + smpsr[i]) * .5f;

            ldiff = smpsl[i] - avg;
            rdiff = smpsr[i] - avg;

            tmp = avg + ldiff * mul;
            smpsl[i] = tmp*cdvalue;

            tmp = avg + rdiff * mul;
            smpsr[i] = tmp*sdvalue;


        }

    }

    if (PAutoPan) {

        ll = lfol;
        lr = lfor;
        lfo->effectlfoout (&lfol, &lfor);
        for (i = 0; i < period; i++) {
            fi = (float) i;
            P_i = (float) (period - i);

            pp = (ll * P_i + lfol * fi) * coeff_PERIOD;

            smpsl[i] *= pp * panning;

            pp =  (lr * P_i + lfor * fi) * coeff_PERIOD;

            smpsr[i] *= pp * (1.0f - panning);

        }

    }







};



void
Pan::setvolume (int Pvolume)
{
    this->Pvolume = Pvolume;
    outvolume = (float)Pvolume / 127.0f;
};



void
Pan::setpanning (int Ppanning)
{
    this->Ppanning = Ppanning;
    panning = ((float)Ppanning)/ 127.0f;
    dvalue= panning*M_PI_2;
    cdvalue=cosf(dvalue);
    sdvalue=sinf(dvalue);


};



void
Pan::setextra (int Pextra)
{
    this->Pextra = Pextra;
    mul = 4.0f * (float)Pextra / 127.0f;
};


void
Pan::setpreset (int npreset )
{
    const int PRESET_SIZE = 9;
    const int NUM_PRESETS = 2;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //AutoPan
        {64, 64, 26, 0, 0, 0, 0, 1, 0},
        //Extra Stereo
        {64, 64, 80, 0, 0, 0, 10, 0, 1}
    };

    if(npreset>NUM_PRESETS-1) {

        Fpre->ReadPreset(13,npreset-NUM_PRESETS+1,pdata);
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, pdata[n]);
    } else {

        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, presets[npreset][n]);
    }

    Ppreset = npreset;


};



void
Pan::changepar (int npar, int value)
{

    switch (npar) {
    case 0:
        setvolume (value);
        break;
    case 1:
        setpanning (value);
        break;
    case 2:
        lfo->Pfreq = value;
        lfo->updateparams (PERIOD);
        break;
    case 3:
        lfo->Prandomness = value;
        lfo->updateparams (PERIOD);
        break;
    case 4:
        lfo->PLFOtype = value;
        lfo->updateparams (PERIOD);
        break;
    case 5:
        lfo->Pstereo = value;
        lfo->updateparams (PERIOD);
        break;
    case 6:
        setextra (value);
        break;
    case 7:
        PAutoPan = value;
        break;
    case 8:
        PextraON = value;
        break;

    }


};


int
Pan::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
        return (Ppanning);
        break;
    case 2:
        return (lfo->Pfreq);
        break;
    case 3:
        return (lfo->Prandomness);
        break;
    case 4:
        return (lfo->PLFOtype);
        break;
    case 5:
        return (lfo->Pstereo);
        break;
    case 6:
        return (Pextra);
        break;
    case 7:
        return (PAutoPan);
        break;
    case 8:
        return (PextraON);
        break;
    default:
        return (0);

    }

};
