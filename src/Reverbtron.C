/*

  Reverbtron.C - Reverbtron effect
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
#include "Reverbtron.h"

//TODO: make LV2 plugins deal with this correctly
#ifndef DATADIR
#define DATADIR "/usr/local/share/rakarrack"
#endif

Reverbtron::Reverbtron (float * efxoutl_, float * efxoutr_, double sample_rate,
		uint32_t intermediate_bufsize, int DS, int uq, int dq)
{
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;

    //default values
    Ppreset = 0;
    Pvolume = 50;
    Ppanning = 64;
    Plrcross = 100;
    Phidamp = 60;
    Filenum = 0;
    Llength = 50;
    Puser = 0;
    Psafe = 0;
    error = 0;
    convlength = 10.0f;  //max reverb time
    fb = 0.0f;
    feedback = 0.0f;
    adjust(DS, sample_rate);
    templ = (float *) malloc (sizeof (float) * intermediate_bufsize);
    tempr = (float *) malloc (sizeof (float) * intermediate_bufsize);

    hrtf_size = nSAMPLE_RATE/2;
    maxx_size = (int) (nfSAMPLE_RATE * convlength);  //just to get the max memory allocated
    time = (int *) malloc (sizeof (int) * 2000);
    rndtime = (int *) malloc (sizeof (int) * 2000);
    data = (float *) malloc (sizeof (float) * 2000);
    rnddata = (float *) malloc (sizeof (float) * 2000);
    lxn = (float *) malloc (sizeof (float) * (1 + maxx_size));
    hrtf =  (float *) malloc (sizeof (float) * (1 + hrtf_size));
    imax = nSAMPLE_RATE/2;  // 1/2 second available
    imdelay = (float *) malloc (sizeof (float) * imax);
    offset = 0;
    hoffset = 0;
    hlength = 0;
    fstretch = 1.0f;
    idelay = 0.0f;
    decay = f_exp(-1.0f/(0.2f*nfSAMPLE_RATE));  //0.2 seconds

    interpbuf = new float[intermediate_bufsize];
    lpfl =  new AnalogFilter (0, 800, 1, 0, sample_rate, interpbuf);
    lpfr =  new AnalogFilter (0, 800, 1, 0, sample_rate, interpbuf);

    lpfl->setSR(nSAMPLE_RATE);
    lpfr->setSR(nSAMPLE_RATE);


    U_Resample = new Resample(dq);  //Downsample, uses sinc interpolation for bandlimiting to avoid aliasing
    D_Resample = new Resample(uq);

    setpreset (Ppreset);
    cleanup ();
};

Reverbtron::~Reverbtron ()
{
    free(templ);
    free(tempr);

    free(time);
    free(rndtime);
    free(data);
    free(rnddata);
    free(lxn);
    free(hrtf);
    free(imdelay);

    delete[] interpbuf;
    delete lpfl;
    delete lpfr;

    delete U_Resample;
    delete D_Resample;
};

/*
 * Cleanup the effect
 */
void
Reverbtron::cleanup ()
{
    memset(lxn,0,sizeof(float)*(maxx_size+1));
    memset(hrtf,0,sizeof(float)*(hrtf_size+1));

    feedback = 0.0f;
    oldl = 0.0f;
    lpfl->cleanup ();
    lpfr->cleanup ();

};

/*
 * Effect output
 */
