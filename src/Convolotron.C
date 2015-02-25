/*

  Convolotron.C - Convolotron effect
  Author: Ryam Billing & Jospe Andreu

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
#include "Convolotron.h"

Convolotron::Convolotron (float * efxoutl_, float * efxoutr_,int DS, int uq, int dq)
{
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;

    //default values
    Ppreset = 0;
    Pvolume = 50;
    Ppanning = 64;
    Plrcross = 100;
    Psafe = 1;
    Phidamp = 60;
    Filenum = 0;
    Plength = 50;
    Puser = 0;
    real_len = 0;
    convlength = .5f;
    fb = 0.0f;
    feedback = 0.0f;
    adjust(DS);

    templ = (float *) malloc (sizeof (float) * period);
    tempr = (float *) malloc (sizeof (float) * period);

    maxx_size = (int) (nfSAMPLE_RATE * convlength);  //just to get the max memory allocated
    buf = (float *) malloc (sizeof (float) * maxx_size);
    rbuf = (float *) malloc (sizeof (float) * maxx_size);
    lxn = (float *) malloc (sizeof (float) * maxx_size);
    maxx_size--;
    offset = 0;
    M_Resample = new Resample(0);
    U_Resample = new Resample(dq);//Downsample, uses sinc interpolation for bandlimiting to avoid aliasing
    D_Resample = new Resample(uq);

    setpreset (Ppreset);
    cleanup ();
};

Convolotron::~Convolotron ()
{
};

/*
 * Cleanup the effect
 */
void
Convolotron::cleanup ()
{


};

void
Convolotron::adjust(int DS)
{

    DS_state=DS;


    switch(DS) {

    case 0:
        nPERIOD = period;
        nSAMPLE_RATE = SAMPLE_RATE;
        nfSAMPLE_RATE = fSAMPLE_RATE;
        break;

    case 1:
        nPERIOD = lrintf(fPERIOD*96000.0f/fSAMPLE_RATE);
        nSAMPLE_RATE = 96000;
        nfSAMPLE_RATE = 96000.0f;
        break;


    case 2:
        nPERIOD = lrintf(fPERIOD*48000.0f/fSAMPLE_RATE);
        nSAMPLE_RATE = 48000;
        nfSAMPLE_RATE = 48000.0f;
        break;

    case 3:
        nPERIOD = lrintf(fPERIOD*44100.0f/fSAMPLE_RATE);
        nSAMPLE_RATE = 44100;
        nfSAMPLE_RATE = 44100.0f;
        break;

    case 4:
        nPERIOD = lrintf(fPERIOD*32000.0f/fSAMPLE_RATE);
        nSAMPLE_RATE = 32000;
        nfSAMPLE_RATE = 32000.0f;
        break;

    case 5:
        nPERIOD = lrintf(fPERIOD*22050.0f/fSAMPLE_RATE);
        nSAMPLE_RATE = 22050;
        nfSAMPLE_RATE = 22050.0f;
        break;

    case 6:
        nPERIOD = lrintf(fPERIOD*16000.0f/fSAMPLE_RATE);
        nSAMPLE_RATE = 16000;
        nfSAMPLE_RATE = 16000.0f;
        break;

    case 7:
        nPERIOD = lrintf(fPERIOD*12000.0f/fSAMPLE_RATE);
        nSAMPLE_RATE = 12000;
        nfSAMPLE_RATE = 12000.0f;
        break;

    case 8:
        nPERIOD = lrintf(fPERIOD*8000.0f/fSAMPLE_RATE);
        nSAMPLE_RATE = 8000;
        nfSAMPLE_RATE = 8000.0f;
        break;

    case 9:
        nPERIOD = lrintf(fPERIOD*4000.0f/fSAMPLE_RATE);
        nSAMPLE_RATE = 4000;
        nfSAMPLE_RATE = 4000.0f;
        break;
    }
    u_up= (double)nPERIOD / (double)period;
    u_down= (double)period / (double)nPERIOD;
}





/*
 * Effect output
 */
