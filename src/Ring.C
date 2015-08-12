/*
  Rakarrack Audio FX
  Ring DSP Code based on "(author)" LADSPA plugin(swh-plugins).
  ZynAddSubFX effect structure - Copyright (C) 2002-2005 Nasca Octavian Paul
  Modified and adapted for rakarrack by Josep Andreu

  Ring.C - Ring Modulator effect

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
#include "Ring.h"




Ring::Ring (float * efxoutl_, float * efxoutr_, double sample_rate)
{

    efxoutl = efxoutl_;
    efxoutr = efxoutr_;
    SAMPLE_RATE = lrintf(sample_rate);

    sin_tbl = (float *) malloc(sizeof(float) * SAMPLE_RATE);
    tri_tbl = (float *) malloc(sizeof(float) * SAMPLE_RATE);
    squ_tbl = (float *) malloc(sizeof(float) * SAMPLE_RATE);
    saw_tbl = (float *) malloc(sizeof(float) * SAMPLE_RATE);
    //sin_tbl = new float[SAMPLE_RATE];//(float *) malloc(sizeof(float) * SAMPLE_RATE);
    //tri_tbl = new float[SAMPLE_RATE];//(float *) malloc(sizeof(float) * SAMPLE_RATE);
    //squ_tbl = new float[SAMPLE_RATE];//(float *) malloc(sizeof(float) * SAMPLE_RATE);
    //saw_tbl = new float[SAMPLE_RATE];//(float *) malloc(sizeof(float) * SAMPLE_RATE);

    Create_Tables(sample_rate);

    offset = 0;


    //default values
    Ppreset = 0;
    Pvolume = 50;
    Plrcross = 40;
    Plevel = 64;
    Pstereo = 0;

    scale = 1.0f;
    sin = 0.0f;
    tri = 0.0f;
    saw = 0.0f;
    squ = 0.0f;

    setpreset (Ppreset);
    cleanup ();
};

Ring::~Ring ()
{
	//delete [] sin_tbl;
	//delete [] sin_tbl;
	//delete [] sin_tbl;
	//delete [] sin_tbl;
	//free(sin_tbl);
	//free(tri_tbl);
	//free(squ_tbl);
	//free(saw_tbl);
};

/*
* Create Tables
*/
void
Ring::Create_Tables(float fSAMPLE_RATE)
{
    unsigned int i;
    float SR = fSAMPLE_RATE;


    for (i=0; i<SAMPLE_RATE; i++) sin_tbl[i]=sinf((float)i*D_PI/SR);
    for (i=0; i<SAMPLE_RATE; i++) tri_tbl[i]=acosf(cosf((float)i*D_PI/SR))/D_PI-1.0f;
    for (i=0; i<SAMPLE_RATE; i++) squ_tbl[i]=(i < SAMPLE_RATE/2) ? 1.0f : -1.0f;
    for (i=0; i<SAMPLE_RATE; i++) saw_tbl[i]=((2.0f*i)-SR)/SR;

}



/*
 * Cleanup the effect
 */
void
Ring::cleanup ()
{


};


/*
 * Apply the filters
 */


/*
 * Effect output
 */
void
Ring::out (float * smpsl, float * smpsr, uint32_t period)
{
    unsigned int i;
    float l, r, lout, rout, tmpfactor;

    float inputvol = (float) Pinput /127.0f;

    if (Pstereo != 0) {
        //Stereo
        for (i = 0; i < period; i++) {
            efxoutl[i] = smpsl[i] * inputvol;
            efxoutr[i] = smpsr[i] * inputvol;
            if(inputvol == 0.0) {
                efxoutl[i]=1.0;
                efxoutr[i]=1.0;
            }
        };
    } else {
        for (i = 0; i < period; i++) {
            efxoutl[i] =
                (smpsl[i]  +  smpsr[i] ) * inputvol;
            if (inputvol == 0.0) efxoutl[i]=1.0;
        };
    };


    for (i=0; i < period; i++) {
        tmpfactor =  depth * (scale * ( sin * sin_tbl[offset] + tri * tri_tbl[offset] + saw * saw_tbl[offset] + squ * squ_tbl[offset] ) + idepth) ;    //This is now mathematically equivalent, but less computation
        efxoutl[i] *= tmpfactor;

        if (Pstereo != 0) {
            efxoutr[i] *= tmpfactor;
        }
        offset += Pfreq;
        if (offset > SAMPLE_RATE) offset -=SAMPLE_RATE;
    }


    if (Pstereo == 0) memcpy (efxoutr , efxoutl, period * sizeof(float));

    float level = dB2rap (60.0f * (float)Plevel / 127.0f - 40.0f);

    for (i= 0; i<period; i++) {
        lout = efxoutl[i];
        rout = efxoutr[i];


        l = lout * (1.0f - lrcross) + rout * lrcross;
        r = rout * (1.0f - lrcross) + lout * lrcross;

        lout = l;
        rout = r;

        //efxoutl[i] = lout * level * panning;
        //efxoutr[i] = rout * level * (1.0f-panning);
        efxoutl[i] = lout * level * (1.0f-panning);
        efxoutr[i] = rout * level * panning;

    }



};


