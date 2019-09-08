/*
  rakarrack - a guitar effects software

 Vibe.C  -  Optical tremolo effect

  Copyright (C) 2008-2010 Ryan Billing
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

#include <math.h>
#include "Vibe.h"

Vibe::Vibe (float * efxoutl_, float * efxoutr_, double sample_rate)
{
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;

    cSAMPLE_RATE = 1.f/sample_rate;
    fSAMPLE_RATE = sample_rate;

//Swing was measured on operating device of: 10K to 250k.
//400K is reported to sound better for the "low end" (high resistance)
//Because of time response, Rb needs to be driven further.
//End resistance will max out to around 10k for most LFO freqs.
//pushing low end a little lower for kicks and giggles
    Ra = 500000.0f;  //Cds cell dark resistance.
    Ra = logf(Ra);		//this is done for clarity
    Rb = 600.0f;         //Cds cell full illumination
    b = exp(Ra/logf(Rb)) - CNST_E;
    dTC = 0.045f;
    //dTC = 0.085f;
    dRCl = dTC;
    dRCr = dTC;   //Right & left channel dynamic time contsants
    minTC = logf(0.0025f/dTC);
    //minTC = logf(0.0025f/dTC);
    alphal = 1.0f - cSAMPLE_RATE/(dRCl + cSAMPLE_RATE);
    alphar = alphal;
    dalphal = dalphar = alphal;
    lampTC = cSAMPLE_RATE/(0.012 + cSAMPLE_RATE);  //guessing twiddle factor
    ilampTC = 1.0f - lampTC;
    lstep = 0.0f;
    rstep = 0.0f;

    Pstereo = 1; //1 if you want to process in stereo, 0 to do mono proc
    Pdepth = 127;
    Ppanning = 64;
    lpanning = 1.0f;
    rpanning = 1.0f;
    fdepth = 1.0f;
    oldgl = 0.0f;
    oldgr = 0.0f;
    gl = 0.0f;
    gr = 0.0f;
    for(int jj = 0; jj<8; jj++) oldcvolt[jj] = 0.0f;
    lfo = new EffectLFO(sample_rate);
    PERIOD = 256;//best guess until we know better


    init_vibes();
    cleanup();

}

Vibe::~Vibe ()
{
	delete lfo;
}


void
Vibe::cleanup ()
{
    /* also used after bypass to reset filter state */
    for (int i = 0; i < 8; ++i) {
        vc[i].clear ();
        vcvo[i].clear ();
        ecvc[i].clear ();
        vevo[i].clear ();
        bootstrap[i].clear ();
    }

    /* should probably reset modulation state here,
     * but Vibe::cleanup is currently called periodically
     * on every bypass call. -- That need fixing first.
     */
};

