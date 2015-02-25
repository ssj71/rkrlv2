/*
  Rakarrack   Audio FX software
  Exciter.C - Harmonic Enhancer
  Based in Steve Harris LADSPA Plugin harmonic_gen
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
#include "Exciter.h"



Exciter::Exciter (float * efxoutl_, float * efxoutr_, double sample_rate, uint32_t intermediate_bufsize)
{
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;


    //default values
    Ppreset = 0;
    Pvolume = 50;
    lpffreq = 8000;
    hpffreq = 2500;

    for(int i=0; i<10; i++) {
        Prm[i]=0;
        rm[i]=0.0f;
    }

    harm = new HarmEnhancer (rm, 2500.0f,8000.0,1.0f, sample_rate, intermediate_bufsize);

    cleanup ();

    setpreset (Ppreset);
};

Exciter::~Exciter ()
{
	delete harm;
};

/*
 * Cleanup the effect
 */
void
Exciter::cleanup ()
{
    harm->cleanup ();
};


/*
 * Effect output
 */
void
Exciter::out (float * smpsl, float * smpsr, uint32_t period)
{
    harm->harm_out(smpsl,smpsr, period);

};


/*
 * Parameter control
 */
void
Exciter::setvolume (int value)
{
    Pvolume = value;
    outvolume = (float)Pvolume / 127.0f;
    harm->set_vol(0,outvolume);
};

void
Exciter::setlpf (int value)
{
    lpffreq=value;
    harm->set_freql (0, (float)value);
};

void
Exciter::sethpf (int value)
{
    hpffreq=value;
    harm->set_freqh (0,(float)value);
};

void
Exciter::sethar(int num,int value)
{
    float har=32.0f*((float)num+1.0f);
    if (num%2==1) har=11200.0f-64.0f*((float)num+1.0f);
    Prm[num]=value;
    rm[num]= (float)value/har;
    harm->calcula_mag(rm);
}



void
Exciter::setpreset (int npreset)
{
    const int PRESET_SIZE = 13;
    const int NUM_PRESETS = 5;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Plain
        {64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20000, 20},
        //Loudness
        {64, 0, 0, 0, 0, 24, 0, -24, 0, 24, 0, 80, 20},
        //Exciter 1
        {64, 64, 0, 0, 0, 64, 0, 0, 0, 64, 0, 20000, 20},
        //Exciter 2
        {64, 0, 0, 0, -32, 0, -32, -64, 0, 48, 0, 14100, 5660},
        //Exciter 3
        {64, 64, 0, -64, 0, 64, 0, -64, 0, 64, 0, 20000, 20}

    };

    if(npreset>NUM_PRESETS-1) {
        Fpre->ReadPreset(22,npreset-NUM_PRESETS+1, pdata);
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
Exciter::changepar (int npar, int value)
{
    switch (npar) {
    case 0:
        setvolume (value);
        break;
    case 1:
        sethar(0,value);
        break;
    case 2:
        sethar(1,value);
        break;
    case 3:
        sethar(2,value);
        break;
    case 4:
        sethar(3,value);
        break;
    case 5:
        sethar(4,value);
        break;
    case 6:
        sethar(5,value);
        break;
    case 7:
        sethar(6,value);
        break;
    case 8:
        sethar(7,value);
        break;
    case 9:
        sethar(8,value);
        break;
    case 10:
        sethar(9,value);
        break;
    case 11:
        setlpf(value);
        break;
    case 12:
        sethpf(value);
        break;

    };
};

int
Exciter::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
        return (Prm[0]);
        break;
    case 2:
        return (Prm[1]);
        break;
    case 3:
        return (Prm[2]);
        break;
    case 4:
        return (Prm[3]);
        break;
    case 5:
        return (Prm[4]);
        break;
    case 6:
        return (Prm[5]);
        break;
    case 7:
        return (Prm[6]);
        break;
    case 8:
        return (Prm[7]);
        break;
    case 9:
        return (Prm[8]);
        break;
    case 10:
        return (Prm[9]);
        break;
    case 11:
        return (lpffreq);
        break;
    case 12:
        return (hpffreq);
        break;
    };
    return (0);			//in case of bogus parameter number
};