/*
 * Parameter control
 */


void
Ring::setpanning (int Ppan)
{
    Ppanning = Ppan;
    panning = (float)(Ppanning+64) / 128.0f;
// is Ok ...

};


void
Ring::setlrcross (int Plrc)
{
    Plrcross = Plrc;
    lrcross = (float)(Plrcross+64) / 128.0f;

};


void
Ring::setscale()
{
    scale = sin + tri + saw + squ;
    if (scale==0.0) scale = 1.0;
    scale = 1.0 / scale;
}

void
Ring::setpreset (int npreset)
{
    const int PRESET_SIZE = 13;
    const int NUM_PRESETS = 6;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Saw-Sin
        {-64, 0, -64, 64, 35, 1, 0, 20, 0, 40, 0, 64, 1},
        //E string
        {0, 0, 0, 64, 100, 82, 0, 100, 0, 0, 0, 64, 0},
        //A string
        {0, 0, 0, 64, 100, 110, 0, 0, 100, 50, 0, 64, 0},
        //dissonance
        {0, 0, 0, 64, 100, 817, 0, 20, 0, 100, 0, 64, 1},
        //Fast Beat
        {0, 0, 0, 64, 100, 15, 0, 20, 0, 100, 0, 64, 1},
        //Ring Amp
        {0, 0, 0, 64, 100, 1, 0, 20, 0, 100, 0, 64, 0},
    };

    if(npreset>NUM_PRESETS-1) {
        Fpre->ReadPreset(21,npreset-NUM_PRESETS+1, pdata);
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
Ring::changepar (int npar, int value)
{
    switch (npar) {
    case 0:
        Pvolume = value;
        outvolume = (float)(64+value)/128.0f;
        break;
    case 1:
        setpanning (value);
        break;
    case 2:
        setlrcross (value);
        break;
    case 3:
        Plevel = value;
        break;
    case 4:
        Pdepthp = value;
        depth = (float) Pdepthp / 100.0;
        idepth = 1.0f - depth;
        break;
    case 5:
        if(value > 20000) {	//Make sure bad inputs can't cause buffer overflow
            Pfreq = 20000;
        } else if (value < 1) {
            Pfreq = 1;
        } else {
            Pfreq = value;
        }
        break;
    case 6:
        if (value > 1)
            value = 1;
        Pstereo = value;
        break;
    case 7:
        Psin = value;
        sin = (float) Psin / 100.0;
        setscale();
        break;
    case 8:
        Ptri = value;
        tri = (float) Ptri / 100.0;
        setscale();
        break;
    case 9:
        Psaw = value;
        saw = (float) Psaw / 100.0;
        setscale();
        break;
    case 10:
        Psqu = value;
        squ = (float) Psqu / 100.0;
        setscale();
        break;
    case 11:
        Pinput = value;
        break;
    case 12:
        Pafreq = value;
        break;

    };
};

int
Ring::getpar (int npar)
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
        return (Plevel);
        break;
    case 4:
        return (Pdepthp);
        break;
    case 5:
        return (Pfreq);
        break;
    case 6:
        return (Pstereo);
        break;
    case 7:
        return (Psin);
        break;
    case 8:
        return (Ptri);
        break;
    case 9:
        return (Psaw);
        break;
    case 10:
        return (Psqu);
        break;
    case 11:
        return (Pinput);
        break;
    case 12:
        return (Pafreq);

    };
    return (0);			//in case of bogus parameter number
};