void
Convolotron::out (float * smpsl, float * smpsr)
{
    int i, j, xindex;
    float l,lyn;

    if(DS_state != 0) {
        memcpy(templ, smpsl,sizeof(float)*period);
        memcpy(tempr, smpsr,sizeof(float)*period);
        U_Resample->out(templ,tempr,smpsl,smpsr,period,u_up);
    }


    for (i = 0; i < nPERIOD; i++) {

        l = smpsl[i] + smpsr[i] + feedback;
        oldl = l * hidamp + oldl * (alpha_hidamp);  //apply damping while I'm in the loop
        lxn[offset] = oldl;


        //Convolve left channel
        lyn = 0;
        xindex = offset;

        for (j =0; j<length; j++) {
            if (--xindex<0) xindex = maxx_size;		//length of lxn is maxx_size.
            lyn += buf[j] * lxn[xindex];		//this is all there is to convolution
        }

        feedback = fb * lyn;
        templ[i] = lyn * levpanl;
        tempr[i] = lyn * levpanr;

        if (++offset>maxx_size) offset = 0;


    };

    if(DS_state != 0) {
        D_Resample->out(templ,tempr,efxoutl,efxoutr,nPERIOD,u_down);

    } else {
        memcpy(efxoutl, templ,sizeof(float)*period);
        memcpy(efxoutr, tempr,sizeof(float)*period);
    }





};


/*
 * Parameter control
 */
void
Convolotron::setvolume (int Pvolume)
{
    this->Pvolume = Pvolume;
    outvolume = (float)Pvolume / 127.0f;
    if (Pvolume == 0)
        cleanup ();

};

void
Convolotron::setpanning (int Ppanning)
{
    this->Ppanning = Ppanning;
    lpanning = ((float)Ppanning + 0.5f) / 127.0f;
    rpanning = 1.0f - lpanning;
    levpanl=lpanning*level*2.0f;
    levpanr=rpanning*level*2.0f;

};

int
Convolotron::setfile(int value)
{

    double sr_ratio;

    offset = 0;
    maxx_read = maxx_size / 2;
    memset(buf,0,sizeof(float) * maxx_size);
    memset(rbuf,0,sizeof(float) * maxx_size);
    if(!Puser) {
        Filenum = value;
        memset(Filename,0, sizeof(Filename));
        sprintf(Filename, "%s/%d.wav",DATADIR,Filenum+1);
    }


    sfinfo.format = 0;
    if(!(infile = sf_open(Filename, SFM_READ, &sfinfo))) {
        real_len = 1;
        length = 1;
        rbuf[0] = 1.0f;
        process_rbuf();
        return(0);
    }

    if (sfinfo.frames > maxx_read) real_len = maxx_read;
    else real_len=sfinfo.frames;
    sf_seek (infile,0, SEEK_SET);
    sf_readf_float(infile,buf,real_len);
    sf_close(infile);




    if (sfinfo.samplerate != (int)nSAMPLE_RATE) {
        sr_ratio = (double)nSAMPLE_RATE/((double) sfinfo.samplerate);
        M_Resample->mono_out(buf,rbuf,real_len,sr_ratio,lrint((double)real_len*sr_ratio));
        real_len =lrintf((float)real_len*(float)sr_ratio);
    }

    else memcpy(rbuf,buf,real_len*sizeof(float));

    process_rbuf();

    return(1);
};


