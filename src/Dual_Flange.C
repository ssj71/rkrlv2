/*
  Rakarrack Audio FX

  Dual_Flange.C - Super Flanger
  Copyright (C) 2010 Ryan Billing
  Authors:
  Ryan Billing (a.k.a Transmogrifox)  --  Signal Processing
  Copyright (C) 2010 Ryan Billing

  Nasca Octavian Paul -- Remnants of ZynAddSubFX Echo.h structure and utility routines common to ZynSubFX source
  Copyright (C) 2002-2005 Nasca Octavian Paul

  Higher intensity flanging accomplished by picking two points out of the delay line to create a wider notch filter.

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
#include "Dual_Flange.h"

Dflange::Dflange (float * efxoutl_, float * efxoutr_, double sample_rate)
{
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;
    fSAMPLE_RATE = sample_rate;

    //default values
    Ppreset = 0;

    maxx_delay = (int) sample_rate * 0.055f;
    ldelay = new float[maxx_delay];
    rdelay = new float[maxx_delay];
    zldelay = new float[maxx_delay];
    zrdelay = new float[maxx_delay];

    ldelayline0  = new delayline(0.055f, 2, sample_rate);
    rdelayline0  = new delayline(0.055f, 2, sample_rate);
    ldelayline1  = new delayline(0.055f, 2, sample_rate);
    rdelayline1  = new delayline(0.055f, 2, sample_rate);
    ldelayline0 -> set_averaging(0.05f);
    rdelayline0 -> set_averaging(0.05f);
    ldelayline0->set_mix( 0.5f );
    rdelayline0->set_mix( 0.5f );
    ldelayline1 -> set_averaging(0.05f);
    rdelayline1 -> set_averaging(0.05f);
    ldelayline1->set_mix( 0.5f );
    rdelayline1->set_mix( 0.5f );

    fsubtract = 0.5f;
    fhidamp = 1.0f;
    fwidth = 800;
    fdepth = 50;
    zcenter = (int) fSAMPLE_RATE/floorf(0.5f * (fdepth + fwidth));
    base = 7.0f;		//sets curve of modulation to frequency relationship
    ibase = 1.0f/base;
    //default values
    Ppreset = 0;
    Pintense = 0;
    rsA = 0.0f;
    rsB = 0.0f;
    lsA  = 0.0f;
    lsB = 0.0f;
    logmax = logf(1000.0f)/logf(2.0f);

    kl = kr = 0;

    lfo = new EffectLFO(sample_rate);
    PERIOD = 255;//best guess for init
    setpreset (Ppreset);
    cleanup ();
};

Dflange::~Dflange ()
{
    delete[] ldelay;
    delete[] rdelay;
    delete[] zldelay;
    delete[] zrdelay;
    delete ldelayline0;
    delete rdelayline0;
    delete ldelayline1;
    delete rdelayline1;
	delete lfo;
};

/*
 * Cleanup the effect
 */
void
Dflange::cleanup ()
{
    int i;
    for (i = 0; i < maxx_delay; i++) {
        ldelay[i] = 0.0;
        rdelay[i] = 0.0;
        zldelay[i] = 0.0;
        zrdelay[i] = 0.0;
    };

    //loop variables
    l = 0.0f;
    r = 0.0f;
    ldl = 0.0f;
    rdl = 0.0f;
    rflange0 = 0.0f;
    lflange0 = 0.0f;
    rflange1 = 0.0f;
    lflange1 = 0.0f;

};


/*
 * Effect output
 */
