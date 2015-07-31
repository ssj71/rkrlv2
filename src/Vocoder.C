/*

  Vocoder.C - Vocoder effect
  Author: Ryam Billing & Josep Andreu

  Adapted effect structure of ZynAddSubFX - a software synthesizer
  Author: Nasca Octavian Paul

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
#include "Vocoder.h"

Vocoder::Vocoder (float * efxoutl_, float * efxoutr_, float *auxresampled_,int bands, int DS, int uq, int dq,
		double sample_rate, uint32_t intermediate_bufsize)
{

    adjust(DS, sample_rate);

    VOC_BANDS = bands;
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;
    auxresampled = auxresampled_;
    //default values
    Ppreset = 0;
    Pvolume = 50;
    Plevel = 0;
    Pinput = 0;
    Ppanning = 64;
    Plrcross = 100;

    //temp value till period actually known
    nPERIOD = intermediate_bufsize*nRATIO;

    filterbank = (fbank *) malloc(sizeof(fbank) * VOC_BANDS);
    tmpl = (float *) malloc (sizeof (float) * nPERIOD);
    tmpr = (float *) malloc (sizeof (float) * nPERIOD);
    tsmpsl = (float *) malloc (sizeof (float) * nPERIOD);
    tsmpsr = (float *) malloc (sizeof (float) * nPERIOD);
    tmpaux = (float *) malloc (sizeof (float) * nPERIOD);



    Pmuffle = 10;
    float tmp = 0.01f;  //10 ms decay time on peak detectors
    alpha = ncSAMPLE_RATE/(ncSAMPLE_RATE + tmp);
    beta = 1.0f - alpha;
    prls = beta;
    gate = 0.005f;


    tmp = 0.05f; //50 ms att/rel on compressor
    calpha =  ncSAMPLE_RATE/(ncSAMPLE_RATE + tmp);
    cbeta = 1.0f - calpha;
    cthresh = 0.25f;
    cpthresh = cthresh; //dynamic threshold
    cratio = 0.25f;


    float center;
    float qq;

    A_Resample = new Resample(dq);
    U_Resample = new Resample(dq);
    D_Resample = new Resample(uq);


    interpbuf = new float[intermediate_bufsize];
    for (int i = 0; i < VOC_BANDS; i++) {
        center = (float) i * 20000.0f/((float) VOC_BANDS);
        qq = 60.0f;

        filterbank[i].l = new AnalogFilter (4, center, qq, 0, sample_rate, interpbuf);
        filterbank[i].l->setSR(nSAMPLE_RATE);
        filterbank[i].r = new AnalogFilter (4, center, qq, 0, sample_rate, interpbuf);
        filterbank[i].r->setSR(nSAMPLE_RATE);
        filterbank[i].aux = new AnalogFilter (4, center, qq, 0, sample_rate, interpbuf);
        filterbank[i].aux->setSR(nSAMPLE_RATE);
    };

    vlp = new AnalogFilter (2, 4000.0f, 1.0f, 1, sample_rate, interpbuf);
    vhp = new AnalogFilter (3, 200.0f, 0.707f, 1, sample_rate, interpbuf);

    vlp->setSR(nSAMPLE_RATE);
    vhp->setSR(nSAMPLE_RATE);

    setbands(VOC_BANDS, 200.0f, 4000.0f);
    setpreset (Ppreset);

};

Vocoder::~Vocoder ()
{
    free(filterbank);
    free(tmpl);
    free(tmpr);
    free(tsmpsl);
    free(tsmpsr);
    free(tmpaux);
    delete A_Resample;
    delete U_Resample;
    delete D_Resample;
    delete[] interpbuf;
    for (int i = 0; i < VOC_BANDS; i++) {
    	delete filterbank[i].l;
    	delete filterbank[i].r;
    	delete filterbank[i].aux;
    }
    delete vlp;
    delete vhp;
};

/*
 * Cleanup the effect
 */
void
Vocoder::cleanup ()
{
    for(int k=0; k<VOC_BANDS; k++) {
        filterbank[k].l->cleanup();
        filterbank[k].r->cleanup();
        filterbank[k].aux->cleanup();
        filterbank[k].speak = 0.0f;
        filterbank[k].gain = 0.0f;
        filterbank[k].oldgain = 0.0f;

    }
    vhp->cleanup();
    vlp->cleanup();

    compeak = compg = compenv = oldcompenv = 0.0f;

};


