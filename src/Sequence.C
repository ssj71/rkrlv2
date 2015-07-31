/*
  Rakarrack Guitar FX

  Sequence.C - Simple compressor/Sequence effect with easy interface, minimal controls
  Copyright (C) 2010 Ryan Billing
  Author: Ryan Billing & Josep Andreu

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 3 of the GNU General Public License
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
#include "Sequence.h"
#include <time.h>
#include "f_sin.h"

Sequence::Sequence (float * efxoutl_, float * efxoutr_, long int Quality, int DS, int uq, int dq, double sample_rate, uint32_t intermediate_bufsize)
{
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;
    hq = Quality;
    fSAMPLE_RATE = sample_rate;
    adjust(DS, sample_rate);

    //temp value till period actually known
    nPERIOD = intermediate_bufsize*nRATIO;

    templ = (float *) malloc (sizeof (float) * intermediate_bufsize);
    tempr = (float *) malloc (sizeof (float) * intermediate_bufsize);

    outi = (float *) malloc (sizeof (float) * nPERIOD);
    outo = (float *) malloc (sizeof (float) * nPERIOD);

    U_Resample = new Resample(dq);
    D_Resample = new Resample(uq);

    beats = new beattracker(sample_rate, intermediate_bufsize);

    filterl = NULL;
    filterr = NULL;

    MAXFREQ = 10000.0f;
    MINFREQ = 100.0f;
    fq = 75.0f;
    Ppreset = 0;
    scount = 0;
    tcount = 0;
    rndflag = 0;
    subdiv = 2;
    lmod = 0.5f;
    rmod = 0.5f;
    interpbuf = new float[intermediate_bufsize];
    filterl = new RBFilter (0, 80.0f, 40.0f, 2,sample_rate, interpbuf);
    filterr = new RBFilter (0, 80.0f, 40.0f, 2,sample_rate, interpbuf);
    modfilterl = new RBFilter (0, 15.0f, 0.5f, 1,sample_rate, interpbuf);
    modfilterr = new RBFilter (0, 15.0f, 0.5f, 1,sample_rate, interpbuf);
    rmsfilter = new RBFilter (0, 15.0f, 0.15f, 1,sample_rate, interpbuf);
    peaklpfilter = new RBFilter (0, 25.0f, 0.5f, 0,sample_rate, interpbuf);
    peaklpfilter2 = new RBFilter (0, 25.0f, 0.5f, 0,sample_rate, interpbuf);
    peakhpfilter = new RBFilter (1, 45.0f, 0.5f, 0,sample_rate, interpbuf);

//Trigger Filter Settings
    peakpulse = peak = envrms = 0.0f;
    peakdecay = 10.0f/sample_rate;
    targatk = 12.0f/sample_rate;   ///for smoothing filter transition
    atk = 200.0f/sample_rate;
    trigtime = sample_rate/12; //time to take next peak
    onset = 0;
    trigthresh = 0.15f;

    setpreset (Ppreset);

    filterl->setmix(1, 0.33f, -1.0f, 0.25f);
    filterr->setmix(1, 0.33f, -1.0f, 0.25f);

    maxdly = 4.0f;  //sets max time to 4 seconds
    tempodiv = maxdly;
    ldelay = new delayline(maxdly, 1, sample_rate);
    rdelay = new delayline(maxdly, 1, sample_rate);
    fb = 0.0f;
    rdlyfb = 0.0f;
    ldlyfb = 0.0f;
    avtime = 0.25f;
    avflag = 1;

    PS = new PitchShifter (window, hq, nfSAMPLE_RATE);
    PS->ratio = 1.0f;

    cleanup ();
};

Sequence::~Sequence ()
{
    free(templ);
    free(tempr);
    free(outi);
    free(outo);
    delete U_Resample;
    delete D_Resample;
    delete beats;
    delete filterl;
    delete filterr;
    delete modfilterl;
    delete modfilterr;
    delete rmsfilter;
    delete peaklpfilter;
    delete peaklpfilter2;
    delete peakhpfilter;
    delete ldelay;
    delete rdelay;
    delete PS;
    delete[] interpbuf;
};

/*
 * Cleanup the effect
 */