void
Vibe::out (float *smpsl, float *smpsr, uint32_t period)
{

    unsigned int i,j;
    float lfol, lfor, xl, xr, fxl, fxr;
#ifdef VIBE_INLINE_PROCESS
    float vbe,vin;
#endif
    float cvolt, ocvolt, evolt, input;
    float emitterfb = 0.0f;
    float outl, outr;

    input = cvolt = ocvolt = evolt = 0.0f;

    lfo->effectlfoout (&lfol, &lfor);

    lfol = fdepth + lfol*fwidth;
    if (lfol > 1.0f)
        lfol = 1.0f;
    else if (lfol < 0.0f)
        lfol = 0.0f;
    lfol = 2.0f - 2.0f/(lfol + 1.0f); //emulate lamp turn on/off characteristic by typical curves

    if(Pstereo) {
        lfor = fdepth + lfor*fwidth;
        if (lfor > 1.0f) lfor = 1.0f;
        else if (lfor < 0.0f) lfor = 0.0f;
        lfor = 2.0f - 2.0f/(lfor + 1.0f);   //
    }

    for (i = 0; i < period; i++) {
        //Left Lamp
        gl = lfol*lampTC + oldgl*ilampTC;
        oldgl = gl;

        //Left Cds
        stepl = gl*alphal + dalphal*oldstepl;
        oldstepl = stepl;
        dRCl = dTC*f_exp(stepl*minTC);
        alphal = cSAMPLE_RATE/(dRCl + cSAMPLE_RATE);
        dalphal = 1.0f - cSAMPLE_RATE/(0.5f*dRCl + cSAMPLE_RATE);     //different attack & release character
        xl = CNST_E + stepl*b;
        fxl = f_exp(Ra/logf(xl));
        fxr = fxl;

        //Right Lamp
        if(Pstereo) {
            gr = lfor*lampTC + oldgr*ilampTC;
            oldgr = gr;

            //Right Cds
            stepr = gr*alphar + dalphar*oldstepr;
            oldstepr = stepr;
            dRCr = dTC*f_exp(stepr*minTC);
            alphar = cSAMPLE_RATE/(dRCr + cSAMPLE_RATE);
            dalphar = 1.0f - cSAMPLE_RATE/(0.5f*dRCr + cSAMPLE_RATE);      //different attack & release character
            xr = CNST_E + stepr*b;
            fxr = f_exp(Ra/logf(xr));
        }

        if(i%4 == 0)  modulate(fxl, fxr);
        //if(i%16 == 0)  modulate(fxl, fxr);

        //Left Channel
        input = bjt_shape(fbl + smpsl[i]);


#ifdef VIBE_INLINE_PROCESS
            vin = 7.5f*(1.0f + fbl+smpsl[i]);
            if(vin<0.0f) vin = 0.0f;
            if(vin>15.0f) vin = 15.0f;
            vbe = 0.8f - 0.8f/(vin + 1.0f);  //really rough, simplistic bjt turn-on emulator
            input = vin - vbe;
            input = input*0.1333333333f -0.90588f;  //some magic numbers to return gain to unity & zero the DC

#endif

        emitterfb = 25.0f/fxl;
        for(j=0; j<4; j++) { //4 stages phasing
#ifdef VIBE_INLINE_PROCESS
            //Inline filter implementation below
                float y0 = 0.0f;
                y0 = input*ecvc[j].n0 + ecvc[j].x1*ecvc[j].n1 - ecvc[j].y1*ecvc[j].d1;
                ecvc[j].y1 = y0 + DENORMAL_GUARD;
                ecvc[j].x1 = input;


                float x0 = 0.0f;
                float data = input + emitterfb*oldcvolt[j];
                x0 = data*vc[j].n0 + vc[j].x1*vc[j].n1 - vc[j].y1*vc[j].d1;
                vc[j].y1 = x0 + DENORMAL_GUARD;
                vc[j].x1 = data;

                cvolt=y0+x0;


                ocvolt= cvolt*vcvo[j].n0 + vcvo[j].x1*vcvo[j].n1 - vcvo[j].y1*vcvo[j].d1;
                vcvo[j].y1 = ocvolt + DENORMAL_GUARD;
                vcvo[j].x1 = cvolt;

                oldcvolt[j] = ocvolt;

                evolt = input*vevo[j].n0 + vevo[j].x1*vevo[j].n1 - vevo[j].y1*vevo[j].d1;
                vevo[j].y1 = evolt + DENORMAL_GUARD;
                vevo[j].x1 = input;

                vin = 7.5f*(1.0f + ocvolt+evolt);
                if(vin<0.0f) vin = 0.0f;
                if(vin>15.0f) vin = 15.0f;
                vbe = 0.8f - 0.8f/(vin + 1.0f);  //really rough, simplistic bjt turn-on emulator
                input = vin - vbe;
                input = input*0.1333333333f -0.90588f;  //some magic numbers to return gain to unity & zero the DC

#else // Orig code, Comment below if instead using inline
            cvolt = vibefilter(input,ecvc,j) + vibefilter(input + emitterfb*oldcvolt[j],vc,j);
            ocvolt = vibefilter(cvolt,vcvo,j);
            oldcvolt[j] = ocvolt;
            evolt = vibefilter(input, vevo,j);

            input = bjt_shape(ocvolt + evolt);
#endif

        }
        fbl = fb*ocvolt;
        outl = lpanning*input;

        //Right channel

        if(Pstereo) {
#ifdef VIBE_INLINE_PROCESS
            //Inline BJT shaper
             vin = 7.5f*(1.0f + fbr+smpsr[i]);
             if(vin<0.0f) vin = 0.0f;
             if(vin>15.0f) vin = 15.0f;
             vbe = 0.8f - 0.8f/(vin + 1.0f);  //really rough, simplistic bjt turn-on emulator
             input = vin - vbe;
             input = input*0.1333333333f -0.90588f;  //some magic numbers to return gain to unity & zero the DC
#endif


//Orig code
            input = bjt_shape(fbr + smpsr[i]);
            //Close Comment here if using Inline instead

            emitterfb = 25.0f/fxr;
            for(j=4; j<8; j++) { //4 stages phasing

#ifdef VIBE_INLINE_PROCESS

                    float y0 = 0.0f;
                    y0 = input*ecvc[j].n0 + ecvc[j].x1*ecvc[j].n1 - ecvc[j].y1*ecvc[j].d1;
                    ecvc[j].y1 = y0 + DENORMAL_GUARD;
                    ecvc[j].x1 = input;


                    float x0 = 0.0f;
                    float data = input + emitterfb*oldcvolt[j];
                    x0 = data*vc[j].n0 + vc[j].x1*vc[j].n1 - vc[j].y1*vc[j].d1;
                    vc[j].y1 = x0 + DENORMAL_GUARD;
                    vc[j].x1 = data;

                    cvolt=y0+x0;


                    ocvolt= cvolt*vcvo[j].n0 + vcvo[j].x1*vcvo[j].n1 - vcvo[j].y1*vcvo[j].d1;
                    vcvo[j].y1 = ocvolt + DENORMAL_GUARD;
                    vcvo[j].x1 = cvolt;

                    oldcvolt[j] = ocvolt;

                    evolt = input*vevo[j].n0 + vevo[j].x1*vevo[j].n1 - vevo[j].y1*vevo[j].d1;
                    vevo[j].y1 = evolt + DENORMAL_GUARD;
                    vevo[j].x1 = input;


                    vin = 7.5f*(1.0f + ocvolt+evolt);
                    if(vin<0.0f) vin = 0.0f;
                    if(vin>15.0f) vin = 15.0f;
                    vbe = 0.8f - 0.8f/(vin + 1.0f);  //really rough, simplistic bjt turn-on emulator
                    input = vin - vbe;
                    input = input*0.1333333333f -0.90588f;  //some magic numbers to return gain to unity & zero the DC

#else
                cvolt = vibefilter(input,ecvc,j) + vibefilter(input + emitterfb*oldcvolt[j],vc,j);
                ocvolt = vibefilter(cvolt,vcvo,j);
                oldcvolt[j] = ocvolt;
                evolt = vibefilter(input, vevo,j);

                input = bjt_shape(ocvolt + evolt);
#endif
            }

            fbr = fb*ocvolt;
            outr = rpanning*input;

            efxoutl[i] = outl*fcross + outr*flrcross;
            efxoutr[i] = outr*fcross + outl*flrcross;
        }  else {  //if(Pstereo)
            efxoutl[i] = outl;
            efxoutr[i] = outl;
        }

    };

};