void
Dflange::out (float * smpsl, float * smpsr, uint32_t period)
{
    unsigned int i;
    //deal with LFO's
    int tmp0, tmp1;
    period_const = 1.0f/(float)period;
    PERIOD = period;

    float lfol, lfor, lmod, rmod, lmodfreq, rmodfreq, rx0, rx1, lx0, lx1;
    float ldif0, ldif1, rdif0, rdif1;  //Difference between fractional delay and floor(fractional delay)
    float drA, drB, dlA, dlB;	//LFO inside the loop.

    lfo->effectlfoout (&lfol, &lfor);
    lmod = lfol;
    if(Pzero && Pintense) rmod = 1.0f - lfol;  //using lfol is intentional
    else rmod = lfor;

    if(Pintense) {
//do intense stuff
        lmodfreq = (f_pow2(lmod*lmod*logmax)) * fdepth;  //2^x type sweep for musical interpretation of moving delay line.
        rmodfreq = (f_pow2(rmod*rmod*logmax)) * fdepth;  //logmax depends on depth
        rflange0 = 0.5f/rmodfreq;		//Turn the notch frequency into 1/2 period delay
        rflange1 = rflange0 + (1.0f - foffset)/fdepth;				//Set relationship of second delay line
        lflange0 = 0.5f/lmodfreq;
        lflange1 = lflange0 + (1.0f - foffset)/fdepth;

        rx0 = (rflange0 - oldrflange0) * period_const;  //amount to add each time around the loop.  Less processing of linear LFO interp inside the loop.
        rx1 =  (rflange1 - oldrflange1) * period_const;
        lx0 = (lflange0 - oldlflange0) * period_const;
        lx1  = (lflange1 - oldlflange1) * period_const;

// Now there is a fractional amount to add
        drA = oldrflange0;
        drB = oldrflange1;
        dlA = oldlflange0;
        dlB = oldlflange1;

        oldrflange0 = rflange0;
        oldrflange1 = rflange1;
        oldlflange0 = lflange0;
        oldlflange1 = lflange1;
        //lfo ready...

        if(Pzero) {
            for (i = 0; i < period; i++) {

                ldl = smpsl[i] * lpan + ldl * ffb;
                rdl = smpsr[i] * rpan + rdl * ffb;

                //LowPass Filter
                ldl = ldl * (1.0f - fhidamp) + oldl * fhidamp;
                rdl = rdl * (1.0f - fhidamp) + oldr * fhidamp;
                oldl = ldl + DENORMAL_GUARD;
                oldr = rdl + DENORMAL_GUARD;
                /*
                Here do the delay line stuff
                basically,
                dl1(dl2(smps));
                ^^This runs two flangers in series so you can get a double notch
                */

                ldl = ldelayline0->delay(ldl,dlA, 0, 1, 0)  + ldelayline1->delay(ldl,drA, 0, 1, 0);
                rdl = rdelayline0->delay(rdl,dlB, 0, 1, 0) + rdelayline1->delay(rdl,drB, 0, 1, 0);

                efxoutl[i] = ldl = ldl * flrcross + rdl * frlcross;
                efxoutr[i] = rdl = rdl * flrcross + ldl * frlcross;

// Increment LFO
                drA += rx0;
                drB += rx1;
                dlA += lx0;
                dlB += lx1;
            }
        } else {
            for (i = 0; i < period; i++) {

                ldl = smpsl[i] * lpan + ldl * ffb;
                rdl = smpsr[i] * rpan + rdl * ffb;

                //LowPass Filter
                ldl = ldl * (1.0f - fhidamp) + oldl * fhidamp;
                rdl = rdl * (1.0f - fhidamp) + oldr * fhidamp;
                oldl = ldl + DENORMAL_GUARD;
                oldr = rdl + DENORMAL_GUARD;
                /*
                Here do the delay line stuff
                basically,
                dl1(dl2(smps));
                ^^This runs two flangers in series so you can get a double notch
                */

                ldl = ldelayline0->delay(ldl,dlA, 0, 1, 0);
                ldl = ldelayline1->delay(ldl,dlB, 0, 1, 0);

                rdl = rdelayline0->delay(rdl,drA, 0, 1, 0);
                rdl = rdelayline1->delay(rdl,drB, 0, 1, 0);

                efxoutl[i] = ldl = ldl * flrcross + rdl * frlcross;
                efxoutr[i] = rdl = rdl * flrcross + ldl * frlcross;

// Increment LFO
                drA += rx0;
                drB += rx1;
                dlA += lx0;
                dlB += lx1;
            }
        }


    } else {

        lmodfreq = fdepth + fwidth*(powf(base, lmod) - 1.0f)*ibase;	//sets frequency of lowest notch. // 20 <= fdepth <= 4000 // 20 <= width <= 16000 //
        rmodfreq = fdepth + fwidth*(powf(base, rmod) - 1.0f)*ibase;


        if (lmodfreq > 10000.0f)
            lmodfreq = 10000.0f;
        else if (lmodfreq < 10.0f)
            lmodfreq = 10.0f;
        if (rmodfreq > 10000.0)
            rmodfreq = 10000.0f;
        else if (rmodfreq < 10.0f)
            rmodfreq = 10.0f;

        rflange0 = fSAMPLE_RATE * 0.5f/rmodfreq;		//Turn the notch frequency into a number for delay
        rflange1 = rflange0 * foffset;				//Set relationship of second delay line
        lflange0 = fSAMPLE_RATE * 0.5f/lmodfreq;
        lflange1 = lflange0 * foffset;

//now is a delay expressed in number of samples.  Number here
//will be fractional, but will use linear interpolation inside the loop to make a decent guess at
//the numbers between samples.

        rx0 = (rflange0 - oldrflange0) * period_const;  //amount to add each time around the loop.  Less processing of linear LFO interp inside the loop.
        rx1 =  (rflange1 - oldrflange1) * period_const;
        lx0 = (lflange0 - oldlflange0) * period_const;
        lx1  = (lflange1 - oldlflange1) * period_const;
// Now there is a fractional amount to add

        drA = oldrflange0;
        drB = oldrflange1;
        dlA = oldlflange0;
        dlB = oldlflange1;
        // dr, dl variables are the LFO inside the loop.

        oldrflange0 = rflange0;
        oldrflange1 = rflange1;
        oldlflange0 = lflange0;
        oldlflange1 = lflange1;
        //lfo ready...


        for (i = 0; i < period; i++) {

            //Delay line utility
            ldl = ldelay[kl];
            rdl = rdelay[kr];
            l = ldl * flrcross + rdl * frlcross;
            r = rdl * flrcross + ldl * frlcross;
            ldl = l;
            rdl = r;
            ldl = smpsl[i] * lpan - ldl * ffb;
            rdl = smpsr[i] * rpan - rdl * ffb;


            //LowPass Filter
            ldelay[kl] = ldl = ldl * (1.0f - fhidamp) + oldl * fhidamp;
            rdelay[kr] = rdl = rdl * (1.0f - fhidamp) + oldr * fhidamp;
            oldl = ldl + DENORMAL_GUARD;
            oldr = rdl + DENORMAL_GUARD;

            if(Pzero) {
                //Offset zero reference delay
                zdl = zldelay[zl];
                zdr = zrdelay[zr];
                zldelay[zl] = smpsl[i];
                zrdelay[zr] = smpsr[i];
                if (--zl < 0)   //Cycle delay buffer in reverse so delay time can be indexed directly with addition
                    zl =  zcenter;
                if (--zr < 0)
                    zr =  zcenter;
            }

            //End delay line management, start flanging:

            //Right Channel, delay A
            rdif0 = drA - floor(drA);
            tmp0 = (kr + (int) floor(drA)) %  maxx_delay;
            tmp1 = tmp0 + 1;
            if (tmp1 >= maxx_delay) tmp1 =  0;
            //rsA = rdelay[tmp0] + rdif0 * (rdelay[tmp1] - rdelay[tmp0] );	//here is the first right channel delay
            rsA = rdelay[tmp1] + rdif0 * (rdelay[tmp0] - rsA );	//All-pass interpolator

            //Right Channel, delay B
            rdif1 = drB - floor(drB);
            tmp0 = (kr + (int) floor(drB)) %  maxx_delay;
            tmp1 = tmp0 + 1;
            if (tmp1 >= maxx_delay) tmp1 =  0;
            //rsB = rdelay[tmp0] + rdif1 * (rdelay[tmp1] - rdelay[tmp0]);	//here is the second right channel delay
            rsB = rdelay[tmp1] + rdif1 * (rdelay[tmp0] - rsB );

            //Left Channel, delay A
            ldif0 = dlA - floor(dlA);
            tmp0 = (kl + (int) floor(dlA)) %  maxx_delay;
            tmp1 = tmp0 + 1;
            if (tmp1 >= maxx_delay) tmp1 =  0;
            //lsA = ldelay[tmp0] + ldif0 * (ldelay[tmp1] - ldelay[tmp0]);	//here is the first left channel delay
            lsA = ldelay[tmp1] + ldif0 * (ldelay[tmp0] - lsA );

            //Left Channel, delay B
            ldif1 = dlB - floor(dlB);
            tmp0 = (kl + (int) floor(dlB)) %  maxx_delay;
            tmp1 = tmp0 + 1;
            if (tmp1 >= maxx_delay) tmp1 =  0;
            //lsB = ldelay[tmp0] + ldif1 * (ldelay[tmp1] - ldelay[tmp0]);	//here is the second left channel delay
            lsB = ldelay[tmp1] + ldif1 * (ldelay[tmp0] - lsB );
            //End flanging, next process outputs

            if(Pzero) {
                efxoutl[i]= dry * smpsl[i] +  fsubtract * wet * (fsubtract * (lsA + lsB)  + zdl);    // Make final FX out mix
                efxoutr[i]= dry * smpsr[i] +  fsubtract * wet * (fsubtract * (rsA + rsB)  + zdr);
            } else {
                efxoutl[i]= dry * smpsl[i] +  wet * fsubtract * (lsA + lsB);    // Make final FX out mix
                efxoutr[i]= dry * smpsr[i] +  wet * fsubtract * (rsA + rsB);
            }




            if (--kl < 0)   //Cycle delay buffer in reverse so delay time can be indexed directly with addition
                kl =  maxx_delay;
            if (--kr < 0)
                kr =  maxx_delay;



// Increment LFO
            drA += rx0;
            drB += rx1;
            dlA += lx0;
            dlB += lx1;

        };  //end for loop

    }  //end intense if statement


};