void
Sequence::cleanup ()
{

    memset(outi, 0, sizeof(float)*nPERIOD);
    memset(outo, 0, sizeof(float)*nPERIOD);

    ldelay->cleanup();
    rdelay->cleanup();
    ldelay->set_averaging(0.25f);
    rdelay->set_averaging(0.25f);

};




/*
 * Effect output
 */
void
Sequence::out (float * smpsl, float * smpsr, uint32_t period)
{
    unsigned int i;
    int nextcount,dnextcount;
    int hPERIOD;

    float ldiff, rdiff, lfol, lfor, ftcount;
    float ldbl, ldbr;
    float tmp;

    float ltarget,rtarget;

    if (avflag) {
        ldelay->set_averaging(avtime);
        rdelay->set_averaging(avtime);
        avflag = 0;
    }

    if((Pmode==3)||(Pmode ==5) || (Pmode==6)){
    	//This should probably be moved to a separate function so it doesn't need to recalculate every time
        nPERIOD = lrintf((float)period*nRATIO);
        u_up= (double)nPERIOD / (double)period;
        u_down= (double)period / (double)nPERIOD;
    	hPERIOD=nPERIOD;
    }
    else hPERIOD=period;


    if ((rndflag) && (tcount < hPERIOD + 1)) { //This is an Easter Egg
        srand(time(NULL));
        for (i = 0; i<8; i++) {
            fsequence[i] = RND1;
        }
    }




    switch(Pmode) {
    case 0:	//Lineal

        nextcount = scount + 1;
        if (nextcount > 7 ) nextcount = 0;
        ldiff = ifperiod * (fsequence[nextcount] - fsequence[scount]);
        lfol = fsequence[scount];

        dscount = (scount + Pstdiff) % 8;
        dnextcount = dscount + 1;
        if (dnextcount > 7 ) dnextcount = 0;
        rdiff = ifperiod * (fsequence[dnextcount] - fsequence[dscount]);
        lfor = fsequence[dscount];

        for ( i = 0; i < period; i++) { //Maintain sequenced modulator

            if (++tcount >= intperiod) {
                tcount = 0;
                scount++;
                if(scount > 7) scount = 0;  //reset to beginning of sequence buffer

                nextcount = scount + 1;
                if (nextcount > 7 ) nextcount = 0;
                ldiff = ifperiod * (fsequence[nextcount] - fsequence[scount]);
                lfol = fsequence[scount];

                dscount = (scount + Pstdiff) % 8;
                dnextcount = dscount + 1;
                if (dnextcount > 7 ) dnextcount = 0;
                rdiff = ifperiod * (fsequence[dnextcount] - fsequence[dscount]);
                lfor = fsequence[dscount];
            }

            ftcount = (float) tcount;


            lmod = lfol + ldiff * ftcount;
            rmod = lfor + rdiff * ftcount;

            if (Pamplitude) {
                ldbl = lmod * (1.0f - cosf(D_PI*ifperiod*ftcount));
                ldbr = rmod * (1.0f - cosf(D_PI*ifperiod*ftcount));

                efxoutl[i] = ldbl * smpsl[i];
                efxoutr[i] = ldbr * smpsr[i];
            }

            float frl = MINFREQ + MAXFREQ*lmod;
            float frr = MINFREQ + MAXFREQ*rmod;

            if ( i % 8 == 0) {
                filterl->setfreq_and_q (frl, fq);
                filterr->setfreq_and_q (frr, fq);
            }

            efxoutl[i] = filterl->filterout_s(efxoutl[i]);
            efxoutr[i] = filterr->filterout_s (efxoutr[i]);

        }
        break;

    case 1:		//Up Down


        for ( i = 0; i < period; i++) { //Maintain sequenced modulator

            if (++tcount >= intperiod) {
                tcount = 0;
                scount++;
                if(scount > 7) scount = 0;  //reset to beginning of sequence buffer
                dscount = (scount + Pstdiff) % 8;
            }

            ftcount = M_PI * ifperiod * (float)(tcount);

            lmod = sinf(ftcount)*fsequence[scount];
            rmod = sinf(ftcount)*fsequence[dscount];

            if (Pamplitude) {
                ldbl = lmod * (1.0f - cosf(2.0f*ftcount));
                ldbr = rmod * (1.0f - cosf(2.0f*ftcount));

                efxoutl[i] = ldbl * smpsl[i];
                efxoutr[i] = ldbr * smpsr[i];
            }

            float frl = MINFREQ + MAXFREQ*lmod;
            float frr = MINFREQ + MAXFREQ*rmod;


            if ( i % 8 == 0) {
                filterl->setfreq_and_q (frl, fq);
                filterr->setfreq_and_q (frr, fq);
            }

            efxoutl[i] = filterl->filterout_s (efxoutl[i]);
            efxoutr[i] = filterr->filterout_s (efxoutr[i]);

        }

        break;

    case 2:  //Stepper

        for ( i = 0; i < period; i++) { //Maintain sequenced modulator

            if (++tcount >= intperiod) {
                tcount = 0;
                scount++;
                if(scount > 7) scount = 0;  //reset to beginning of sequence buffer
                dscount = (scount + Pstdiff) % 8;
            }

            lmod = fsequence[scount];
            rmod = fsequence[dscount];

            lmod = modfilterl->filterout_s(lmod);
            rmod = modfilterr->filterout_s(rmod);

            if (Pamplitude) {
                ldbl = seqpower * lmod;
                ldbr = seqpower * rmod;

                efxoutl[i] = ldbl * smpsl[i];
                efxoutr[i] = ldbr * smpsr[i];
            }

            float frl = MINFREQ + lmod * MAXFREQ;
            float frr = MINFREQ + rmod * MAXFREQ;


            if ( i % 8 == 0) {
                filterl->setfreq_and_q (frl, fq);
                filterr->setfreq_and_q (frr, fq);
            }

            efxoutl[i] = filterl->filterout_s (efxoutl[i]);
            efxoutr[i] = filterr->filterout_s (efxoutr[i]);

        }

        break;

    case 3:  //Shifter

        nextcount = scount + 1;
        if (nextcount > 7 ) nextcount = 0;
        ldiff = ifperiod * (fsequence[nextcount] - fsequence[scount]);
        lfol = fsequence[scount];

        if(DS_state != 0) {
            memcpy(templ, smpsl,sizeof(float)*period);
            memcpy(tempr, smpsr,sizeof(float)*period);
            U_Resample->out(templ,tempr,smpsl,smpsr,period,u_up);
        }


        for ( i = 0; i < nPERIOD; i++) { //Maintain sequenced modulator

            if (++tcount >= intperiod) {
                tcount = 0;
                scount++;
                if(scount > 7) scount = 0;  //reset to beginning of sequence buffer

                nextcount = scount + 1;
                if (nextcount > 7 ) nextcount = 0;
                ldiff = ifperiod * (fsequence[nextcount] - fsequence[scount]);
                lfol = fsequence[scount];
            }

            ftcount = (float) tcount;

            lmod = 1.0f + lfol + ldiff * ftcount;

            if (Pamplitude) lmod = 1.0f - (lfol + ldiff * ftcount) * .5f;

            outi[i] = (smpsl[i] + smpsr[i])*.5;
            if (outi[i] > 1.0)
                outi[i] = 1.0f;
            if (outi[i] < -1.0)
                outi[i] = -1.0f;
        }



        PS->ratio = lmod;
        PS->smbPitchShift (PS->ratio, nPERIOD, window, hq, nfSAMPLE_RATE, outi, outo);


        memcpy(templ, outo, sizeof(float)*nPERIOD);
        memcpy(tempr, outo, sizeof(float)*nPERIOD);

        if(DS_state != 0) {
            D_Resample->out(templ,tempr,efxoutl,efxoutr,nPERIOD,u_down);
        } else {
            memcpy(efxoutl, templ,sizeof(float)*period);
            memcpy(efxoutr, tempr,sizeof(float)*period);
        }






        break;

    case 4:      //Tremor

        nextcount = scount + 1;
        if (nextcount > 7 ) nextcount = 0;
        ldiff = ifperiod * (fsequence[nextcount] - fsequence[scount]);
        lfol = fsequence[scount];

        dscount = (scount + Pstdiff) % 8;
        dnextcount = dscount + 1;
        if (dnextcount > 7 ) dnextcount = 0;
        rdiff = ifperiod * (fsequence[dnextcount] - fsequence[dscount]);
        lfor = fsequence[dscount];

        for ( i = 0; i < period; i++) { //Maintain sequenced modulator

            if (++tcount >= intperiod) {
                tcount = 0;
                scount++;
                if(scount > 7) scount = 0;  //reset to beginning of sequence buffer

                nextcount = scount + 1;
                if (nextcount > 7 ) nextcount = 0;
                ldiff = ifperiod * (fsequence[nextcount] - fsequence[scount]);
                lfol = fsequence[scount];

                dscount = (scount + Pstdiff) % 8;
                dnextcount = dscount + 1;
                if (dnextcount > 7 ) dnextcount = 0;
                rdiff = ifperiod * (fsequence[dnextcount] - fsequence[dscount]);
                lfor = fsequence[dscount];
            }
//Process Amplitude modulation
            if (Pamplitude) {
                ftcount = (float) tcount;
                lmod = lfol + ldiff * ftcount;
                rmod = lfor + rdiff * ftcount;

                ldbl = seqpower * lmod * (1.0f - cosf(D_PI*ifperiod*ftcount));
                ldbr = seqpower * rmod * (1.0f - cosf(D_PI*ifperiod*ftcount));

                efxoutl[i] = ldbl * smpsl[i];
                efxoutr[i] = ldbr * smpsr[i];
            } else {
                lmod = seqpower * fsequence[scount];
                rmod = seqpower * fsequence[dscount];
                lmod = modfilterl->filterout_s(lmod);
                rmod = modfilterr->filterout_s(rmod);

                efxoutl[i] = lmod * smpsl[i];
                efxoutr[i] = rmod * smpsr[i];
            }


        };
        break;

    case 5:  //Arpegiator
        lfol = floorf(fsequence[scount]*12.75f);

        if(DS_state != 0) {
            memcpy(templ, smpsl,sizeof(float)*period);
            memcpy(tempr, smpsr,sizeof(float)*period);
            U_Resample->out(templ,tempr,smpsl,smpsr,period,u_up);
        }



        for ( i = 0; i < nPERIOD; i++) { //Maintain sequenced modulator

            if (++tcount >= intperiod) {
                tcount = 0;
                scount++;
                if(scount > 7) scount = 0;  //reset to beginning of sequence buffer
                lfol = floorf(fsequence[scount]*12.75f);
            }

            lmod = powf (2.0f, lfol / 12.0f);

            if (Pamplitude) lmod = powf (2.0f, -lfol / 12.0f);

            outi[i] = (smpsl[i] + smpsr[i])*.5;
            if (outi[i] > 1.0)
                outi[i] = 1.0f;
            if (outi[i] < -1.0)
                outi[i] = -1.0f;
        }


        PS->ratio = lmod;
        PS->smbPitchShift (PS->ratio, nPERIOD, window, hq, nfSAMPLE_RATE, outi, outo);



        memcpy(templ, outo, sizeof(float)*nPERIOD);
        memcpy(tempr, outo, sizeof(float)*nPERIOD);

        if(DS_state != 0) {
            D_Resample->out(templ,tempr,efxoutl,efxoutr,nPERIOD,u_down);
        } else {
            memcpy(efxoutl, templ,sizeof(float)*nPERIOD);
            memcpy(efxoutr, tempr,sizeof(float)*nPERIOD);
        }


        break;

    case 6:  //Chorus

        nextcount = scount + 1;
        if (nextcount > 7 ) nextcount = 0;
        ldiff = ifperiod * (fsequence[nextcount] - fsequence[scount]);
        lfol = fsequence[scount];

        if(DS_state != 0) {
            memcpy(templ, smpsl,sizeof(float)*period);
            memcpy(tempr, smpsr,sizeof(float)*period);
            U_Resample->out(templ,tempr,smpsl,smpsr,period,u_up);
        }



        for ( i = 0; i < nPERIOD; i++) { //Maintain sequenced modulator

            if (++tcount >= intperiod) {
                tcount = 0;
                scount++;
                if(scount > 7) scount = 0;  //reset to beginning of sequence buffer

                nextcount = scount + 1;
                if (nextcount > 7 ) nextcount = 0;
                ldiff = ifperiod * (fsequence[nextcount] - fsequence[scount]);
                lfol = fsequence[scount];
            }

            ftcount = (float) tcount;

            lmod = 1.0f + (lfol + ldiff * ftcount)*.03f;
            if (Pamplitude) lmod = 1.0f - (lfol + ldiff * ftcount)*.03f;

            outi[i] = (smpsl[i] + smpsr[i])*.5;
            if (outi[i] > 1.0)
                outi[i] = 1.0f;
            if (outi[i] < -1.0)
                outi[i] = -1.0f;
        }


        PS->ratio = lmod;
        PS->smbPitchShift (PS->ratio, nPERIOD, window, hq, nfSAMPLE_RATE, outi, outo);

        if(Pstdiff==1) {
            for ( i = 0; i < nPERIOD; i++) {
                templ[i]=smpsl[i]-smpsr[i]+outo[i];
                tempr[i]=smpsl[i]-smpsr[i]+outo[i];
            }
        } else if(Pstdiff==2) {
            for ( i = 0; i < nPERIOD; i++) {
                templ[i]=outo[i]*(1.0f-panning);
                tempr[i]=outo[i]*panning;
            }
        } else {
            memcpy(templ, outo, sizeof(float)*nPERIOD);
            memcpy(tempr, outo, sizeof(float)*nPERIOD);
        }

        if(DS_state != 0) {
            D_Resample->out(templ,tempr,efxoutl,efxoutr,nPERIOD,u_down);
        } else {
            memcpy(efxoutl, templ,sizeof(float)*nPERIOD);
            memcpy(efxoutr, tempr,sizeof(float)*nPERIOD);
        }



        break;

    case 7:  //TrigStepper

        //testing beattracker object -- doesn't do anything useful yet other than a convenient place
        //to see how well it performs.
        beats->detect(smpsl, smpsr, period);

        for ( i = 0; i < period; i++) { //Detect dynamics onset

            tmp = 10.0f*fabs(smpsl[i] + smpsr[i]);
            envrms = rmsfilter->filterout_s(tmp);
            if ( tmp > peak) peak =  atk + tmp;
            if ( envrms < peak) peak -= peakdecay;
            if(peak<0.0f) peak = 0.0f;

            peakpulse = peaklpfilter2->filterout_s(fabs(peakhpfilter->filterout_s(peak)));


            if( peakpulse > trigthresh ) {
                if (trigtimeout==0) {
                    onset = 1;
                    trigtimeout = trigtime;
                } else {
                    onset = 0;
                }
            } else {
                if (--trigtimeout<0) {
                    trigtimeout = 0;
                }

            }



            if (onset) {
                tcount = 0;
                scount++;
                if(scount > 7) scount = 0;  //reset to beginning of sequence buffer
                dscount = (scount + Pstdiff) % 8;
            }

            ltarget = fsequence[scount];
            rtarget = fsequence[dscount];

            if (lmod<ltarget) lmod+=targatk;
            else lmod-=targatk;
            if (rmod<rtarget) rmod+=targatk;
            else rmod-=targatk;
            ltarget = peaklpfilter->filterout_s(lmod);
            rtarget = peaklpfilter->filterout_s(rmod);

            if (Pamplitude) {
                ldbl = seqpower * ltarget;
                ldbr = seqpower * rtarget;

                efxoutl[i] = ldbl * smpsl[i];
                efxoutr[i] = ldbr * smpsr[i];
            }

            float frl = MINFREQ + ltarget * MAXFREQ;
            float frr = MINFREQ + rtarget * MAXFREQ;


            if ( i % 8 == 0) {
                filterl->setfreq_and_q (frl, fq);
                filterr->setfreq_and_q (frr, fq);
            }

            efxoutl[i] = filterl->filterout_s (efxoutl[i]);
            efxoutr[i] = filterr->filterout_s (efxoutr[i]);

            //efxoutl[i] += triggernow;  //test to see the pulse
            //efxoutr[i] = peakpulse;
        }

        break;

    case 8:  //delay

        for ( i = 0; i < period; i++) { //Maintain sequenced modulator

            if (++tcount >= intperiod) {
                tcount = 0;
                scount++;
                if(scount > 7) scount = 0;  //reset to beginning of sequence buffer
                dscount = (scount + Pstdiff) % 8;
            }

            if (Pamplitude) {
                ftcount = M_PI * ifperiod * (float)(tcount);

                lmod = f_sin(ftcount)*fsequence[scount];
                rmod = f_sin(ftcount)*fsequence[dscount];

                ldbl = lmod * (1.0f - f_cos(2.0f*ftcount));
                ldbr = rmod * (1.0f - f_cos(2.0f*ftcount));

                lmod = tempodiv*fsequence[scount];
                rmod = tempodiv*fsequence[dscount];

                efxoutl[i] = ldbl*ldelay->delay((ldlyfb + smpsl[i]), lmod, 0, 1, 0);
                efxoutr[i] = ldbr*rdelay->delay((rdlyfb + smpsr[i]), rmod, 0, 1, 0);

            }

            lmod = tempodiv*fsequence[scount];
            rmod = tempodiv*fsequence[dscount];

            efxoutl[i] = ldelay->delay_simple((ldlyfb + smpsl[i]), lmod, 0, 1, 0);
            efxoutr[i] = rdelay->delay_simple((rdlyfb + smpsr[i]), rmod, 0, 1, 0);

            ldlyfb = fb*efxoutl[i];
            rdlyfb = fb*efxoutr[i];

        }
        break;
        // here case 9:
        //
        // break;




    }

};