float
Vibe::vibefilter(float data, fparams *ftype, int stage) const
{
    float y0 = 0.0f;
    y0 = data*ftype[stage].n0 + ftype[stage].x1*ftype[stage].n1 - ftype[stage].y1*ftype[stage].d1;
    ftype[stage].y1 = y0 + DENORMAL_GUARD;
    ftype[stage].x1 = data;
    return y0;
};

float
Vibe::bjt_shape(float data) const
{
    float vbe, vout;
    float vin = 7.5f*(1.0f + data);
    if(vin<0.0f) vin = 0.0f;
    if(vin>15.0f) vin = 15.0f;
    vbe = 0.8f - 0.8f/(vin + 1.0f);  //really rough, simplistic bjt turn-on emulator
    vout = vin - vbe;
    vout = vout*0.1333333333f - 0.90588f;  //some magic numbers to return gain to unity & zero the DC
    return vout;
}

void
Vibe::init_vibes()
{
    k = 2.0f*fSAMPLE_RATE;
    float tmpgain = 1.0f;
    R1 = 4700.0f;
    Rv = 4700.0f;
    C2 = 1e-6f;
    beta = 150.0f;  //transistor forward gain.
    gain = -beta/(beta + 1.0f);

//Univibe cap values 0.015uF, 0.22uF, 470pF, and 0.0047uF
    C1[0] = 0.015e-6f;
    C1[1] = 0.22e-6f;
    C1[2] = 470e-12f;
    C1[3] = 0.0047e-6f;
    C1[4] = 0.015e-6f;
    C1[5] = 0.22e-6f;
    C1[6] = 470e-12f;
    C1[7] = 0.0047e-6f;

    for(int i =0; i<8; i++) {
//Vo/Ve driven from emitter
        en1[i] = k*R1*C1[i];
        en0[i] = 1.0f;
        ed1[i] = k*(R1 + Rv)*C1[i];
        ed0[i] = 1.0f + C1[i]/C2;

// Vc~=Ve/(Ic*Re*alpha^2) collector voltage from current input.
//Output here represents voltage at the collector

        cn1[i] = k*gain*Rv*C1[i];
        cn0[i] = gain*(1.0f + C1[i]/C2);
        cd1[i] = k*(R1 + Rv)*C1[i];
        cd0[i] = 1.0f + C1[i]/C2;

//Contribution from emitter load through passive filter network
        ecn1[i] = k*gain*R1*(R1 + Rv)*C1[i]*C2/(Rv*(C2 + C1[i]));
        ecn0[i] = 0.0f;
        ecd1[i] = k*(R1 + Rv)*C1[i]*C2/(C2 + C1[i]);
        ecd0[i] = 1.0f;

// %Represents Vo/Vc.  Output over collector voltage
        on1[i] = k*Rv*C2;
        on0[i] = 1.0f;
        od1[i] = k*Rv*C2;
        od0[i] = 1.0f + C2/C1[i];

//%Bilinear xform stuff
        tmpgain =  1.0f/(cd1[i] + cd0[i]);
        vc[i].n1 = tmpgain*(cn0[i] - cn1[i]);
        vc[i].n0 = tmpgain*(cn1[i] + cn0[i]);
        vc[i].d1 = tmpgain*(cd0[i] - cd1[i]);
        vc[i].d0 = 1.0f;

        tmpgain =  1.0f/(ecd1[i] + ecd0[i]);
        ecvc[i].n1 = tmpgain*(ecn0[i] - ecn1[i]);
        ecvc[i].n0 = tmpgain*(ecn1[i] + ecn0[i]);
        ecvc[i].d1 = tmpgain*(ecd0[i] - ecd1[i]);
        ecvc[i].d0 = 1.0f;

        tmpgain =  1.0f/(od1[i] + od0[i]);
        vcvo[i].n1 = tmpgain*(on0[i] - on1[i]);
        vcvo[i].n0 = tmpgain*(on1[i] + on0[i]);
        vcvo[i].d1 = tmpgain*(od0[i] - od1[i]);
        vcvo[i].d0 = 1.0f;

        tmpgain =  1.0f/(ed1[i] + ed0[i]);
        vevo[i].n1 = tmpgain*(en0[i] - en1[i]);
        vevo[i].n0 = tmpgain*(en1[i] + en0[i]);
        vevo[i].d1 = tmpgain*(ed0[i] - ed1[i]);
        vevo[i].d0 = 1.0f;

// bootstrap[i].n1
// bootstrap[i].n0
// bootstrap[i].d1
    }


};