/*
 * Parameter control
 */

void
Dflange::changepar (int npar, int value)
{
    switch (npar) {
    case 0:
        Pwetdry = value;
        dry = (float) (Pwetdry+64) /128.0f;
        wet = 1.0f - dry;

        if(Psubtract) {
            ldelayline0->set_mix(-dry);
            rdelayline0->set_mix(-dry);
            ldelayline1->set_mix(-dry);
            rdelayline1->set_mix(-dry);
        } else {
            ldelayline0->set_mix(dry);
            rdelayline0->set_mix(dry);
            ldelayline1->set_mix(dry);
            rdelayline1->set_mix(dry);
        }

        break;
    case 1:
        Ppanning = value;
        if (value < 0) {
            rpan = 1.0f + (float) Ppanning/64.0;
            lpan = 1.0f;
        } else {
            lpan = 1.0f - (float) Ppanning/64.0;
            rpan = 1.0f;
        };
        break;
    case 2:
        Plrcross = value;
        flrcross = (float) Plrcross/127.0;
        frlcross = 1.0f - flrcross;	//keep this out of the DSP loop
        break;
    case 3:
        Pdepth = value;
        fdepth =  (float) Pdepth;
        zcenter = (int) fSAMPLE_RATE/floor(0.5f * (fdepth + fwidth));
        logmax = logf( (fdepth + fwidth)/fdepth )/LOG_2;
        break;
    case 4:
        Pwidth = value;
        fwidth = (float) Pwidth;
        zcenter = (int) fSAMPLE_RATE/floor(0.5f * (fdepth + fwidth));
        logmax = logf( (fdepth + fwidth)/fdepth )/LOG_2;
        break;
    case 5:
        Poffset = value;
        foffset = 0.5f + (float) Poffset/255.0;
        break;
    case 6:
        Pfb = value;
        ffb = (float) Pfb/64.5f;
        break;
    case 7:
        Phidamp = value;
        fhidamp = f_exp(-D_PI * (float) Phidamp/fSAMPLE_RATE);
        break;
    case 8:
        Psubtract = value;
        fsubtract = 0.5f;
        if(Psubtract) {
            fsubtract = -0.5f;  //In loop a mult by 0.5f is necessary, so this kills 2 birds with 1...
            ldelayline0->set_mix(-dry);
            rdelayline0->set_mix(-dry);
            ldelayline1->set_mix(-dry);
            rdelayline1->set_mix(-dry);
        }
        break;
    case 9:
        Pzero = value;
        if (Pzero) fzero = 1.0f;
        break;
    case 10:
        lfo->Pfreq = value;
        lfo->updateparams (PERIOD);
        break;
    case 11:
        lfo->Pstereo = value;
        lfo->updateparams (PERIOD);
        break;
    case 12:
        lfo->PLFOtype = value;
        lfo->updateparams (PERIOD);
        break;
    case 13:
        lfo->Prandomness = value;
        lfo->updateparams (PERIOD);
        break;
    case 14:
        Pintense = value;
        break;
    };
};