void
Vocoder::adjust(int DS, double SAMPLE_RATE)
{

    DS_state=DS;


    switch(DS) {

    case 0:
        nRATIO = 1;
        nSAMPLE_RATE = SAMPLE_RATE;
        nfSAMPLE_RATE = SAMPLE_RATE;
        break;

    case 1:
        nRATIO = 96000.0f/SAMPLE_RATE;
        nSAMPLE_RATE = 96000;
        nfSAMPLE_RATE = 96000.0f;
        break;


    case 2:
        nRATIO = 48000.0f/SAMPLE_RATE;
        nSAMPLE_RATE = 48000;
        nfSAMPLE_RATE = 48000.0f;
        break;

    case 3:
        nRATIO = 44100.0f/SAMPLE_RATE;
        nSAMPLE_RATE = 44100;
        nfSAMPLE_RATE = 44100.0f;
        break;

    case 4:
        nRATIO = 32000.0f/SAMPLE_RATE;
        nSAMPLE_RATE = 32000;
        nfSAMPLE_RATE = 32000.0f;
        break;

    case 5:
        nRATIO = 22050.0f/SAMPLE_RATE;
        nSAMPLE_RATE = 22050;
        nfSAMPLE_RATE = 22050.0f;
        break;

    case 6:
        nRATIO = 16000.0f/SAMPLE_RATE;
        nSAMPLE_RATE = 16000;
        nfSAMPLE_RATE = 16000.0f;
        break;

    case 7:
        nRATIO = 12000.0f/SAMPLE_RATE;
        nSAMPLE_RATE = 12000;
        nfSAMPLE_RATE = 12000.0f;
        break;

    case 8:
        nRATIO = 8000.0f/SAMPLE_RATE;
        nSAMPLE_RATE = 8000;
        nfSAMPLE_RATE = 8000.0f;
        break;

    case 9:
        nRATIO = 4000.0f/SAMPLE_RATE;
        nSAMPLE_RATE = 4000;
        nfSAMPLE_RATE = 4000.0f;
        break;

    }

    ncSAMPLE_RATE = 1.0f / nfSAMPLE_RATE;

}




/*
 * Effect output
 */
void
Vocoder::out (float * smpsl, float * smpsr, uint32_t period)
{
    int i, j;

    float tempgain;
    float maxgain=0.0f;
    float auxtemp, tmpgain;

    //This should probably be moved to a separate function so it doesn't need to recalculate every time
    nPERIOD = lrintf((float)period*nRATIO);
    u_up= (double)nPERIOD / (double)period;
    u_down= (double)period / (double)nPERIOD;

    if(DS_state != 0) {
        A_Resample->mono_out(auxresampled,tmpaux,period,u_up,nPERIOD);
    } else
        memcpy(tmpaux,auxresampled,sizeof(float)*nPERIOD);


    for (i = 0; i<nPERIOD; i++) {  //apply compression to auxresampled
        auxtemp = input * tmpaux[i];
        if(fabs(auxtemp > compeak)) compeak = fabs(auxtemp);   //First do peak detection on the signal
        compeak *= prls;
        compenv = cbeta * oldcompenv + calpha * compeak;       //Next average into envelope follower
        oldcompenv = compenv;

        if(compenv > cpthresh) {                              //if envelope of signal exceeds thresh, then compress
            compg = cpthresh + cpthresh*(compenv - cpthresh)/compenv;
            cpthresh = cthresh + cratio*(compg - cpthresh);   //cpthresh changes dynamically
            tmpgain = compg/compenv;
        } else {
            tmpgain = 1.0f;
        }



        if(compenv < cpthresh) cpthresh = compenv;
        if(cpthresh < cthresh) cpthresh = cthresh;

        tmpaux[i] = auxtemp * tmpgain;

        tmpaux[i]=vlp->filterout_s(tmpaux[i]);
        tmpaux[i]=vhp->filterout_s(tmpaux[i]);

    };


    //End compression

    auxtemp = 0.0f;

    if(DS_state != 0) {
        U_Resample->out(smpsl,smpsr,tsmpsl,tsmpsr,period,u_up);
    } else {
        memcpy(tsmpsl,smpsl,sizeof(float)*nPERIOD);
        memcpy(tsmpsr,smpsr,sizeof(float)*nPERIOD);
    }


    memset(tmpl,0,sizeof(float)*nPERIOD);
    memset(tmpr,0,sizeof(float)*nPERIOD);



    for (j = 0; j < VOC_BANDS; j++) {

        for (i = 0; i<nPERIOD; i++) {
            auxtemp = tmpaux[i];

            if(filterbank[j].speak < gate) filterbank[j].speak = 0.0f;  //gate
            if(auxtemp>maxgain) maxgain = auxtemp; //vu meter level.

            auxtemp = filterbank[j].aux->filterout_s(auxtemp);
            if(fabs(auxtemp) > filterbank[j].speak) filterbank[j].speak = fabs(auxtemp);  //Leaky Peak detector

            filterbank[j].speak*=prls;

            filterbank[j].gain = beta * filterbank[j].oldgain + alpha * filterbank[j].speak;
            filterbank[j].oldgain = filterbank[j].gain;


            tempgain = (1.0f-ringworm)*filterbank[j].oldgain+ringworm*auxtemp;

            tmpl[i] +=filterbank[j].l->filterout_s(tsmpsl[i])*tempgain;
            tmpr[i] +=filterbank[j].r->filterout_s(tsmpsr[i])*tempgain;

        };


    };


    for (i = 0; i<nPERIOD; i++) {
        tmpl[i]*=lpanning*level;
        tmpr[i]*=rpanning*level;
    };


    if(DS_state != 0) {
        D_Resample->out(tmpl,tmpr,efxoutl,efxoutr,nPERIOD,u_down);
    } else {
        memcpy(efxoutl,tmpl,sizeof(float)*nPERIOD);
        memcpy(efxoutr,tmpr,sizeof(float)*nPERIOD);
    }

    vulevel = (float)CLAMP(rap2dB(maxgain), -48.0, 15.0);



};