/*
 * Parameter control
 */


void
Sequence::setranges(int value)
{



    switch(value) {

    case 1:              //typical for wahwah pedal
        MINFREQ = 450.0f;
        MAXFREQ = 2500.0f;
        break;
    case 2:
        MINFREQ = 150.0f;
        MAXFREQ = 4000.0f;
        break;
    case 3:
        MINFREQ = 40.0f;
        MAXFREQ = 800.0f;
        break;
    case 4:
        MINFREQ = 100.0f;
        MAXFREQ = 1600.0f;
        break;
    case 5:
        MINFREQ = 80.0f;
        MAXFREQ = 16000.0f;
        break;
    case 6:
        MINFREQ = 60.0f;
        MAXFREQ = 18000.0f;
        break;
    case 7:
        MINFREQ = 40.0f;
        MAXFREQ = 2200.0f;
        break;
    case 8:
        MINFREQ = 20.0f;
        MAXFREQ = 6000.0f;
        break;


    }
}


void
Sequence::adjust(int DS, double SAMPLE_RATE)
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
Sequence::settempo(int value)
{
    if ((Pmode==3) || (Pmode==5) || (Pmode==6))  fperiod = nfSAMPLE_RATE * 60.0f/(subdiv * (float) value);
    else  fperiod = fSAMPLE_RATE * 60.0f/(subdiv * (float) value);  //number of samples before next value

    ifperiod = 1.0f/fperiod;
    intperiod = (int) fperiod;


    tempodiv = 240.0f/((float) value);
    if (tempodiv>maxdly) tempodiv = maxdly;
    avtime = 60.0f/((float) value);
    avflag = 1;

}




