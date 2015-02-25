/*
  rakarrack - a guitar effects software

 Compressor.C  -  Compressor Effect
 Based on artscompressor.cc by Matthias Kretz <kretz@kde.org>
 Stefan Westerfeld <stefan@space.twc.de>

  Copyright (C) 2008-2010 Josep Andreu
  Author: Josep Andreu

	Patches:
	September 2009  Ryan Billing (a.k.a. Transmogrifox)
		--Modified DSP code to fix discontinuous gain change at threshold.
		--Improved automatic gain adjustment function
		--Improved handling of knee
		--Added support for user-adjustable knee
		--See inline comments

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

#include <math.h>
#include "global.h"
#include "Compressor.h"
#define  MIN_GAIN  0.00001f        // -100dB  This will help prevent evaluation of denormal numbers

Compressor::Compressor (float * efxoutl_, float * efxoutr_, double samplerate)
{
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;

    rvolume = 0.0f;
    rvolume_db = 0.0f;
    lvolume = 0.0f;
    lvolume_db = 0.0f;
    tthreshold = -24;
    tratio = 4;
    toutput = -10;
    tatt = 20;
    trel = 50;
    a_out = 1;
    stereo = 0;
    tknee = 30;
    rgain = 1.0f;
    rgain_old = 1.0f;
    lgain = 1.0f;
    lgain_old = 1.0f;
    lgain_t = 1.0f;
    rgain_t = 1.0f;
    ratio = 1.0;
    kpct = 0.0f;
    peak = 0;
    lpeak = 0.0f;
    rpeak = 0.0f;
    rell = relr = attr = attl = 1.0f;

    ltimer = rtimer = 0;
    hold = (int) (samplerate*0.0125);  //12.5ms
    clipping = 0;
    limit = 0;

    cSAMPLE_RATE = 1.0/samplerate;
}

Compressor::~Compressor ()
{
}


void
Compressor::cleanup ()
{

    lgain = rgain = 1.0f;
    lgain_old = rgain_old = 1.0f;
    rpeak = 0.0f;
    lpeak = 0.0f;
    limit = 0;
    clipping = 0;
}


void
Compressor::Compressor_Change (int np, int value)
{

    switch (np) {

    case 1:
        tthreshold = value;
        thres_db = (float)tthreshold;    //implicit type cast int to float
        break;

    case 2:
        tratio = value;
        ratio = (float)tratio;
        break;

    case 3:
        toutput = value;
        break;

    case 4:
        tatt = value;
        att = cSAMPLE_RATE /(((float)value / 1000.0f) + cSAMPLE_RATE);
        attr = att;
        attl = att;
        break;

    case 5:
        trel = value;
        rel = cSAMPLE_RATE /(((float)value / 1000.0f) + cSAMPLE_RATE);
        rell = rel;
        relr = rel;
        break;

    case 6:
        a_out = value;
        break;

    case 7:
        tknee = value;  //knee expressed a percentage of range between thresh and zero dB
        kpct = (float)tknee/100.1f;
        break;

    case 8:
        stereo = value;
        break;
    case 9:
        peak = value;
        break;


    }

    kratio = logf(ratio)/LOG_2;  //  Log base 2 relationship matches slope
    knee = -kpct*thres_db;

    coeff_kratio = 1.0 / kratio;
    coeff_ratio = 1.0 / ratio;
    coeff_knee = 1.0 / knee;

    coeff_kk = knee * coeff_kratio;


    thres_mx = thres_db + knee;  //This is the value of the input when the output is at t+k
    makeup = -thres_db - knee/kratio + thres_mx/ratio;
    makeuplin = dB2rap(makeup);
    if (a_out)
        outlevel = dB2rap((float)toutput) * makeuplin;
    else
        outlevel = dB2rap((float)toutput);

}

int
Compressor::getpar (int np)
{

    switch (np)

    {

    case 1:
        return (tthreshold);
        break;
    case 2:
        return (tratio);
        break;
    case 3:
        return (toutput);
        break;
    case 4:
        return (tatt);
        break;
    case 5:
        return (trel);
        break;
    case 6:
        return (a_out);
        break;
    case 7:
        return (tknee);
        break;
    case 8:
        return (stereo);
        break;
    case 9:
        return (peak);
        break;
    }

    return (0);

}

void
Compressor::Compressor_Change_Preset (int dgui, int npreset)
{

    const int PRESET_SIZE = 10;
    const int NUM_PRESETS = 7;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //2:1
        {-30, 2, -6, 20, 120, 1, 0, 0, 0},
        //4:1
        {-26, 4, -8, 20, 120, 1, 10, 0, 0},
        //8:1
        {-24, 8, -12, 20, 35, 1, 30, 0, 0},
        //Final Limiter
        {-1, 15, 0, 5, 250, 0 ,0 ,1 ,1},
        //HarmonicEnhancer
        {-20, 15, -3, 5, 50, 0 ,0 ,1 ,1},
        //Band CompBand
        {-3, 2, 0, 5, 50, 1, 0, 1, 0},
        //End CompBand
        {-60, 2, 0, 10, 500, 1, 0, 1, 1},


    };

    if((dgui)&&(npreset>2)) {
        Fpre->ReadPreset(1,npreset-2,pdata);
        for (int n = 1; n < PRESET_SIZE; n++)
            Compressor_Change (n , pdata[n-1]);

    } else {
        for (int n = 1; n < PRESET_SIZE; n++)
            Compressor_Change (n , presets[npreset][n-1]);
    }

}



void
Compressor::out (float *efxoutl, float *efxoutr, uint32_t period)
{

    unsigned int i;


    for (i = 0; i < period; i++) {
        float rdelta = 0.0f;
        float ldelta = 0.0f;
//Right Channel

        if(peak) {
            if (rtimer > hold) {
                rpeak *= 0.9998f;   //The magic number corresponds to ~0.1s based on T/(RC + T),
                rtimer--;
            }
            if (ltimer > hold) {
                lpeak *= 0.9998f;	//leaky peak detector.
                ltimer --;  //keeps the timer from eventually exceeding max int & rolling over
            }
            ltimer++;
            rtimer++;
            if(rpeak<fabs(efxoutr[i])) {
                rpeak = fabs(efxoutr[i]);
                rtimer = 0;
            }
            if(lpeak<fabs(efxoutl[i])) {
                lpeak = fabs(efxoutl[i]);
                ltimer = 0;
            }

            if(lpeak>20.0f) lpeak = 20.0f;
            if(rpeak>20.0f) rpeak = 20.0f; //keeps limiter from getting locked up when signal levels go way out of bounds (like hundreds)

        } else {
            rpeak = efxoutr[i];
            lpeak = efxoutl[i];
        }

        if(stereo) {
            rdelta = fabsf (rpeak);
            if(rvolume < 0.9f) {
                attr = att;
                relr = rel;
            } else if (rvolume < 1.0f) {
                attr = att + ((1.0f - att)*(rvolume - 0.9f)*10.0f);	//dynamically change attack time for limiting mode
                relr = rel/(1.0f + (rvolume - 0.9f)*9.0f);  //release time gets longer when signal is above limiting
            } else {
                attr = 1.0f;
                relr = rel*0.1f;
            }

            if (rdelta > rvolume)
                rvolume = attr * rdelta + (1.0f - attr)*rvolume;
            else
                rvolume = relr * rdelta + (1.0f - relr)*rvolume;


            rvolume_db = rap2dB (rvolume);
            if (rvolume_db < thres_db) {
                rgain = outlevel;
            } else if (rvolume_db < thres_mx) {
                //Dynamic ratio that depends on volume.  As can be seen, ratio starts
                //at something negligibly larger than 1 once volume exceeds thres, and increases toward selected
                // ratio by the time it has reached thres_mx.  --Transmogrifox

                eratio = 1.0f + (kratio-1.0f)*(rvolume_db-thres_db)* coeff_knee;
                rgain =   outlevel*dB2rap(thres_db + (rvolume_db-thres_db)/eratio - rvolume_db);
            } else {
                rgain = outlevel*dB2rap(thres_db + coeff_kk + (rvolume_db-thres_mx)*coeff_ratio - rvolume_db);
                limit = 1;
            }

            if ( rgain < MIN_GAIN) rgain = MIN_GAIN;
            rgain_t = .4f * rgain + .6f * rgain_old;
        };

//Left Channel
        if(stereo)  {
            ldelta = fabsf (lpeak);
        } else  {
            ldelta = 0.5f*(fabsf (lpeak) + fabsf (rpeak));
        };  //It's not as efficient to check twice, but it's small expense worth code clarity

        if(lvolume < 0.9f) {
            attl = att;
            rell = rel;
        } else if (lvolume < 1.0f) {
            attl = att + ((1.0f - att)*(lvolume - 0.9f)*10.0f);	//dynamically change attack time for limiting mode
            rell = rel/(1.0f + (lvolume - 0.9f)*9.0f);  //release time gets longer when signal is above limiting
        } else {
            attl = 1.0f;
            rell = rel*0.1f;
        }

        if (ldelta > lvolume)
            lvolume = attl * ldelta + (1.0f - attl)*lvolume;
        else
            lvolume = rell*ldelta + (1.0f - rell)*lvolume;

        lvolume_db = rap2dB (lvolume);

        if (lvolume_db < thres_db) {
            lgain = outlevel;
        } else if (lvolume_db < thres_mx) { //knee region
            eratio = 1.0f + (kratio-1.0f)*(lvolume_db-thres_db)* coeff_knee;
            lgain =   outlevel*dB2rap(thres_db + (lvolume_db-thres_db)/eratio - lvolume_db);
        } else {
            lgain = outlevel*dB2rap(thres_db + coeff_kk + (lvolume_db-thres_mx)*coeff_ratio - lvolume_db);
            limit = 1;
        }


        if ( lgain < MIN_GAIN) lgain = MIN_GAIN;
        lgain_t = .4f * lgain + .6f * lgain_old;

        if (stereo) {
            efxoutl[i] *= lgain_t;
            efxoutr[i] *= rgain_t;
            rgain_old = rgain;
            lgain_old = lgain;
        } else {
            efxoutl[i] *= lgain_t;
            efxoutr[i] *= lgain_t;
            lgain_old = lgain;
        }

        if(peak) {
            if(efxoutl[i]>0.999f) {            //output hard limiting
                efxoutl[i] = 0.999f;
                clipping = 1;
            }
            if(efxoutl[i]<-0.999f) {
                efxoutl[i] = -0.999f;
                clipping = 1;
            }
            if(efxoutr[i]>0.999f) {
                efxoutr[i] = 0.999f;
                clipping = 1;
            }
            if(efxoutr[i]<-0.999f) {
                efxoutr[i] = -0.999f;
                clipping = 1;
            }
            //highly probably there is a more elegant way to do that, but what the hey...
        }
    }

}