int
Dflange::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pwetdry);
        break;
    case 1:
        return (Ppanning);
        break;
    case 2:
        return (Plrcross);
        break;
    case 3:
        return (Pdepth);
        break;
    case 4:
        return (Pwidth);
        break;
    case 5:
        return (Poffset);
        break;
    case 6:
        return (Pfb);
        break;
    case 7:
        return (Phidamp);
        break;
    case 8:
        return (Psubtract);
        break;
    case 9:
        return (Pzero);
        break;
    case 10:
        return (lfo->Pfreq);
        break;
    case 11:
        return (lfo->Pstereo);
        break;
    case 12:
        return (lfo->PLFOtype);
        break;
    case 13:
        return (lfo->Prandomness);
        break;
    case 14:
        return Pintense;
        break;
    };
    return (0);			//in case of bogus parameter number
};


void
Dflange::setpreset (int npreset)
{
    const int PRESET_SIZE = 15;
    const int NUM_PRESETS = 9;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Preset 1
        {-32, 0, 0, 110, 800, 10, -27, 16000, 1, 0, 24, 64, 1, 10, 0},
        //Flange-Wha
        {0, 0, 64, 500, 3000, 50, -40, 8000, 1, 0, 196, 96, 0, 0, 0},
        //FbFlange
        {0, 0, 64, 68, 75, 50, -50, 8000, 0, 1, 23, 96, 5, 0, 0},
        //SoftFlange
        {-32, 0, 64, 60, 10, 100, 20, 16000, 0, 0, 16, 90, 4, 0, 0},
        //Flanger
        {-32, 0, 64, 170, 1200, 50, 0, 16000, 1, 0, 68, 127, 0, 0, 0},
        //Chorus 1
        {-15, 0, 0, 42, 12, 50, -10, 1500, 0, 0, 120, 0, 0, 20, 0},
        //Chorus 2
        {-40, 0, 0, 35, 9, 67, 12, 4700, 1, 1, 160, 75, 0, 60, 0},
        //Preset 8
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //Preset 9
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    if(npreset>NUM_PRESETS-1) {
        Fpre->ReadPreset(20,npreset-NUM_PRESETS+1, pdata);
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, pdata[n]);
    } else {

        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, presets[npreset][n]);
    }
    Ppreset = npreset;
};