void
Vibe::modulate(float ldrl, float ldrr)
{
    float tmpgain;
    float R1pRv;
    float C2pC1;
    Rv = 4700.0f + ldrl;
    R1pRv = R1 + Rv;


    for(int i =0; i<8; i++) {
        if(i==4) {
            Rv = 4700.0f + ldrr;
            R1pRv = R1 + Rv;
        }

        C2pC1 = C2 + C1[i];
//Vo/Ve driven from emitter
        ed1[i] = k*(R1pRv)*C1[i];
//ed1[i] = R1pRv*kC1[i];

// Vc~=Ve/(Ic*Re*alpha^2) collector voltage from current input.
//Output here represents voltage at the collector
        cn1[i] = k*gain*Rv*C1[i];
//cn1[i] = kgainCl[i]*Rv;
//cd1[i] = (R1pRv)*C1[i];
        cd1[i]=ed1[i];

//Contribution from emitter load through passive filter network
        ecn1[i] = k*gain*R1*cd1[i]*C2/(Rv*(C2pC1));
//ecn1[i] = iC2pC1[i]*kgainR1C2*cd1[i]/Rv;
        ecd1[i] = k*cd1[i]*C2/(C2pC1);
//ecd1[i] = iC2pC1[i]*k*cd1[i]*C2/(C2pC1);

// %Represents Vo/Vc.  Output over collector voltage
        on1[i] = k*Rv*C2;
        od1[i] = on1[i];

//%Bilinear xform stuff
        tmpgain =  1.0f/(cd1[i] + cd0[i]);
        vc[i].n1 = tmpgain*(cn0[i] - cn1[i]);
        vc[i].n0 = tmpgain*(cn1[i] + cn0[i]);
        vc[i].d1 = tmpgain*(cd0[i] - cd1[i]);

        tmpgain =  1.0f/(ecd1[i] + ecd0[i]);
        ecvc[i].n1 = tmpgain*(ecn0[i] - ecn1[i]);
        ecvc[i].n0 = tmpgain*(ecn1[i] + ecn0[i]);
        ecvc[i].d1 = tmpgain*(ecd0[i] - ecd1[i]);
        ecvc[i].d0 = 1.0f;

        tmpgain =  1.0f/(od1[i] + od0[i]);
        vcvo[i].n1 = tmpgain*(on0[i] - on1[i]);
        vcvo[i].n0 = tmpgain*(on1[i] + on0[i]);
        vcvo[i].d1 = tmpgain*(od0[i] - od1[i]);

        tmpgain =  1.0f/(ed1[i] + ed0[i]);
        vevo[i].n1 = tmpgain*(en0[i] - en1[i]);
        vevo[i].n0 = tmpgain*(en1[i] + en0[i]);
        vevo[i].d1 = tmpgain*(ed0[i] - ed1[i]);

    }


};

