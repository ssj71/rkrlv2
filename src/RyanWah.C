/*
  ZynAddSubFX - a software synthesizer

  RyanWah.C - "WahWah" effect and others
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  Modified for rakarrack by Ryan Billing

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

#include <math.h>
#include "RyanWah.h"
#include "AnalogFilter.h"
#include <stdio.h>

RyanWah::RyanWah (float * efxoutl_, float * efxoutr_, double sample_rate, uint32_t intermediate_bufsize)
{
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;

    fSAMPLE_RATE = sample_rate;

    Ppreset = 0;

    filterl = NULL;
    filterr = NULL;

    base = 7.0f;		//sets curve of modulation to frequency relationship
    ibase = 1.0f/base;

    Pampsns = 0;
    Pampsnsinv= 0;
    Pampsmooth= 0;
    Pamode = 0;
    maxfreq = 5000.0f;
    minfreq = 40.0f;
    frequency = 40.0f;
    q = 10.0f;
    Pqm = 1;  //Set backward compatibility mode by default.
    hpmix = 0.0f;
    lpmix = 0.5f;
    bpmix = 2.0f;
    Ppreset = 0;
    wahsmooth = 1.0f - expf(-1.0f/(0.02f*sample_rate));  //0.02 seconds

    lfo = new EffectLFO(sample_rate);
    PERIOD = 256; //best guess until we know better

    Fstages = 1;
    Ftype = 1;
    interpbuf = new float[intermediate_bufsize];
    filterl = new RBFilter (0, 80.0f, 70.0f, 1, sample_rate,interpbuf);
    filterr = new RBFilter (0, 80.0f, 70.0f, 1, sample_rate,interpbuf);
    
    sidechain_filter = new AnalogFilter (1, 630.0, 1.0, 1, sample_rate,interpbuf);
    setpreset (Ppreset);

    cleanup ();
};

RyanWah::~RyanWah ()
{
	delete lfo;
	delete filterl;
	delete filterr;
	delete sidechain_filter;
	delete[] interpbuf;
};


/*
 * Apply the effect
 */
void
RyanWah::out (float * smpsl, float * smpsr, uint32_t period)
{
    unsigned int i;
    float lmod, rmod;
    float lfol, lfor;
    float rms = 0.0f;

    lfo->effectlfoout (&lfol, &lfor);
    if (Pamode) {
        lfol *= depth;
        lfor *= depth;
    } else {
        lfol *= depth * 5.0f;
        lfor *= depth * 5.0f;
    }

    for (i = 0; i < period; i++) {
        efxoutl[i] = smpsl[i];
        efxoutr[i] = smpsr[i];

        float x = (fabsf ( sidechain_filter->filterout_s(smpsl[i] + smpsr[i]))) * 0.5f;
        ms1 = ms1 * ampsmooth + x * (1.0f - ampsmooth) + 1e-10f;

        //oldfbias -= 0.001 * oldfbias2;
        oldfbias = oldfbias * (1.0f - wahsmooth) + fbias * wahsmooth + 1e-10f;  //smooth MIDI control
        oldfbias1 = oldfbias1 * (1.0f - wahsmooth) + oldfbias * wahsmooth + 1e-10f;
        oldfbias2 = oldfbias2 * (1.0f - wahsmooth) + oldfbias1 * wahsmooth + 1e-10f;

        if (Pamode) {
            rms = ms1 * ampsns + oldfbias2;
            if (rms<0.0f) rms = 0.0f;
            lmod = (minfreq + lfol + rms)*maxfreq;
            rmod = (minfreq + lfor + rms)*maxfreq;
            if(variq) q = f_pow2((2.0f*(1.0f-rms)+1.0f));
            filterl->setq(q);
            filterr->setq(q);
            filterl->directmod(rmod);
            filterr->directmod(lmod);
            efxoutl[i] = filterl->filterout_s (smpsl[i]);
            efxoutr[i] = filterr->filterout_s (smpsr[i]);

        }
    };

    if (!Pamode) {
        rms = ms1 * ampsns + oldfbias2;

        if(rms>0.0f) { //apply some smooth limiting
            rms = 1.0f - 1.0f/(rms*rms + 1.0f);
        } else {
            rms = -1.0f + 1.0f/(rms*rms + 1.0f);
        }

        if(variq) q = f_pow2((2.0f*(1.0f-rms)+1.0f));

        lmod =(lfol + rms);
        rmod = (lfor + rms);
        if(lmod>1.0f) lmod = 1.0f;
        if(lmod<0.0f) lmod = 0.0f;
        if(rmod>1.0f) rmod = 1.0f;
        if(rmod<0.0f) rmod = 0.0f;

        //rms*=rms;
        float frl = minfreq + maxfreq*(powf(base, lmod) - 1.0f)*ibase;
        float frr = minfreq + maxfreq*(powf(base, rmod) - 1.0f)*ibase;

        centfreq = frl; //testing variable

        filterl->setfreq_and_q (frl, q);
        filterr->setfreq_and_q (frr, q);

        filterl->filterout (efxoutl, period);
        filterr->filterout (efxoutr, period);
    }

};

/*
 * Cleanup the effect
 */
void
RyanWah::cleanup ()
{
    reinitfilter ();
    ms1 = 0.0;
    oldfbias = oldfbias1 = oldfbias2 = 0.0f;
    filterl->cleanup();
    filterr->cleanup();
};


/*
 * Parameter control
 */

void
RyanWah::setwidth (int Pwidth)
{
    this->Pwidth = Pwidth;
    depth = powf (((float)Pwidth / 127.0f), 2.0f);
};


void
RyanWah::setvolume (int Pvolume)
{
    this->Pvolume = Pvolume;
    outvolume = (float)Pvolume / 127.0f;
};