void
Sequence::setpreset (int npreset)
{
    const int PRESET_SIZE = 15;
    const int NUM_PRESETS = 10;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Jumpy
        {20, 100, 10, 50, 25, 120, 60, 127, 0, 90, 40, 0, 0, 0, 3},
        //Stair Step
        {10, 20, 30, 50, 75, 90, 100, 127, 64, 90, 96, 0, 0, 2, 5},
        //Mild
        {20, 30, 10, 40, 25, 60, 100, 50, 0, 90, 40, 0, 0, 0, 4},
        //WahWah
        {11, 55, 15, 95, 12, 76, 11, 36, 30, 80, 110, 0, 4, 1, 2},
        //Filter Pan
        {28, 59, 94, 127, 120, 80, 50, 24, 64, 180, 107, 0, 3, 0, 8},
        //Stepper
        {30, 127, 30, 50, 80, 40, 110, 80, 0, 240, 95, 1, 1, 2, 2},
        //Shifter
        {0, 0, 127, 127, 0, 0, 127, 127, 64, 114, 64, 1, 0, 3, 0},
        //Tremor
        {30, 127, 30, 50, 80, 40, 110, 80, 0, 240, 95, 1, 1, 4, 2},
        //Boogie
        {0, 40, 50, 60, 70, 60, 40, 0, 0, 220, 64, 0, 0, 5, 0},
        //Chorus
        {64, 30, 45, 20, 60, 25, 42, 15, 64, 120, 64, 0, 0, 6, 0}

    };

    if(npreset>NUM_PRESETS-1) {
        Fpre->ReadPreset(37,npreset-NUM_PRESETS+1,pdata);
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, pdata[n]);
    } else {
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, presets[npreset][n]);
    }
    Ppreset = npreset;
};


