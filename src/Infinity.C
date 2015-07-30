/*
  ZynAddSubFX - a software synthesizer

  Rakarrack Guitar Effects Processor

  InfiniT : Infinitely rising & falling filter effect
  Author:  Ryan Billing (aka Transmogrifox)
  Copyright (C) 2010

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
#include "Infinity.h"
#include <stdio.h>

Infinity::Infinity (float * efxoutl_, float * efxoutr_, double sample_rate, uint32_t intermediate_bufsize)
{
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;
    fSAMPLE_RATE = sample_rate;

    int i;
    interpbuf = new float[intermediate_bufsize];
    for (i = 0; i<NUM_INF_BANDS; i++) {
        filterl[i] = new RBFilter (0, 80.0f, 70.0f, 1.0f, sample_rate, interpbuf);
        filterr[i] = new RBFilter (0, 80.0f, 70.0f, 1.0f, sample_rate, interpbuf);
        rbandstate[i].level = 1.0f;
        rbandstate[i].vol = 1.0f;
        lphaser[i].gain = 0.5f;
        rphaser[i].gain = 0.5f;
        for (int j = 0; j<MAX_PHASER_STAGES; j++) {
            lphaser[i].yn1[j] = 0.0f;
            rphaser[i].yn1[j] = 0.0f;
            lphaser[i].xn1[j] = 0.0f;
            rphaser[i].xn1[j] = 0.0f;
        }

        Pb[i] = 1;
    }
    Ppreset = 2;
    setpreset (Ppreset);
    Pvolume = 64;
    outvolume = 0.5;
    Pq = 30;
    Pstartfreq = 5;
    Pendfreq = 80;
    Prate = 2;
    Psubdiv = 32;
    Pstdf = 0;
    volmaster = 0.25;
    Preverse = 0;
    Pautopan = 0;
    autopan = 0.0f;
    Pstages = 0;
    phaserfb = 0.0f;
    dsin = 0.0f;
    tflag = 0;
    ratescale = 1.0f;

    float dt = 1.0f/fSAMPLE_RATE;
    alpha = dt/(0.5f + dt);          //200ms time constant on parameter change -- quick but not jerky
    beta = 1.0f - alpha;

    adjustfreqs();
    reinitfilter();

};

Infinity::~Infinity ()
{
	delete interpbuf;
    for (int i = 0; i<NUM_INF_BANDS; i++) {
    	delete filterl[i];
    	delete filterr[i];
    }
};

float inline
Infinity::phaser(phasevars *pstruct, float fxn, int j)
{
    int k;
    float xn = fxn + DENORMAL_GUARD;

    for (k = 0; k < Pstages; k++) {
        pstruct[j].yn1[k] = pstruct[j].xn1[k] - pstruct[j].gain * (xn + pstruct[j].yn1[k]);
        //pstruct[j].yn1[k] += DENORMAL_GUARD;
        pstruct[j].xn1[k] = fxn;
        xn = pstruct[j].yn1[k];
    };
    pstruct[j].yn1[0] -= phaserfb*fxn;

    return fxn;

}

void inline
Infinity::oscillator()
{
    float rmodulate, lmodulate, ratemod;

    //master oscillator
    msin += mconst*mcos;
    mcos -= msin*mconst;

//introduce doppler effect
    if(Pstages<9) {
//smooth parameters for tempo change
        rampconst = alpha*rampconst + beta*crampconst;
        irampconst = 1.0f/rampconst;
        fconst = alpha*fconst + beta*cfconst;
    } else {
        dsin = autopan*ratescale*msin;
        ratemod = (1.0f + dsin/fSAMPLE_RATE);
//smooth parameters for tempo change
        rampconst = alpha*rampconst + beta*crampconst*ratemod;
        irampconst = 1.0f/rampconst;
        fconst = alpha*fconst + beta*cfconst*ratemod;
    }


    for (int i=0; i<NUM_INF_BANDS; i++)  {
//right
        rbandstate[i].sinp += fconst*rbandstate[i].cosp;
        rbandstate[i].cosp -= rbandstate[i].sinp*fconst;
        rbandstate[i].lfo = (1.0f + rbandstate[i].sinp);  //lfo modulates filter band volume
        rbandstate[i].ramp *= rampconst;  //ramp modulates filter band frequency cutoff
        if (rbandstate[i].ramp > maxlevel)  {
            rbandstate[i].ramp = minlevel;
            rbandstate[i].sinp = -1.0f;
            rbandstate[i].cosp = 0.0f;  //phase drift catch-all
//printf("i: %d sin: %f lfo: %f ramp: %f\n",i,rbandstate[i].sinp, rbandstate[i].lfo, rbandstate[i].ramp);
        }
        if (rbandstate[i].ramp < minlevel) {
            rbandstate[i].ramp = maxlevel;  //if it is going in reverse (rampconst < 0)
            rbandstate[i].sinp = -1.0f;
            rbandstate[i].cosp = 0.0f;   //phase drift catch-all
        }
        rbandstate[i].vol = rbandstate[i].level*rbandstate[i].lfo;

//left
        lbandstate[i].sinp += fconst*lbandstate[i].cosp;
        lbandstate[i].cosp -= lbandstate[i].sinp*fconst;
        lbandstate[i].lfo = (1.0f + lbandstate[i].sinp);  //lfo modulates filter band volume
        if (Preverse) lbandstate[i].ramp *= irampconst;  //left reversed from right
        else lbandstate[i].ramp *= rampconst;   //normal mode
        if (lbandstate[i].ramp > maxlevel)  {
            lbandstate[i].ramp = minlevel;
            lbandstate[i].sinp = -1.0f;
            lbandstate[i].cosp = 0.0f;  //phase drift catch-all
//printf("i: %d sin: %f lfo: %f ramp: %f\n",i,lbandstate[i].sinp, lbandstate[i].lfo, lbandstate[i].ramp);
        }
        if (lbandstate[i].ramp < minlevel) {
            lbandstate[i].ramp = maxlevel;  //if it is going in reverse (rampconst < 0)
            lbandstate[i].sinp = -1.0f;
            lbandstate[i].cosp = 0.0f;  //phase drift catch-all
        }
        lbandstate[i].vol = lbandstate[i].level*lbandstate[i].lfo;

        //lmodulate = linconst*f_pow2(logconst*lbandstate[i].ramp);
        //rmodulate = linconst*f_pow2(logconst*rbandstate[i].ramp);
        lmodulate = 1.0f - 0.25f*lbandstate[i].ramp;
        rmodulate = 1.0f - 0.25f*rbandstate[i].ramp;

        filterl[i]->directmod(lbandstate[i].ramp);
        filterr[i]->directmod(rbandstate[i].ramp);

        lphaser[i].gain = lmodulate;
        rphaser[i].gain = rmodulate;
    }

}

/*
 * Apply the effect
 */
