/*
  MBVvol.C - Vary Band Volumen effect

  ZynAddSubFX - a software synthesizer
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
#include <stdlib.h>
#include <math.h>
#include "MBVvol.h"

/*
 * Waveshape (this is called by OscilGen::waveshape and Distorsion::process)
 */



MBVvol::MBVvol (float * efxoutl_, float * efxoutr_, double sample_rate, uint32_t intermediate_bufsize)
{
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;

    lowl = (float *) malloc (sizeof (float) * intermediate_bufsize);
    lowr = (float *) malloc (sizeof (float) * intermediate_bufsize);
    midll = (float *) malloc (sizeof (float) * intermediate_bufsize);
    midlr = (float *) malloc (sizeof (float) * intermediate_bufsize);
    midhl = (float *) malloc (sizeof (float) * intermediate_bufsize);
    midhr = (float *) malloc (sizeof (float) * intermediate_bufsize);
    highl = (float *) malloc (sizeof (float) * intermediate_bufsize);
    highr = (float *) malloc (sizeof (float) * intermediate_bufsize);


    interpbuf = new float[intermediate_bufsize];
    lpf1l = new AnalogFilter (2, 500.0f, .7071f, 0, sample_rate, interpbuf);
    lpf1r = new AnalogFilter (2, 500.0f, .7071f, 0, sample_rate, interpbuf);
    hpf1l = new AnalogFilter (3, 500.0f, .7071f, 0, sample_rate, interpbuf);
    hpf1r = new AnalogFilter (3, 500.0f, .7071f, 0, sample_rate, interpbuf);
    lpf2l = new AnalogFilter (2, 2500.0f, .7071f, 0, sample_rate, interpbuf);
    lpf2r = new AnalogFilter (2, 2500.0f, .7071f, 0, sample_rate, interpbuf);
    hpf2l = new AnalogFilter (3, 2500.0f, .7071f, 0, sample_rate, interpbuf);
    hpf2r = new AnalogFilter (3, 2500.0f, .7071f, 0, sample_rate, interpbuf);
    lpf3l = new AnalogFilter (2, 5000.0f, .7071f, 0, sample_rate, interpbuf);
    lpf3r = new AnalogFilter (2, 5000.0f, .7071f, 0, sample_rate, interpbuf);
    hpf3l = new AnalogFilter (3, 5000.0f, .7071f, 0, sample_rate, interpbuf);
    hpf3r = new AnalogFilter (3, 5000.0f, .7071f, 0, sample_rate, interpbuf);

    lfo1 = new EffectLFO(sample_rate);
    lfo2 = new EffectLFO(sample_rate);
    PERIOD = 256;

    //default values
    Ppreset = 0;
    Pvolume = 50;
    volL=volLr=volML=volMLr=volMH=volMHr=volH=volHr=2.0f;
    one = 1.0f;
    zero = 0.0f;
    setpreset (Ppreset);
    cleanup ();
};

MBVvol::~MBVvol ()
{
	free(lowl);
    free(lowr);
    free(midll);
    free(midlr);
    free(midhl);
    free(midhr);
    free(highl);
    free(highr);
    delete lpf1l;
    delete lpf1r;
    delete hpf1l;
    delete hpf1r;
    delete lpf2l;
    delete lpf2r;
    delete hpf2l;
    delete hpf2r;
    delete lpf3l;
    delete lpf3r;
    delete hpf3l;
    delete hpf3r;
    delete[] interpbuf;
};

/*
 * Cleanup the effect
 */
void
MBVvol::cleanup ()
{
    lpf1l->cleanup ();
    hpf1l->cleanup ();
    lpf1r->cleanup ();
    hpf1r->cleanup ();
    lpf2l->cleanup ();
    hpf2l->cleanup ();
    lpf2r->cleanup ();
    hpf2r->cleanup ();
    lpf3l->cleanup ();
    hpf3l->cleanup ();
    lpf3r->cleanup ();
    hpf3r->cleanup ();

};
/*
 * Effect output
 */
