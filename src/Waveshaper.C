/*
  ZynAddSubFX - a software synthesizer

  Waveshaper.C - Distortion effect
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  Modified and evolved for rakarrack by Josep Andreu and Ryan Billing

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


#include <stdlib.h>
#include <math.h>
#include "Waveshaper.h"
#include "Resample.h"
#include "f_sin.h"

Waveshaper::Waveshaper(double samplerate, int Wave_Res_Amount, int Wave_up_q, int Wave_down_q, unsigned short tempbufsize)
{
double csamplerate = 1/samplerate;
	Wave_res_amount = Wave_Res_Amount;

    switch(Wave_res_amount) {
    case 0:
        period_coeff = 1;
        ncSAMPLE_RATE=csamplerate;
        break;
    case 1:
        period_coeff = 2;
        ncSAMPLE_RATE=csamplerate/2.0f;
        break;
    case 2:
        period_coeff = 4;
        ncSAMPLE_RATE=csamplerate/4.0f;
        break;
    case 3:
        period_coeff = 8;
        ncSAMPLE_RATE=csamplerate/8.0f;
        break;
    case 4:
        period_coeff = 12;
        ncSAMPLE_RATE=csamplerate/12.0f;
        break;
    }


    //create and zero temp buffer
    temps = (float *) malloc (sizeof (float) * tempbufsize * period_coeff);
    long i;
    for(i=0; i<tempbufsize*period_coeff; i++)
    {
    	temps[i] = 0;
    }
    u_up= (double)period_coeff;
    u_down = 1.0 / u_up;

    compg = 0.0f;  //used by compression distortion
    cratio = 0.25f;  //used by compression for hardness
    tmpgain = 1.0f;  // compression distortion temp variable
    dthresh = 0.25;
    dyno = 0.0f;
    dynodecay = 0.0167f/(ncSAMPLE_RATE + 0.0167f); //about 60Hz sub modulation from this

    Ip = 0.0f;
    Vsupp = 200.0f;
    Vgbias = 0.075f;  //bias point for Valve1 model
    R = 220000.0f; //Plate resistor, 220k
    P = 0.0002f;  //constant tuning bias current for Valve1

    mu = 100.0f;  //Valve2 gain
    V2bias = 1.5f;  //Valve2 bias voltage
    Is = 105/(R*powf(V2bias*mu, 1.5f));  //bias point for Valve2
    Vg2 = mu*V2bias;
    vfact = 12;   //adjustment of valve shut-off.  Small is hard clipping, large is softer clipping
    ffact = 40;  //Valve2 ffact and vfact set onset of significant limiting.   Small is hard clip, large is soft clip
    Vlv2out = 0.0f;
    V2dyno = 0.0f;

    Vmin = Vsupp - 2.5;  //Approximate cathode voltage when tube is saturated.
    Vfactor = 1.5f;
    Vdyno = 0.0f;

    U_Resample = new Resample(Wave_up_q);  //Downsample, uses sinc interpolation for bandlimiting to avoid aliasing
    D_Resample = new Resample(Wave_down_q);




};

Waveshaper::~Waveshaper()
{
	free(temps);
	delete U_Resample;
	delete D_Resample;
};
void Waveshaper::cleanup()
{
    compg = 0.0f;  //used by compression distortion
    cratio = 0.25f;  //used by compression for hardness
    tmpgain = 1.0f;  // compression distortion temp variable
    dthresh = 0.25;
    dyno = 0.0f;
    dynodecay = 0.0167f/(ncSAMPLE_RATE + 0.0167f); //about 60Hz sub modulation from this
}


void
Waveshaper::waveshapesmps (int n, float * smps, int type,
                           int drive, int eff)
{

    int nn=n;

    if(Wave_res_amount > 0) {
        nn=n*period_coeff;
        U_Resample->mono_out(smps,temps,n,u_up,nn);
    }
    else memcpy(temps,smps,sizeof(float)*n);

    int i;
    float ws = (float)drive / 127.0f + .00001f;
    ws = 1.0f - expf (-ws * 4.0f);
    float tmpv;
    float factor;

    switch (type + 1 ) {
    case 1:
        ws = powf (10.0f, ws * ws * 3.0f) - 1.0f + 0.001f;	//Arctangent
        for (i = 0; i < nn; i++)
            temps[i] = atanf (temps[i] * ws) / atanf (ws);
        break;
    case 2:
        ws = ws * ws * 32.0f + 0.0001f;	//Asymmetric
        if (ws < 1.0)
            tmpv = f_sin (ws) + 0.1f;
        else
            tmpv = 1.1f;
        for (i = 0; i < nn; i++) {
            temps[i] = f_sin (temps[i] * (0.1f + ws - ws * temps[i])) / tmpv;
        };
        break;
    case 3:
        ws = ws * ws * ws * 20.0f + 0.0001f;	//Pow
        for (i = 0; i < nn; i++) {
            temps[i] *= ws;
            if (fabsf (temps[i]) < 1.0) {
                temps[i] = (temps[i] - powf (temps[i], 3.0f)) * 3.0f;
                if (ws < 1.0)
                    temps[i] /= ws;
            } else
                temps[i] = 0.0;
        };
        break;
    case 4:
        ws = ws * ws * ws * 32.0f + 0.0001f;	//Sine
        if (ws < 1.57f)
            tmpv = f_sin (ws);
        else
            tmpv = 1.0f;
        for (i = 0; i < nn; i++)
            temps[i] = f_sin (temps[i] * ws) / tmpv;
        break;
    case 5:
        ws = ws * ws + 0.000001f;	//Quantisize
        for (i = 0; i < nn; i++)
            temps[i] = floorf (temps[i] / ws + 0.15f) * ws;
        break;
    case 6:
        ws = ws * ws * ws * 32.0f + 0.0001f;	//Zigzag
        if (ws < 1.0)
            tmpv = f_sin (ws);
        else
            tmpv = 1.0f;
        for (i = 0; i < nn; i++)
            temps[i] = asinf (f_sin (temps[i] * ws)) / tmpv;
        break;
    case 7:
        ws = powf (2.0f, -ws * ws * 8.0f);	//Limiter
        for (i = 0; i < nn; i++) {
            float tmp = temps[i];
            if (fabsf (tmp) > ws) {
                if (tmp >= 0.0)
                    temps[i] = 1.0f;
                else
                    temps[i] = -1.0f;
            } else
                temps[i] /= ws;
        };
        break;
    case 8:
        ws = powf (2.0f, -ws * ws * 8.0f);	//Upper Limiter
        for (i = 0; i < nn; i++) {
            float tmp = temps[i];
            if (tmp > ws)
                temps[i] = ws;
            temps[i] *= 2.0f;
        };
        break;
    case 9:
        ws = powf (2.0f, -ws * ws * 8.0f);	//Lower Limiter
        for (i = 0; i < nn; i++) {
            float tmp = temps[i];
            if (tmp < -ws)
                temps[i] = -ws;
            temps[i] *= 2.0f;
        };
        break;
    case 10:
        ws = (powf (2.0f, ws * 6.0f) - 1.0f) / powf (2.0f, 6.0f);	//Inverse Limiter
        for (i = 0; i < nn; i++) {
            float tmp = temps[i];
            if (fabsf (tmp) > ws) {
                if (tmp >= 0.0)
                    temps[i] = tmp - ws;
                else
                    temps[i] = tmp + ws;
            } else
                temps[i] = 0;
        };
        break;
    case 11:
        ws = powf (5.0f, ws * ws * 1.0f) - 1.0f;	//Clip
        for (i = 0; i < nn; i++)
            temps[i] =
                temps[i] * (ws + 0.5f) * 0.9999f - floorf (0.5f +
                        temps[i] * (ws +
                                    0.5f) * 0.9999f);
        break;
    case 12:
        ws = ws * ws * ws * 30.0f + 0.001f;	//Asym2
        if (ws < 0.3)
            tmpv = ws;
        else
            tmpv = 1.0f;
        for (i = 0; i < nn; i++) {
            float tmp = temps[i] * ws;
            if ((tmp > -2.0) && (tmp < 1.0))
                temps[i] = tmp * (1.0f - tmp) * (tmp + 2.0f) / tmpv;
            else
                temps[i] = 0.0f;
        };
        break;
    case 13:
        ws = ws * ws * ws * 32.0f + 0.0001f;	//Pow2
        if (ws < 1.0)
            tmpv = ws * (1.0f + ws) / 2.0f;
        else
            tmpv = 1.0f;
        for (i = 0; i < nn; i++) {
            float tmp = temps[i] * ws;
            if ((tmp > -1.0f) && (tmp < 1.618034f))
                temps[i] = tmp * (1.0f - tmp) / tmpv;
            else if (tmp > 0.0)
                temps[i] = -1.0f;
            else
                temps[i] = -2.0f;
        };
        break;
    case 14:
        ws = powf (ws, 5.0f) * 80.0f + 0.0001f;	//sigmoid
        if (ws > 10.0)
            tmpv = 0.5f;
        else
            tmpv = 0.5f - 1.0f / (expf (ws) + 1.0f);
        for (i = 0; i < nn; i++) {
            float tmp = temps[i] * ws;
            if (tmp < -10.0)
                tmp = -10.0f;
            else if (tmp > 10.0)
                tmp = 10.0f;
            tmp = 0.5f - 1.0f / (expf (tmp) + 1.0f);
            temps[i] = tmp / tmpv;
        };
        break;
    case 15:		//Sqrt "Crunch" -- Asymmetric square root distortion.
        ws = ws*ws*CRUNCH_GAIN + 1.0f;

        for (i = 0; i < nn; i++) {
            float tmp = temps[i] * ws;
            if (tmp < Tlo) {
                temps[i] = Tlc - sqrtf(-tmp*DIV_TLC_CONST);

            } else if (tmp > Thi) {
                temps[i] = Thc + sqrtf(tmp*DIV_THC_CONST);
            } else {
                temps[i] = tmp;
            };

            if (temps[i] < -1.0)
                temps[i] = -1.0f;
            else if (temps[i] > 1.0)
                temps[i] = 1.0f;

        };
        break;
    case 16:		//Sqrt "Crunch2" -- Asymmetric square root distortion.
        ws = ws*ws*CRUNCH_GAIN + 1.0f;

        for (i = 0; i < nn; i++) {
            float tmp = temps[i] * ws;
            if (tmp < Tlo) {
                temps[i] = Tlc;

            } else if (tmp > Thi) {
                temps[i] = Thc + sqrtf(tmp*DIV_THC_CONST);
            } else {
                temps[i] = tmp;
            };

            if (temps[i] < -1.0)
                temps[i] = -1.0f;
            else if (temps[i] > 1.0)
                temps[i] = 1.0f;
        };
        break;

    case 17:		//Octave Up
        ws = ws*ws*30.0f + 1.0f;

        for (i = 0; i < nn; i++) {
            float tmp = fabs(temps[i])* ws;
            if (tmp > 1.0f) {
                tmp = 1.0f;
            }

            temps[i] = tmp;		//a little bit of DC correction

        };
        break;
    case 18:
        ws = ws*D_PI+.00001;
        if (ws < 1.57f)
            tmpv = f_sin (ws);
        else
            tmpv = 1.0f;
        for (i = 0; i < nn; i++)
            temps[i]=f_sin(ws * temps[i] + f_sin( ws * 2.0f*temps[i]))/ tmpv;

        break;

    case 19:
        ws =  ws * D_PI + 0.0001f;
        if (ws < 1.57f)
            tmpv = f_sin (ws);
        else
            tmpv = 1.0f;
        for (i = 0; i < nn; i++)
            temps[i]=f_sin(ws * temps[i] + f_sin(ws * temps[i])/tmpv);
        break;

    case 20:  //Compression
        cratio = 1.0f - 0.25f * ws;
        ws =  1.5f*ws*CRUNCH_GAIN + 4.0f;
        for (i = 0; i < nn; i++) {  //apply compression
            tmpv = fabs(ws * temps[i]);
            dyno += 0.01f * (1.0f - dynodecay) * tmpv;
            dyno *= dynodecay;
            tmpv += dyno;

            if(tmpv > dthresh) {                              //if envelope of signal exceeds thresh, then compress
                compg = dthresh + dthresh*(tmpv - dthresh)/tmpv;
                dthresh = 0.25f + cratio*(compg - dthresh);   //dthresh changes dynamically

                if (temps[i] > 0.0f) {
                    temps[i] = compg;
                } else {
                    temps[i] = -1.0f * compg;
                }

            } else {
                temps[i] *= ws;
            }

            if(tmpv < dthresh) dthresh = tmpv;
            if(dthresh < 0.25f) dthresh = 0.25f;


        };
        break;

    case 21: //Overdrive
        ws = powf (10.0f, ws * ws * 3.0f) - 1.0f + 0.001f;
        for (i = 0; i < nn; i++) {
            if(temps[i]>0.0f) temps[i] = sqrtf(temps[i]*ws);
            else temps[i] = -sqrtf(-temps[i]*ws);
        }
        break;

    case 22: //Soft
        ws = powf(4.0f, ws*ws+1.0f);
        for (i = 0; i < nn; i++) {
            if(temps[i]>0.0f) temps[i] = ws*powf(temps[i],1.4142136f);
            else temps[i] = ws* -powf(-temps[i],1.4142136f);
        }
        break;

    case 23: //Super Soft

        ws = powf (20.0f, ws * ws) + 0.5f;
        factor = 1.0f / ws;
        for (i = 0; i < nn; i++) {
            if(temps[i] > 1.0) temps[i] = 1.0f;
            if(temps[i] < -1.0) temps[i] = -1.0f;

            if(temps[i]<factor) temps[i]=temps[i];
            else if(temps[i]>factor) temps[i]=factor+(temps[i]-factor)/powf(1.0f+((temps[i]-factor)/(1.0f-temps[i])),2.0f);
            else if(temps[i]>1.0f) temps[i]=(factor+1.0f)*.5f;
            temps[i]*=ws;
        }
        break;

    case 24:  // Hard Compression (used by stompboxes)
        cratio = 0.05;
        if (eff) {
            ws =  1.5f*ws*CRUNCH_GAIN + 1.0f;
        } else {
            ws = 1.0f;
        }           //allows functions applying gain before waveshaper

        for (i = 0; i < nn; i++) {  //apply compression
            tmpv = fabs(ws * temps[i]);

            if(tmpv > dthresh) {                              //if envelope of signal exceeds thresh, then compress
                compg = dthresh + dthresh*(tmpv - dthresh)/tmpv;
                dthresh = 0.5f + cratio*(compg - dthresh);   //dthresh changes dynamically

                if (temps[i] > 0.0f) {
                    temps[i] = compg;
                } else {
                    temps[i] = -1.0f * compg;
                }

            } else {
                temps[i] *= ws;
            }

            if(tmpv < dthresh) dthresh = tmpv;
            if(dthresh < 0.5f) dthresh = 0.5f;

        };
        break;

    case 25:  // Op Amp limiting (used by stompboxes), needs to get a large signal to do something
        cratio = 0.05;
        for (i = 0; i < nn; i++) {  //apply compression
            tmpv = fabs(temps[i]);

            if(tmpv > dthresh) {                              //if envelope of signal exceeds thresh, then compress
                compg = dthresh + dthresh*(tmpv - dthresh)/tmpv;
                dthresh = 3.5f + cratio*(compg - dthresh);   //dthresh changes dynamically

                if (temps[i] > 0.0f) {
                    temps[i] = compg;
                } else {
                    temps[i] = -1.0f * compg;
                }

            } else {
                temps[i] *= 1.0f;
            }

            if(tmpv < dthresh) dthresh = tmpv;
            if(dthresh < 3.5f) dthresh = 3.5f;

        };
        break;

    case 26: //JFET

        ws = powf (35.0f, ws * ws) + 4.0f;
        factor = sqrt(1.0f / ws);
        for (i = 0; i < nn; i++) {
            temps[i] = temps[i] + factor;
            if(temps[i] < 0.0) temps[i] = 0.0f;
            temps[i] = 1.0f - 2.0f/(ws*temps[i]*temps[i] + 1.0f);
        }
        break;

    case 27: //dyno JFET

        ws = powf (85.0f, ws * ws) + 10.0f;

        for (i = 0; i < nn; i++) {
            tmpv = fabs(temps[i]);
            if(tmpv > 0.15f) { // -16dB crossover distortion... dyno only picks up the peaks above 16dB.  Good for nasty fuzz
                dyno += (1.0f - dynodecay) * tmpv;
            }
            dyno *= dynodecay;  //always decays
            temps[i] = temps[i] + sqrtf((1.0f + 0.05f*dyno) / ws);
            if(temps[i] < 0.0) temps[i] = 0.0f;

            temps[i] = 1.0f - 2.0f/(ws*temps[i]*temps[i] + 1.0f);


        }
        break;

    case 28: //Valve 1

        ws = powf (4.0f, ws * ws) - 0.98f;

        for (i = 0; i < nn; i++) {
            Vg = Vgbias + ws*temps[i] - 0.1f*Vdyno;

            if(Vg<=0.05f) Vg = 0.05f/((-20.0f*Vg) + 2.0f);

            Ip = P*powf(Vg,Vfactor);
            tmpv = Vsupp - (Vmin - (Vmin/(R*Ip + 1.0f)));  //Here is the plate voltage
            tmpv = (tmpv - 106.243f)/100.0f;
            Vdyno += (1.0f - dynodecay) * tmpv;
            Vdyno *= dynodecay;
            temps[i] = tmpv;

        }
        break;

    case 29: //Valve 2

        ws = powf (110.0f, ws);

        for (i = 0; i < nn; i++) {

            Vg2 = mu*(V2bias + V2dyno + ws*temps[i]);

            if(Vg2 <= vfact) Vg2 = vfact/((-Vg2/vfact) + 2.0f);	 //Toward cut-off, behavior is a little different than 2/3 power law
            Vlv2out = Vsupp - R*Is*powf(Vg2,1.5f);   //2/3 power law relationship
            if(Vlv2out <= ffact) Vlv2out = ffact/((-Vlv2out/ffact) + 2.0f);  //Then as Vplate decreases, gain decreases until saturation

            temps[i] = (Vlv2out - 95.0f)*0.01f;
            V2dyno += (1.0f - dynodecay)*temps[i];
            V2dyno *= dynodecay;  //always decays

        }
        break;
    case 30: //Diode clipper

        ws = 5.0f + powf (110.0f, ws);

        for (i = 0; i < nn; i++) {

            tmpv = ws*temps[i];
            if (tmpv>0.0f) tmpv = 1.0f - 1.0f/powf(4.0f, tmpv);
            else tmpv = -(1.0f - 1.0f/powf(4.0f, -tmpv));
            temps[i] = tmpv;

        }
        break;


    };

    if(Wave_res_amount> 0) {
        D_Resample->mono_out(temps,smps,nn,u_down,n);
    } else
        memcpy(smps,temps,sizeof(float)*n);


};