void
Infinity::out (float * smpsl, float * smpsr, uint32_t period)
{
    unsigned int i, j;
    float tmpr, tmpl;

    for (i = 0; i<period; i++)  {
        //modulate
        oscillator();
        tmpr = tmpl = 0.0f;
        //run filter



        if(Pstages) {
            for (j=0; j<NUM_INF_BANDS; j++)  {
                tmpl+=phaser(lphaser, filterl[j]->filterout_s(lbandstate[j].vol*smpsl[i]), j );
                tmpr+=phaser(rphaser, filterr[j]->filterout_s(rbandstate[j].vol*smpsr[i]), j );
            }
        } else {
            for (j=0; j<NUM_INF_BANDS; j++)  {
                tmpl+=filterl[j]->filterout_s(lbandstate[j].vol*smpsl[i]);
                tmpr+=filterr[j]->filterout_s(rbandstate[j].vol*smpsr[i]);
            }
        }


        efxoutl[i] = (1.0f + autopan*mcos)*volmaster*tmpl;
        efxoutr[i] = (1.0f - autopan*mcos)*volmaster*tmpr;



    }

};

/*
 * Cleanup the effect
 */
void
Infinity::cleanup ()
{
    reinitfilter ();
    for ( int i = 0; i<NUM_INF_BANDS; i++) {
        filterl[i]->cleanup();
        filterr[i]->cleanup();
        lphaser[i].gain = 0.5f;
        rphaser[i].gain = 0.5f;
        for (int j = 0; j<MAX_PHASER_STAGES; j++) {
            lphaser[i].yn1[j] = 0.0f;
            rphaser[i].yn1[j] = 0.0f;
            lphaser[i].xn1[j] = 0.0f;
            rphaser[i].xn1[j] = 0.0f;
        }
    }


};