void
MBVvol::out (float * smpsl, float * smpsr, uint32_t period)
{
    unsigned int i;

    memcpy(lowl,smpsl,sizeof(float) * period);
    memcpy(midll,smpsl,sizeof(float) * period);
    memcpy(midhl,smpsl,sizeof(float) * period);
    memcpy(highl,smpsl,sizeof(float) * period);

    lpf1l->filterout(lowl, period);
    hpf1l->filterout(midll, period);
    lpf2l->filterout(midll, period);
    hpf2l->filterout(midhl, period);
    lpf3l->filterout(midhl, period);
    hpf3l->filterout(highl, period);

    memcpy(lowr,smpsr,sizeof(float) * period);
    memcpy(midlr,smpsr,sizeof(float) * period);
    memcpy(midhr,smpsr,sizeof(float) * period);
    memcpy(highr,smpsr,sizeof(float) * period);

    lpf1r->filterout(lowr, period);
    hpf1r->filterout(midlr, period);
    lpf2r->filterout(midlr, period);
    hpf2r->filterout(midhr, period);
    lpf3r->filterout(midhr, period);
    hpf3r->filterout(highr, period);

    lfo1->effectlfoout (&lfo1l, &lfo1r);
    lfo2->effectlfoout (&lfo2l, &lfo2r);

    d1=(lfo1l-v1l)/(float)period;
    d2=(lfo1r-v1r)/(float)period;
    d3=(lfo2l-v2l)/(float)period;
    d4=(lfo2r-v2r)/(float)period;

    for (i = 0; i < period; i++) {

    	updateVols();

        efxoutl[i]=lowl[i]*volL+midll[i]*volML+midhl[i]*volMH+highl[i]*volH;
        efxoutr[i]=lowr[i]*volLr+midlr[i]*volMLr+midhr[i]*volMHr+highr[i]*volHr;
    }

};


/*
 * Parameter control
 */
void
MBVvol::setvolume (int value)
{
    Pvolume = value;
    outvolume = (float)Pvolume / 127.0f;
};


void
MBVvol::updateVols(void)
{
    v1l+=d1;
    v1r+=d2;
    v2l+=d3;
    v2r+=d4;
    
    volL  = *sourceL;
    volLr = *sourceLr;
    volML = *sourceML;
    volMLr= *sourceMLr;
    volMH = *sourceMH;
    volMHr= *sourceMHr;
    volH  = *sourceH;
    volHr = *sourceHr;
    
}

//legacy support
void
MBVvol::setCombi(int value)
{

    switch(value) {
    case 0:
        sourceL = &v1l;
        sourceLr = &v1r;
        sourceML = &v1l;
        sourceMLr = &v1r;
        sourceMH  = &v2l;
        sourceMHr = &v2r;
        sourceH = &v2l;
        sourceHr = &v2r;
        break;
    case 1:
        sourceL = &v1l;
        sourceLr = &v1r;
        sourceML = &v2l;
        sourceMLr = &v2r;
        sourceMH = &v2l;
        sourceMHr = &v2r;
        sourceH = &v1l;
        sourceHr = &v1r;
        break;
    case 2:
        sourceL = &v1l;
        sourceLr = &v1r;
        sourceML = &v2l;
        sourceMLr = &v2r;
        sourceMH = &v1l;
        sourceMHr = &v1r;
        sourceH = &v2l;
        sourceHr = &v2r;
        break;
    case 3:
        sourceL = &one;
        sourceLr = &one;
        sourceML = &v1l;
        sourceMLr = &v1r;
        sourceMH = &v1l;
        sourceMHr = &v1r;
        sourceH = &one;
        sourceHr = &one;
        break;
    case 4:
        sourceL = &one;
        sourceLr = &one;
        sourceML = &v1l;
        sourceMLr = &v1r;
        sourceMH = &v2l;
        sourceMHr = &v2r;
        sourceH = &one;
        sourceHr = &one;
        break;
    case 5:
        sourceL = &zero;
        sourceLr = &zero;
        sourceML = &v1l;
        sourceMLr = &v1r;
        sourceMH = &v1l;
        sourceMHr = &v1r;
        sourceH = &zero;
        sourceHr = &zero;
        break;
    case 6:
        sourceL = &zero;
        sourceLr = &zero;
        sourceML = &v1l;
        sourceMLr = &v1r;
        sourceMH = &v2l;
        sourceMHr = &v2r;
        sourceH = &zero;
        sourceHr = &zero;
        break;
    case 7:
        sourceL = &v1l;
        sourceLr = &v1r;
        sourceML = &one;
        sourceMLr = &one;
        sourceMH = &one;
        sourceMHr = &one;
        sourceH = &v1l;
        sourceHr = &v1r;
        break;
    case 8:
        sourceL = &v1l;
        sourceLr = &v1r;
        sourceML = &one;
        sourceMLr = &one;
        sourceMH = &one;
        sourceMHr = &one;
        sourceH = &v2l;
        sourceHr = &v2r;
        break;
    case 9:
        sourceL = &v1l;
        sourceLr = &v1r;
        sourceML = &zero;
        sourceMLr = &zero;
        sourceMH = &zero;
        sourceMHr = &zero;
        sourceH = &v1l;
        sourceHr = &v1r;
        break;
    case 10:
        sourceL = &v1l;
        sourceLr = &v1r;
        sourceML = &zero;
        sourceMLr = &zero;
        sourceMH = &zero;
        sourceMHr = &zero;
        sourceH = &v2l;
        sourceHr = &v2r;
        break;

    }


}