void
Vibe::setpanning (int value)
{
    Ppanning = value;
    rpanning = ((float)Ppanning) / 64.0f;
    lpanning = 2.0f - rpanning;
    lpanning = 10.0f * powf(lpanning, 4);
    rpanning = 10.0f * powf(rpanning, 4);
    lpanning = 1.0f - 1.0f/(lpanning + 1.0f);
    rpanning = 1.0f - 1.0f/(rpanning + 1.0f);
    lpanning *= 1.3f;
    rpanning *= 1.3f;
};


void
Vibe::setvolume (int value)
{
    Pvolume = value;
    outvolume = (float)Pvolume / 127.0f;
};



void
Vibe::setpreset (int npreset)
{
    const int PRESET_SIZE = 11;
    const int NUM_PRESETS = 8;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Classic
        {35, 120, 10, 0, 64, 64, 64, 64, 3, 64, 0},
        //Stereo Classic
        {35, 120, 10, 0, 48, 64, 64, 64, 3, 64, 1},
        //Wide Vibe
        {127, 80, 10, 0, 0, 64, 64, 64, 0, 64, 1},
        //Classic Chorus
        {35, 360, 10, 0, 48, 64, 0, 64, 3, 64, 0},
        //Vibe Chorus
        {75, 330, 10, 0, 50, 64, 0, 64, 17, 64, 0},
        //Lush Chorus
        {55, 260, 10, 0, 64, 70, 0, 49, 20, 48, 0},
        //Sick Phaser
        {110, 75, 10, 0, 32, 64, 64, 14, 0, 30, 1},
        //Warble
        {127, 360, 10, 0, 0, 64, 0, 0, 0, 37, 0}

    };

    if(npreset>NUM_PRESETS-1) {
        Fpre->ReadPreset(45,npreset-NUM_PRESETS+1, pdata);
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, pdata[n]);
    } else {
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, presets[npreset][n]);
    }

};

void
Vibe::changepar (int npar, int value)
{

    switch (npar) {

    case 0:
        Pwidth = value;
        fwidth = ((float) Pwidth)/90.0f;
        break;
    case 1:
        lfo->Pfreq = value;
        lfo->updateparams (PERIOD);
        break;
    case 2:
        lfo->Prandomness = value;
        lfo->updateparams (PERIOD);
        break;
    case 3:
        lfo->PLFOtype = value;
        lfo->updateparams (PERIOD);
        break;
    case 4:
        lfo->Pstereo = value;
        lfo->updateparams (PERIOD);
        break;
    case 5: // pan
        setpanning(value);
        break;
    case 6:
        setvolume(value);
        break;
    case 7: //fb
        Pfb = value;
        fb = ((float) (Pfb - 64))/65.0f;
        break;
    case 8: //depth
        Pdepth = value;
        fdepth = ((float) Pdepth)/127.0f;
        break;
    case 9: //lrcross
        Plrcross = value;
        flrcross = ((float) (Plrcross - 64))/64.0f;
        fcross = 1.0f - fabs(flrcross);
        break;
    case 10: //Stereo
        Pstereo = value;
        break;

    }

};

int
Vibe::getpar (int npar)
{

    switch (npar)

    {
    case 0:
        return (Pwidth);
        break;
    case 1:
        return (lfo->Pfreq);
        break;
    case 2:
        return (lfo->Prandomness);
        break;
    case 3:
        return (lfo->PLFOtype);
        break;
    case 4:
        return (lfo->Pstereo);
        break;
    case 5:
        return (Ppanning); //pan
        break;
    case 6:
        return(Pvolume);
        break;
    case 7:
        return(Pfb);
        break;
    case 8:
        return(Pdepth);
        break;
    case 9:
        return(Plrcross);
        break;
    case 10: //Stereo
        return(Pstereo);
        break;


    }

    return (0);

};