/*
 * Parameter control
 */

void
Infinity::setvolume (int Pvolume)
{
    this->Pvolume = Pvolume;
    outvolume = (float)Pvolume / 127.0f;
};

void
Infinity::setq ()
{
    float fq = (float) Pq;
    if(Pq<0) {
        qq = powf(2.0f, fq/500.0f);  //q ranges down to 0.5
        volmaster = 1.0f;
    } else {
        qq = powf(2.0f, fq/125.0f);  //q can go up to 256
        volmaster = (1.0f-fq/1500.0f)/sqrt(qq);
    }

    for (int i=0; i<NUM_INF_BANDS; i++)  {
        filterl[i]->setq(qq);
        filterr[i]->setq(qq);
    }
}
void
Infinity::reinitfilter ()
{
    float fbandnum = (float) (NUM_INF_BANDS);
    float halfpi = -M_PI/2.0f;  //offset so rbandstate[0].sinp = -1.0 when rbandstate[0].ramp = 0;
    float stateconst = 0;
    float idx = 0;

    for (int i=0; i<NUM_INF_BANDS; i++)  {  //get them started on their respective phases
//right
        idx = (float) i;
        rbandstate[i].sinp = sinf(halfpi + D_PI*idx/fbandnum);
        rbandstate[i].cosp = cosf(halfpi + D_PI*idx/fbandnum);
        rbandstate[i].ramp = linconst*powf(2.0f,logconst*idx/fbandnum);
        rbandstate[i].lfo = 0.5f*(1.0f + rbandstate[i].sinp);  //lfo modulates filter band volume
//left
        stateconst = fmod((stdiff + idx), fbandnum);
        lbandstate[i].sinp = sinf(halfpi + D_PI*stateconst/fbandnum);
        lbandstate[i].cosp = cosf(halfpi + D_PI*stateconst/fbandnum);
        lbandstate[i].ramp = linconst*powf(2.0f,logconst*stateconst/fbandnum);
        lbandstate[i].lfo = 0.5f*(1.0f + rbandstate[i].sinp);  //lfo modulates filter band volume
//printf("i: %d sin: %f lfo: %f ramp: %f max: %f min: %f\n",i,rbandstate[i].sinp, rbandstate[i].lfo, rbandstate[i].ramp, maxlevel, minlevel);

        filterl[i]->setmix(0, 80.0f, 70.0f, 1.0f);
        filterr[i]->setmix(0, 80.0f, 70.0f, 1.0f);
        filterl[i]->setmode(1);
        filterr[i]->setmode(1);
        filterl[i]->settype(2);
        filterr[i]->settype(2);  //bandpass
        filterl[i]->setq(qq);
        filterr[i]->setq(qq);
        filterl[i]->directmod(lbandstate[i].ramp);
        filterr[i]->directmod(rbandstate[i].ramp);

    }
    msin = 0.0f;
    mcos = 1.0f;
};

void
Infinity::adjustfreqs()
{

    float frate;
    float fs = fSAMPLE_RATE;

    fstart = 20.0f + 6000.0f*((float) Pstartfreq/127.0f);
    fend =  20.0f + 6000.0f*((float) Pendfreq/127.0f);
    if(Psubdiv>0) frate = ((float) ( 1+ Prate ))/(((float) Psubdiv)*60.0f);    //beats/second
    else frate = ((float) (1 - Psubdiv))*((float) Prate)/60.0f;

    if (fstart < fend) {
        frmin = fstart;
        frmax = fend;
        crampconst = 1.0f + frate*logf(frmax/frmin)/fs;
    } else {
        frmax = fstart;
        frmin = fend;
        crampconst = 1.0f/(1.0f + frate*logf(frmax/frmin)/fs);
    }

    cirampconst = 1.0f/crampconst;
    logconst = logf(frmax/frmin)/logf(2.0f);
    linconst = D_PI*frmin/fs;  //these lines are for using powf() in the initialization

    minlevel = D_PI*frmin/fs;
    maxlevel = minlevel*frmax/frmin;
    //printf("min %f max %f rampconst %f irampconst %f\n", minlevel, maxlevel,rampconst, irampconst);
    //fconst =  2.0f * sinf(PI*frate / fs);  //this is a more precise interpretation
    cfconst =  D_PI*frate / fs;  //this seems to work well at low frequencies
    mconst = D_PI*((float) Prate)/(fs*60.0f*4.0f);

}