void
MBVvol::setCross1 (int value)
{
    Cross1 = value;
    lpf1l->setfreq ((float)value);
    lpf1r->setfreq ((float)value);
    hpf1l->setfreq ((float)value);
    hpf1r->setfreq ((float)value);

};

void
MBVvol::setCross2 (int value)
{
    Cross2 = value;
    hpf2l->setfreq ((float)value);
    hpf2r->setfreq ((float)value);
    lpf2l->setfreq ((float)value);
    lpf2r->setfreq ((float)value);

};

void
MBVvol::setCross3 (int value)
{
    Cross3 = value;
    hpf3l->setfreq ((float)value);
    hpf3r->setfreq ((float)value);
    lpf3l->setfreq ((float)value);
    lpf3r->setfreq ((float)value);

};

void
MBVvol::setSource (float* ptr, float* ptrr, int val)
{
	switch(val){
	case 0:
		ptr = &v1l;
		ptrr = &v1r;
		break;
	case 1:
		ptr = &v2l;
		ptrr = &v2r;
		break;
	case 2:
		ptr = &one;
		ptrr = &one;
		break;
	case 3:
		ptr = &zero;
		ptrr = &zero;
		break;
	default:
		return; //no change
	}
}

void
MBVvol::setpreset (int npreset)
{
    const int PRESET_SIZE = 11;
    const int NUM_PRESETS = 3;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Vary1
        {0, 40, 0, 64, 80, 0, 0, 500, 2500, 5000, 0},
        //Vary2
        {0, 80, 0, 64, 40, 0, 0, 120, 600, 2300, 1},
        //Vary3
        {0, 120, 0, 64, 40, 0, 0, 800, 2300, 5200, 2}
    };

    if(npreset>NUM_PRESETS-1) {
        Fpre->ReadPreset(28,npreset-NUM_PRESETS+1, pdata);
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
MBVvol::changepar (int npar, int value)
{
    switch (npar) {
    case 0:
        setvolume (value);
        break;
    case 1:
        lfo1->Pfreq = value;
        lfo1->updateparams (PERIOD);
        break;
    case 2:
        lfo1->PLFOtype = value;
        lfo1->updateparams (PERIOD);
        break;
    case 3:
        lfo1->Pstereo = value;
        lfo1->updateparams (PERIOD);
        break;
    case 4:
        lfo2->Pfreq = value;
        lfo2->updateparams (PERIOD);
        break;
    case 5:
        lfo2->PLFOtype = value;
        lfo2->updateparams (PERIOD);
        break;
    case 6:
        lfo2->Pstereo = value;
        lfo2->updateparams (PERIOD);
        break;
    case 7:
        setCross1 (value);
        break;
    case 8:
        setCross2 (value);
        break;
    case 9:
        setCross3(value);
        break;
    case 10:
        Pcombi=value;
        setCombi(value);
        break;
    case 11:
    	PsL=value;
    	setSource(sourceL, sourceLr, value);
    	break;
    case 12:
    	PsML=value;
    	setSource(sourceML, sourceMLr, value);
    	break;
    case 13:
    	PsMH=value;
    	setSource(sourceMH, sourceMHr, value);
    	break;
    case 14:
    	PsH=value;
    	setSource(sourceH, sourceHr, value);
    	break;
    };
};

int
MBVvol::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
        return (lfo1->Pfreq);
        break;
    case 2:
        return (lfo1->PLFOtype);
        break;
    case 3:
        return (lfo1->Pstereo);
        break;
    case 4:
        return (lfo2->Pfreq);
        break;
    case 5:
        return (lfo2->PLFOtype);
        break;
    case 6:
        return (lfo2->Pstereo);
        break;
    case 7:
        return (Cross1);
        break;
    case 8:
        return (Cross2);
        break;
    case 9:
        return (Cross3);
        break;
    case 10:
        return (Pcombi);
        break;
    case 11:
    	return (PsL);
    	break;
    case 12:
    	return (PsML);
    	break;
    case 13:
    	return (PsMH);
    	break;
    case 14:
    	return (PsH);
    	break;

    };
    return (0);			//in case of bogus parameter number
};