void
Convolotron::process_rbuf()
{
    int ii,j,N,N2;
    float tailfader, alpha, a0, a1, a2, Nm1p, Nm1pp, IRpowa, IRpowb, ngain, maxamp;
    memset(buf,0, sizeof(float)*real_len);

    if (length > real_len) length = real_len;
    /*Blackman Window function
    wn = a0 - a1*cos(2*pi*n/(N-1)) + a2 * cos(4*PI*n/(N-1)
    a0 = (1 - alpha)/2; a1 = 0.5; a2 = alpha/2
    */
    alpha = 0.16f;
    a0 = 0.5f*(1.0f - alpha);
    a1 = 0.5f;
    a2 = 0.5*alpha;
    N = length;
    N2 = length/2;
    Nm1p = D_PI/((float) (N - 1));
    Nm1pp = 4.0f * PI/((float) (N - 1));

    for(ii=0; ii<length; ii++) {
        if (ii<N2) {
            tailfader = 1.0f;
        } else {
            tailfader = a0 - a1*cosf(ii*Nm1p) + a2 * cosf(ii*Nm1pp);   //Calculate Blackman Window for right half of IR
        }

        buf[ii]= rbuf[ii] * tailfader;   //Apply window function

    }


    //memcpy(buf,rbuf,real_len*sizeof(float));

    IRpowa = IRpowb = maxamp = 0.0f;
    //compute IR signal power
    for(j=0; j<length; j++) {
        IRpowa += fabsf(rbuf[j]);
        if(maxamp < fabsf(buf[j])) maxamp = fabsf(buf[j]);   //find maximum level to normalize

        if(j < length) {
            IRpowb += fabsf(buf[j]);
        }

    }

    //if(maxamp < 0.3f) maxamp = 0.3f;
    ngain = IRpowa/IRpowb;
    if (ngain > maxx_read) ngain = maxx_read;
    for(j=0; j<length; j++) buf[j] *= ngain;

    if (Psafe) {
        impulse.resample_impulse(length, buf);
        length = 156;
        convlength = length/fSAMPLE_RATE;
    }
    /*
    //This section can be uncommented to make a text file you can plot
    //with something like gnuplot
    FILE * textfile;
    textfile = fopen("IR.txt", "w");
    if (textfile!=NULL)
    {
    for(j=0;j<length;j++) {
    fprintf (textfile, "%d\t%f\n",j,buf[j]);
    }
    fclose (textfile);
    }
    */



}

void
Convolotron::sethidamp (int Phidamp)
{
    this->Phidamp = Phidamp;
    hidamp = 1.0f - (float)Phidamp / 127.1f;
    alpha_hidamp = 1.0f - hidamp;
};

void
Convolotron::setpreset (int npreset)
{
    const int PRESET_SIZE = 11;
    const int NUM_PRESETS = 4;
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Convolotron 1
        {67, 64, 1, 100, 0, 64, 30, 20, 0, 0, 0},
        //Convolotron 2
        {67, 64, 1, 100, 0, 64, 30, 20, 1, 0, 0},
        //Convolotron 3
        {67, 75, 1, 100, 0, 64, 30, 20, 2, 0, 0},
        //Convolotron 4
        {67, 60, 1, 100, 0, 64, 30, 20, 3, 0, 0}
    };

    if(npreset>NUM_PRESETS-1) {
        Fpre->ReadPreset(29,npreset-NUM_PRESETS+1);
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, pdata[n]);
    } else {

        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, presets[npreset][n]);
    }
    Ppreset = npreset;
};

void
Convolotron::UpdateLength ()
{
    convlength = ((float) Plength)/1000.0f;                   //time in seconds
    length = (int) (nfSAMPLE_RATE * convlength);        //time in samples
    process_rbuf();
}

void
Convolotron::changepar (int npar, int value)
{
    switch (npar) {
    case 0:
        setvolume (value);
        break;
    case 1:
        setpanning (value);
        break;
    case 2:
        Psafe = value;
        UpdateLength();
        break;
    case 3:
        Plength = value;
        UpdateLength();
        break;
    case 8:
        if(!setfile(value)) error_num=1;
        UpdateLength();
        break;
    case 5:
        break;
    case 6:
        sethidamp (value);
        break;
    case 7:
        Plevel = value;
        level =  dB2rap (60.0f * (float)Plevel / 127.0f - 40.0f);
        levpanl=lpanning*level*2.0f;
        levpanr=rpanning*level*2.0f;
        break;
    case 4:
        Puser = value;
        break;
    case 9:
        break;
    case 10:
        Pfb = value;
        if(Pfb<0) {
            fb = (float) .1f*value/250.0f*.15f;
        } else {
            fb = (float) .1f*value/500.0f*.15f;
        }
        break;

    };
};

int
Convolotron::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
        return (Ppanning);
        break;
    case 2:
        return(Psafe);
        break;
    case 3:
        return(Plength);
        break;
    case 8:
        return (Filenum);
        break;
    case 5:
        return (0);
        break;
    case 6:
        return (Phidamp);
        break;
    case 7:
        return(Plevel);
        break;
    case 4:
        return(Puser);
        break;
    case 9:
        return(0);
        break;
    case 10:
        return(Pfb);
        break;

    };
    return (0);			//in case of bogus parameter number
};