void
Infinity::setpreset (int npreset)
{
    const int PRESET_SIZE = 18;
    const int NUM_PRESETS = 10;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Basic
        {64, 64, 64, 64, 64, 64, 64, 64, 64, 700, 20, 80, 60, 0, 1, 0, 0, 1 },
        //Rising Comb
        {64, 64, -64, 64, -64, 64, -64, 64, -64, 600, 0, 127, 32, 0, 16, 0, 0, 1 },
        //Falling Comb
        {64, 64, -64, 64, -64, 64, -64, 64, -64, 600, 127, 0, 32, 0, 16, 0, 0, 1 },
        //Laser
        {0, 64, -64, 64, -64, 64, -64, 64, -64, 600, 127, 2, 70, 0, 1, 0, 0, 1 },
        //Doppler
        {0,64,-64,64,-64,64,-64,64,-64,-564,0,127,150,-31,2,99,0,10},
        //Freq Shifter
        {0,64,-64,64,-64,64,-64,64,-64,-564,0,127,60,-48,-1,0,0,10},
        //Dizzy Sailor
        {0,64,-64,64,-64,64,-64,64,-64,-564,0,127,60,-48,-1,110,0,10},
        //Stereo Phaser
        {42,64,0,64,0,64,0,64,0,91,0,127,60,32,16,92,1,6},
        //Corkscrew
        {64,64,0,-64,0,64,0,-64,0,120,0,127,120,-16,15,67,1,4},
        //FreqeeVox
        {0,64,-64,64,-64,64,-64,64,-64,-164,0,127,556,-16,-3,0,0,8}

    };

    if(npreset>NUM_PRESETS-1) {
        Fpre->ReadPreset(46,npreset-NUM_PRESETS+1,pdata);
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
Infinity::changepar (int npar, int value)
{

    switch (npar) {
    case 0:
        setvolume (value);
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        Pb[npar - 1] = value;
        rbandstate[npar - 1].level = (float) value/64.0f;
        lbandstate[npar - 1].level = (float) value/64.0f;
        break;
    case 9:
        Pq = value;
        setq();
        break;
    case 10:
        Pstartfreq = value;
        adjustfreqs();
        reinitfilter ();
        break;
    case 11:
        Pendfreq = value;
        adjustfreqs();
        reinitfilter ();
        break;
    case 12:
        Prate = value;
        adjustfreqs();
        break;
    case 13:
        Pstdf = value;
        stdiff = ((float) value)/127.0f;
        reinitfilter ();
        break;
    case 14:
        Psubdiv = value;
        if(value!=0) ratescale =  10.0f/((float) abs(value));
        else ratescale = 10.0f;
        adjustfreqs();
        break;
    case 15:
        Pautopan = value;
        autopan = ((float) Pautopan)/127.0f;
        if (autopan > 1.0f) autopan = 1.0f;
        if (autopan < 0.0f) autopan = 0.0f;
        break;
    case 16:
        Preverse = value;
        adjustfreqs();
        reinitfilter ();
        break;
    case 17:
        Pstages = value - 1;
//        for (int i=0; i<NUM_INF_BANDS; i++)  {
//        filterl[i]->setstages(value - 1);
//        filterr[i]->setstages(value - 1);
//        }
        phaserfb = 0.5f + (((float) (Pstages))/11.01f)*0.5f;
        break;
    };
};

int
Infinity::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        return (Pb[npar - 1]);
        break;
    case 9:
        return (Pq);
        break;
    case 10:
        return (Pstartfreq);
        break;
    case 11:
        return (Pendfreq);
        break;
    case 12:
        return (Prate);
        break;
    case 13:
        return (Pstdf);
        break;
    case 14:
        return (Psubdiv);
        break;
    case 15:
        return (Pautopan);
        break;
    case 16:
        return (Preverse);
        break;
    case 17:
        return (Pstages + 1);
        break;
    default:
        return (0);
    };

};