void
Reverbtron::out (float * smpsl, float * smpsr, uint32_t period)
{
    int  i, j, xindex, hindex;
    float l,lyn, hyn;
    float ldiff,rdiff;
    int length = Llength;
    hlength = Pdiff;
    int doffset;

    nPERIOD = lrintf((float)period*nRATIO);
    if(DS_state != 0) {
        memcpy(templ, smpsl,sizeof(float)*period);
        memcpy(tempr, smpsr,sizeof(float)*period);
        u_up= (double)nPERIOD / (double)period;
        u_down= (double)period / (double)nPERIOD;
        U_Resample->out(templ,tempr,smpsl,smpsr,period,u_up);
    }


    for (i = 0; i < nPERIOD; i++) {

        l = 0.5f*(smpsr[i] + smpsl[i]);
        oldl = l * hidamp + oldl * (alpha_hidamp);  //apply damping while I'm in the loop
        if(Prv) {
            oldl = 0.5f*oldl - smpsl[i];
        }

        lxn[offset] = oldl;

        //Convolve
        lyn = 0.0f;
        xindex = offset;

        for (j =0; j<length; j++) {
            xindex = offset + time[j];
            if(xindex>=maxx_size) xindex -= maxx_size;
            lyn += lxn[xindex] * data[j];		//this is all of the magic
        }

        hrtf[hoffset] = lyn;

        if(Pdiff > 0) {
            //Convolve again with approximated hrtf
            hyn = 0.0f;
            hindex = hoffset;

            for (j =0; j<hlength; j++) {
                hindex = hoffset + rndtime[j];
                if(hindex>=hrtf_size) hindex -= hrtf_size;
                hyn += hrtf[hindex] * rnddata[j];		//more magic
            }
            lyn = hyn + (1.0f - diffusion)*lyn;
        }

        if(Pes) { // just so I have the code to get started

            ldiff = lyn;
            rdiff = imdelay[imctr];

            ldiff = lpfl->filterout_s(ldiff);
            rdiff = lpfr->filterout_s(rdiff);

            imdelay[imctr] = decay*ldiff;
            imctr--;
            if (imctr<0) imctr = roomsize;

            templ[i] = (lyn + ldiff ) * levpanl;
            tempr[i] = (lyn + rdiff ) * levpanr;

            feedback = fb*rdiff*decay;

        } else {
            feedback = fb * lyn;
            templ[i] = lyn * levpanl;
            tempr[i] = lyn * levpanr;

        }

        offset--;
        if (offset<0) offset = maxx_size;

        doffset = (offset + roomsize);
        if (doffset>maxx_size) doffset -= maxx_size;

        hoffset--;
        if (hoffset<0) hoffset = hrtf_size;

        lxn[doffset] += feedback;

        xindex = offset + roomsize;

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
Reverbtron::setvolume (int Pvolume)
{
    this->Pvolume = Pvolume;
    outvolume = (float)Pvolume / 127.0f;
    if (Pvolume == 0)
        cleanup ();

};

void
Reverbtron::setpanning (int value)
{
    Ppanning = value;
    rpanning = ((float)Ppanning) / 64.0f;
    lpanning = 2.0f - rpanning;
    lpanning = 10.0f * powf(lpanning, 4);
    rpanning = 10.0f * powf(rpanning, 4);
    lpanning = 1.0f - 1.0f/(lpanning + 1.0f);
    rpanning = 1.0f - 1.0f/(rpanning + 1.0f);
    lpanning *= 1.1f;
    rpanning *= 1.1f;
    levpanl=level*lpanning;
    levpanr=level*rpanning;
};

//legacy
int
Reverbtron::setfile(int value)
{
    RvbFile filedata;

    if(!Puser) {
        Filenum = value;
        memset(Filename,0, sizeof(Filename));
        sprintf(Filename, "%s/%d.rvb",DATADIR,Filenum+1);//DATADIR comes from  config.h (autotools)
    }
    filedata = loadfile(Filename);
    applyfile(filedata);
    if(error)
    	return 0;
    return 1;

    /*
    if ((fs = fopen (Filename, "r")) == NULL) {
        File = loaddefault();
        cleanup();
        convert_time();
        return(0);
    }
    cleanup();
    memset(File.tdata, 0, sizeof(float)*2000);
    memset(File.ftime, 0, sizeof(float)*2000);


//Name
    memset(wbuf,0, sizeof(wbuf));
    fgets(wbuf,sizeof wbuf,fs);

// Subsample Compresion Skip
    memset(wbuf,0, sizeof(wbuf));
    fgets(wbuf,sizeof wbuf,fs);
    sscanf(wbuf,"%f,%f\n",&compresion,&quality);

//Length
    memset(wbuf,0,sizeof(wbuf));
    fgets(wbuf,sizeof wbuf,fs);
    sscanf(wbuf, "%d\n", &File.data_length);
    if(File.data_length>2000) File.data_length = 2000;
//Time Data
    for(i=0; i<File.data_length; i++) {
        memset(wbuf,0, sizeof(wbuf));
        fgets(wbuf,sizeof wbuf,fs);
        sscanf(wbuf,"%f,%f\n",&File.ftime[i],&File.tdata[i]);
    }

    fclose(fs);

    File.maxtime = 0.0f;
    File.maxdata = 0.0f;
    float averaget = 0.0f;
    float tempor = 0.0f;
    for(i=0; i<File.data_length; i++) {
        if(File.ftime[i] > File.maxtime) File.maxtime = File.ftime[i];
        if(File.tdata[i] > File.maxdata) File.maxdata = File.tdata[i];  //used to normalize so feedback is more predictable
        if(i>0) {
            tempor = File.ftime[i] - File.ftime[i-1];
            if(tempor>averaget) averaget = tempor;
        }
    }

    cleanup();
    convert_time();
    return(1);
    */
};

RvbFile
Reverbtron::loadfile(char* filename)
{
    int i;
    float compresion = 0.0f;
    float quality = 0.0f;
    char wbuf[128];

    FILE *fs;

    RvbFile f;
    error = 0;

    if ((fs = fopen (filename, "r")) == NULL) {
        f = loaddefault();
        error = 1;
        return(f);
    }
    strcpy(f.Filename,filename);
    memset(f.tdata, 0, sizeof(float)*2000);
    memset(f.ftime, 0, sizeof(float)*2000);


//Name
    memset(wbuf,0, sizeof(wbuf));
    if(fgets(wbuf,sizeof wbuf,fs) == NULL) {
        f = loaddefault();
        error = 1;
        return(f);
    }

// Subsample Compresion Skip
    memset(wbuf,0, sizeof(wbuf));
    if(fgets(wbuf,sizeof wbuf,fs) == NULL) {
        f = loaddefault();
        error = 1;
        return(f);
    }

    sscanf(wbuf,"%f,%f\n",&compresion,&quality);

//Length
    memset(wbuf,0,sizeof(wbuf));
    if (fgets(wbuf,sizeof wbuf,fs) == NULL) {
        f = loaddefault();
        error = 1;
        return(f);
    }
    sscanf(wbuf, "%d\n", &f.data_length);
    if(f.data_length>2000) f.data_length = 2000;
//Time Data
    for(i=0; i<f.data_length; i++) {
        memset(wbuf,0, sizeof(wbuf));
        if(fgets(wbuf,sizeof wbuf,fs) == NULL) {
            f = loaddefault();
            error = 1;
            return(f);
        }
        sscanf(wbuf,"%f,%f\n",&f.ftime[i],&f.tdata[i]);
    }

    fclose(fs);

    f.maxtime = 0.0f;
    f.maxdata = 0.0f;
    float averaget = 0.0f;
    float tempor = 0.0f;
    for(i=0; i<f.data_length; i++) {
        if(f.ftime[i] > f.maxtime) f.maxtime = f.ftime[i];
        if(f.tdata[i] > f.maxdata) f.maxdata = f.tdata[i];  //used to normalize so feedback is more predictable
        if(i>0) {
            tempor = f.ftime[i] - f.ftime[i-1];
            if(tempor>averaget) averaget = tempor;
        }
    }

    return f;
};

void
Reverbtron::applyfile(RvbFile file)
{
    cleanup();
    File = file;
    convert_time();
};

RvbFile Reverbtron::loaddefault()
{
	RvbFile f;
	strcpy(f.Filename,"default");
    f.data_length = Llength = 2;
    f.ftime[0] = 1.0f;
    f.ftime[1] = 1.25f;
    f.tdata[0] = 0.75f;
    f.tdata[1] = 0.5f;
    f.maxtime = 1.25f;
    f.maxdata = 0.75f;
    return f;
}

void Reverbtron::convert_time()
{
    int i;
    int index = 0;
    int count;
    float tmp;
    float skip = 0.0f;
    float incr = 0.0f;
    float findex;
    float tmpstretch = 1.0f;
    float normal = 0.9999f/File.maxdata;

    memset(data, 0, sizeof(float)*2000);
    memset(time, 0, sizeof(int)*2000);

    if(Llength>=File.data_length) Llength = File.data_length;
    if(Llength==0) Llength=400;
    incr = ((float) Llength)/((float) File.data_length);


    if(fstretch>0.0) {
        tmpstretch = 1.0f + fstretch * (convlength/File.maxtime);
    } else {
        tmpstretch = 1.0f + 0.95f*fstretch;
    }


    skip = 0.0f;
    index = 0;

    if(File.data_length>Llength) {
        for(i=0; i<File.data_length; i++) {
            skip += incr;
            findex = (float)index;
            if( findex<skip) {
                if(index<Llength) {
                    if( (tmpstretch*(idelay + File.ftime[i] )) > 9.9f ) {
                        File.ftime[i] = 0.0f;//TODO: why are we destroying the file data?
                        data[i] = 0.0f;
                    }
                    time[index]=lrintf(tmpstretch*(idelay + File.ftime[i])*nfSAMPLE_RATE);  //Add initial delay to all the samples
                    data[index]=normal*File.tdata[i];
                    index++;
                }
            }
        };
        Llength = index;
    } //endif
    else {
        for(i=0; i<File.data_length; i++) {

            if( (idelay + File.ftime[i] ) > 5.9f ) File.ftime[i] = 5.9f;
            time[i]=lrintf(tmpstretch*(idelay + File.ftime[i])*nfSAMPLE_RATE);  //Add initial delay to all the samples
            data[i]=normal*File.tdata[i];

        };
        Llength = i;
    }

//generate an approximated randomized hrtf for diffusing reflections:
    int tmptime = 0;
    int hrtf_tmp = Pdiff;
    if(hrtf_tmp>File.data_length) hrtf_tmp = File.data_length -1;
    if(hlength>File.data_length) hlength =  File.data_length -1;
    for (i =0; i<hrtf_tmp; i++) {
        tmptime = (int) (RND * hrtf_size);
        rndtime[i] = tmptime;  //randomly jumble the head of the transfer function
        rnddata[i] = 3.0f*(0.5f - RND)*data[tmptime];
    }

    if(Pfade > 0) {
        count = lrintf(ffade * ((float) index));
        tmp = 0.0f;
        for (i=0; i<count; i++) { //head fader

            tmp = ((float) i)/((float) count);
            data[i] *= tmp;
            //fade the head here
        }
    }

//guess at room size
    roomsize = time[0] + (time[1] - time[0])/2;  //to help stagger left/right reflection times
    if(roomsize>imax) roomsize = imax;
    setfb(Pfb);


};


void
Reverbtron::setlpf (int value)
{
    Plpf = value;
    float fr = (float)Plpf;
    lpfl->setfreq (fr);
    lpfr->setfreq (fr);

};



void
Reverbtron::sethidamp (int Phidamp)
{
    this->Phidamp = Phidamp;
    hidamp = 1.0f - (float)Phidamp / 127.1f;
    alpha_hidamp = 1.0f - hidamp;
};

void
Reverbtron::setfb(int value)
{

    if(Pfb<=0)
        fb = (float)value/64.0f * 0.3f;
    else
        fb = (float)value/64.0f * 0.15f;

    fb*=((1627.0f-(float)Pdiff-(float)Llength)/1627.0f);
    fb*=(1.0f-((float)Plevel/127.0f));
    fb*=(1.0f-diffusion)*.5f;

}


void
Reverbtron::adjust(int DS, double fSAMPLE_RATE)
{

    DS_state=DS;


    switch(DS) {

    case 0:
        nRATIO = 1;
        nSAMPLE_RATE = fSAMPLE_RATE;
        nfSAMPLE_RATE = fSAMPLE_RATE;
        break;

    case 1:
        nRATIO = 96000.0/fSAMPLE_RATE;
        nSAMPLE_RATE = 96000;
        nfSAMPLE_RATE = 96000.0f;
        break;


    case 2:
        nRATIO = 48000.0/fSAMPLE_RATE;
        nSAMPLE_RATE = 48000;
        nfSAMPLE_RATE = 48000.0f;
        break;

    case 3:
        nRATIO = 44100.0/fSAMPLE_RATE;
        nSAMPLE_RATE = 44100;
        nfSAMPLE_RATE = 44100.0f;
        break;

    case 4:
        nRATIO = 32000.0/fSAMPLE_RATE;
        nSAMPLE_RATE = 32000;
        nfSAMPLE_RATE = 32000.0f;
        break;

    case 5:
        nRATIO = 22050.0/fSAMPLE_RATE;
        nSAMPLE_RATE = 22050;
        nfSAMPLE_RATE = 22050.0f;
        break;

    case 6:
        nRATIO = 16000.0/fSAMPLE_RATE;
        nSAMPLE_RATE = 16000;
        nfSAMPLE_RATE = 16000.0f;
        break;

    case 7:
        nRATIO = 12000.0/fSAMPLE_RATE;
        nSAMPLE_RATE = 12000;
        nfSAMPLE_RATE = 12000.0f;
        break;

    case 8:
        nRATIO = 8000.0/fSAMPLE_RATE;
        nSAMPLE_RATE = 8000;
        nfSAMPLE_RATE = 8000.0f;
        break;

    case 9:
        nRATIO = 4000.0/fSAMPLE_RATE;
        nSAMPLE_RATE = 4000;
        nfSAMPLE_RATE = 4000.0f;
        break;
    }
}




void
Reverbtron::setpreset (int npreset)
{
    const int PRESET_SIZE = 16;
    const int NUM_PRESETS = 9;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Spring
        {64, 0, 1, 500, 0, 0, 99, 70, 0, 0, 0, 64, 0, 0, 20000, 0},
        //Concrete Stair
        {64, 0, 1, 500, 0, 0, 0, 40, 1, 0, 0, 64, 0, 0, 20000, 0},
        //Nice Hall
        {64, 0, 1, 500, 0, 0, 60, 15, 2, 0, 0, 64, 0, 0, 20000, 0},
        //Hall
        {64, 16, 1, 500, 0, 0, 0, 22, 3, -17, 0, 64, 0, 0, 20000, 0},
        //Room
        {64, 0, 1, 1500, 0, 0, 48, 20, 4, 0, 0, 64, 0, 0, 20000, 0},
        //Hall
        {88, 0, 1, 1500, 0, 0, 88, 14, 5, 0, 0, 64, 0, 0, 20000, 0},
        //Guitar
        {64, 0, 1, 1500, 0, 0, 30, 34, 6, 0, 0, 64, 0, 0, 20000, 0},
        //Studio
        {64, 0, 1, 1500, 0, 0, 30, 20, 7, 0, 0, 64, 0, 0, 20000, 0},
        //Cathedral
        {64, 0, 1, 1500, 0, 30, 0, 40, 9, 0, 0, 64, 0, 0, 20000, 0}

    };

    if(npreset>NUM_PRESETS-1) {
        Fpre->ReadPreset(40,npreset-NUM_PRESETS+1, pdata);
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, pdata[n]);
    } else {
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, presets[npreset][n]);
    }
    Ppreset = npreset;
};