void
Sequence::changepar (int npar, int value)
{
    int testegg, i;

    switch (npar) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        Psequence[npar] = value;
        fsequence[npar] = (float) value / 127.0f;

        seqpower = 0.0f;
        for (i = 0; i<8; i++)  seqpower += fsequence[i];
        if(seqpower > 0.1f) {
            seqpower = 15.0f/seqpower;
            rndflag = 0;
        }

        testegg = 0;
        for (i = 0; i<8; i++)  testegg += Psequence[i];
        if(testegg < 4) {
            seqpower = 5.0f;  //Easter egg
            rndflag = 1;
        }
        break;
    case 8:
        Pvolume = value;
        outvolume = (float)Pvolume / 127.0f;
        break;
    case 9:
        Ptempo = value;
        settempo(value);
        break;
    case 10:
        Pq = value;
        panning = (((float)value) + 64.0f) /128.0f;
        fq = powf (60.0f, ((float)value - 64.0f) / 64.0f);
        fb = ( (float) value)/128.0f;
        break;
    case 11:
        Pamplitude = value;
        break;
    case 12:
        Pstdiff = value;
        break;
    case 13:
        Pmode = value;
        settempo(Ptempo);
        lmod = 0.5f;
        rmod = 0.5f;
        break;
    case 14:
        Prange = value;
        setranges(Prange);
        break;

    };
};

int
Sequence::getpar (int npar)
{
    switch (npar) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        return (Psequence[npar]);
        break;
    case 8:
        return (Pvolume);
        break;
    case 9:
        return (Ptempo);
        break;
    case 10:
        return (Pq);
        break;
    case 11:
        return (Pamplitude);
        break;
    case 12:
        return (Pstdiff);
        break;
    case 13:
        return (Pmode);
        break;
    case 14:
        return (Prange);
        break;
    };
    return (0);			//in case of bogus parameter number
};