void
Vocoder::setbands (int numbands, float startfreq, float endfreq)
{
    float start = startfreq;   //useful variables
    float endband = endfreq;
    float fnumbands = (float) numbands;
    float output[VOC_BANDS + 1];
    int k;

    //calculate intermediate values
    float pwer = logf(endband/start)/log(2.0f);

    for(k=0; k<=VOC_BANDS; k++) output[k] = start*f_pow2(((float) k)*pwer/fnumbands);
    for(k=0; k<VOC_BANDS; k++) {
        filterbank[k].sfreq = output[k] + (output[k+1] - output[k])*0.5f;
        filterbank[k].sq = filterbank[k].sfreq/(output[k+1] - output[k]);

        filterbank[k].l->setfreq_and_q (filterbank[k].sfreq, filterbank[k].sq);
        filterbank[k].r->setfreq_and_q (filterbank[k].sfreq, filterbank[k].sq);
        filterbank[k].aux->setfreq_and_q (filterbank[k].sfreq, filterbank[k].sq);
    }
    cleanup();

}

/*
 * Parameter control
 */
void
Vocoder::setvolume (int Pvolume)
{
    this->Pvolume = Pvolume;
    outvolume = (float)Pvolume / 127.0f;
    if (Pvolume == 0)
        cleanup ();

};

void
Vocoder::setpanning (int Ppanning)
{
    this->Ppanning = Ppanning;
    lpanning = ((float)Ppanning + 0.5f) / 127.0f;
    rpanning = 1.0f - lpanning;
};


void
Vocoder::init_filters()
{
    float ff, qq;

    for (int ii = 0; ii < VOC_BANDS; ii++) {
        ff = filterbank[ii].sfreq;
        qq = filterbank[ii].sq;
        filterbank[ii].l->setfreq_and_q (ff, qq);
        filterbank[ii].r->setfreq_and_q (ff, qq);
        filterbank[ii].aux->setfreq_and_q (ff, qq);
    };

}

void
Vocoder::adjustq(float q)
{

    for (int ii = 0; ii < VOC_BANDS; ii++) {
        filterbank[ii].l->setq (q);
        filterbank[ii].r->setq (q);
        filterbank[ii].aux->setq (q);
    };

}

void
Vocoder::setpreset (int npreset)
{
    const int PRESET_SIZE = 7;
    const int NUM_PRESETS = 4;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Vocoder 1
        {0, 64, 10, 70, 70, 40, 0},
        //Vocoder 2
        {0, 64, 14, 80, 70, 40, 32},
        //Vocoder 3
        {0, 64, 20, 90, 70, 40, 64},
        //Vocoder 4
        {0, 64, 30, 100, 70, 40, 127}
    };

    if(npreset>NUM_PRESETS-1) {
        Fpre->ReadPreset(35,npreset-NUM_PRESETS+1,pdata);
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, pdata[n]);
    } else {
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, presets[npreset][n]);
    }

    Ppreset = npreset;
};


void
Vocoder::changepar (int npar, int value)
{
    float tmp = 0;
    switch (npar) {
    case 0:
        setvolume (value);
        break;
    case 1:
        setpanning (value);
        break;
    case 2:
        Pmuffle = value;
        tmp = (float) Pmuffle;
        tmp *= 0.0001f + tmp/64000;
        alpha = ncSAMPLE_RATE/(ncSAMPLE_RATE + tmp);
        beta = 1.0f - alpha;
        break;
    case 3:
        Pqq = value;
        tmp = (float) value;
        adjustq(tmp);
        break;
    case 4:
        Pinput = value;
        input = dB2rap (75.0f * (float)Pinput / 127.0f - 40.0f);
        break;
    case 5:
        Plevel = value;
        level = dB2rap (60.0f * (float)Plevel / 127.0f - 40.0f);
        break;

    case 6:
        Pring = value;
        ringworm = (float) Pring/127.0f;
        break;
    };
};

int
Vocoder::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
        return (Ppanning);
        break;
    case 2:
        return(Pmuffle);
        break;
    case 3:
        return(Pqq);
        break;
    case 4:
        return (Pinput);
        break;
    case 5:
        return (Plevel);
        break;
    case 6:
        return (Pring);
        break;

    };
    return (0);			//in case of bogus parameter number
};