void
Reverbtron::changepar (int npar, int value)
{
    switch (npar) {
    case 0:
        setvolume (value);
        break;
    case 1:
        Pfade=value;
        ffade = ((float) value)/127.0f;
        convert_time();
        break;
    case 2:
        Psafe=value;
        break;
    case 3:
        Llength = Plength = value;
        if((Psafe) && (Llength>400)) Llength = 400;
        convert_time();
        break;
    case 4:
        Puser = value;
        break;
    case 5:
        Pidelay = value;
        idelay = ((float) value)/1000.0f;
        convert_time();
        break;
    case 6:
        sethidamp (value);
        break;
    case 7:
        Plevel = value;
        level =  2.0f * dB2rap (60.0f * (float)Plevel / 127.0f - 40.0f);
        levpanl=level*lpanning;
        levpanr=level*rpanning;
        break;
    case 8:
        if(!setfile(value)) ;//error_num=2;//TODO: how to handle error
        break;
    case 9:
        Pstretch = value;
        fstretch = ((float) value)/64.0f;
        convert_time();
        break;
    case 10:
        Pfb = value;
        setfb(value);
        break;
    case 11:
        setpanning (value);
        break;
    case 12:
        Pes = value;
        break;
    case 13:
        Prv = value;
        break;
    case 14:
        setlpf (value);
        break;
    case 15:
        Pdiff=value;
        diffusion = ((float) value)/127.0f;
        convert_time();
        break;

    };
};

int
Reverbtron::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
        return (Pfade);
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
        return (Pidelay);
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
        return(Pstretch);
        break;
    case 10:
        return(Pfb);
        break;
    case 11:
        return(Ppanning);
        break;
    case 12:
        return(Pes);
        break;
    case 13:
        return(Prv);
        break;
    case 14:
        return(Plpf);
        break;
    case 15:
        return(Pdiff);
        break;


    };
    return (0);			//in case of bogus parameter number
};