void
RyanWah::setampsns (int Pp)
{
    Pampsns = Pp;
    if(Pampsns>0) {
        ampsns = expf(0.083f*(float)Pampsns);
    } else {
        ampsns = - expf(-0.083f*(float)Pampsns);
    }
    fbias  =  ((float)Pampsnsinv )/ 127.0f;

    ampsmooth = f_exp(-1.0f/((((float) Pampsmooth)/127.0f + 0.01f)*fSAMPLE_RATE)); //goes up to 1 second

};

void
RyanWah::reinitfilter ()
{
    //setmix (int mix, float lpmix, float bpmix, float hpmix)
    filterl->setmix(1, lpmix, bpmix, hpmix);
    filterr->setmix(1, lpmix, bpmix, hpmix);

};

void
RyanWah::setpreset (int npreset)
{
    const int PRESET_SIZE = 19;
    const int NUM_PRESETS = 6;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Wah Pedal
        {16, 10, 60, 0, 0, 64, 0, 0, 10, 7, -16, 40, -3, 1, 2000, 450, 1, 1, 0},
        //Mutron
        {0, 15, 138, 0, 0, 64, 0, 50, 0, 30, 32, 0, 5, 1, 2000, 60, 0, 1, 1},
        //Phase Wah
        {0, 50, 60, 0, 0, 64, 30, 10, 10, 30, 32, 0, 10, 2, 2000, 350, 1, 1, 2},
        //Succulent Phaser
        {64, 8, 35, 10, 0, 64, 50, -10, 53, 35, 28, -16, 32, 4, 2600, 300, 1, 1, 3},
        //Quacky
        {16, 10, 60, 0, 0, 64, 0, 40, 10, 32, -16, 40, -3, 1, 2000, 400, 1, 1, 4},
        //Smoothtron
        {0, 15, 138, 0, 0, 64, 0, 15, 0, 20, 32, 0, 5, 1, 2000, 60, 0, 3, 5}

    };

    if(npreset>NUM_PRESETS-1) {
        Fpre->ReadPreset(31,npreset-NUM_PRESETS+1,pdata);
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, pdata[n]);
    } else {

        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, presets[npreset][n]);
    }

    Ppreset = npreset;

    reinitfilter ();
};


void
RyanWah::changepar (int npar, int value)
{
    switch (npar) {
    case 0:
        setvolume (value);
        break;
    case 1:
        Pq = value;
        q = (float) Pq;
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
        setwidth (value);
        break;
    case 7:
        setampsns (value);
        break;
    case 8:
        Pampsnsinv = value;
        setampsns (Pampsns);
        break;
    case 9:
        Pampsmooth = value;
        setampsns (Pampsns);
        break;
    case 10:
        Plp = value;
        lpmix = ((float) Plp)/32.0f;
        reinitfilter ();
        break;
    case 11:
        Pbp = value;
        bpmix = ((float) Pbp)/32.0f;
        reinitfilter ();
        break;
    case 12:
        Php = value;
        hpmix = ((float) Php)/32.0f;
        reinitfilter ();
        break;
    case 13:
        Pstages = (value - 1);
        filterl->setstages(Pstages);
        filterr->setstages(Pstages);
        cleanup();
        break;
    case 14:
        Prange = value;
        if(Pamode) maxfreq = ((float) Prange)/(fSAMPLE_RATE/6.0f);
        else maxfreq = ((float) Prange);
        break;
    case 15:
        Pminfreq = value;
        if (Pamode)  minfreq = ((float) Pminfreq)/(fSAMPLE_RATE/6.0f);
        else minfreq = (float) value;
        break;
    case 16:
        variq = value;
        break;
    case 17:
    	//legacy method of changing Pqm and Pamode, presets use this
        Pmode=value;
        if((Pmode==1) || (Pmode==3)) Pqm = 1;
        else Pqm = 0;
        filterl->setmode(Pqm);
        filterr->setmode(Pqm);

        if((Pmode==2) || (Pmode==3)) Pamode = 1;
        else Pamode = 0;
        if(Pamode) {
            minfreq = ((float) Pminfreq)/(fSAMPLE_RATE/6.0f);
            maxfreq = ((float) Prange)/(fSAMPLE_RATE/6.0f);
        } else {
            minfreq = (float) Pminfreq;
            maxfreq = (float) Prange;
        }

        break;
    case 18:
        Ppreset = value;
        break;
    case 19:
    	Pqm=value;
        filterl->setmode(Pqm);
        filterr->setmode(Pqm);
        break;
    case 20:
    	Pamode=value;
        if(Pamode) {
            minfreq = ((float) Pminfreq)/(fSAMPLE_RATE/6.0f);
            maxfreq = ((float) Prange)/(fSAMPLE_RATE/6.0f);
        } else {
            minfreq = (float) Pminfreq;
            maxfreq = (float) Prange;
        }
        break;
    };
};

int
RyanWah::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
        return (Pq);
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
        return (Pwidth);
        break;
    case 7:
        return (Pampsns);
        break;
    case 8:
        return (Pampsnsinv);
        break;
    case 9:
        return (Pampsmooth);
        break;
    case 10:
        return (Plp);
        break;
    case 11:
        return (Pbp);
        break;
    case 12:
        return (Php);
        break;
    case 13:
        return (Pstages + 1);
        break;
    case 14:
        return (Prange);
        break;
    case 15:
        return (Pminfreq);
        break;
    case 16:
        return (variq);
        break;
    case 17:
        return (Pmode);
        break;
    case 18:
        return (Ppreset);
        break;
    case 19:
    	return (Pqm);
    	break;
    case 20:
    	return (Pamode);
    	break;
    default:
        return (0);
    };

};
