/*
  Rakarrack   Audio FX software
  Stompbox.h - stompbox modeler
  Using Steve Harris LADSPA Plugin harmonic_gen
  Modified for rakarrack by Ryan Billing & Josep Andreu

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

#ifndef STOMPBOX_H
#define STOMPBOX_H

#include "global.h"
#include "AnalogFilter.h"
#include "Waveshaper.h"

class StompBox
{
public:
    StompBox (float * efxoutl_, float * efxoutr_, double sample_rate, uint32_t intermediate_bufsize,
    		int wave_res, int wave_upq, int wave_dnq);
    ~StompBox ();
    void out (float * smpsl, float * smpr, uint32_t period);
    void setpreset (int npreset);
    void changepar (int npar, int value);
    int getpar (int npar);
    void cleanup ();


    int Ppreset;

    float *efxoutl;
    float *efxoutr;

private:

    void setvolume (int value);
    void init_mode (int value);
    void init_tone ();

    int Pvolume;
    int Pgain;
    int Phigh;
    int Pmid;
    int Plow;
    int Pmode;

    float gain, pre1gain, pre2gain, lowb, midb, highb, volume;
    float LG, MG, HG, RGP2, RGPST, pgain;

    float* interpbuf; //buffer for filters
    AnalogFilter *linput, *lpre1, *lpre2, *lpost, *ltonehg, *ltonemd, *ltonelw;
    AnalogFilter *rinput, *rpre1, *rpre2, *rpost, *rtonehg, *rtonemd, *rtonelw;
    AnalogFilter *ranti, *lanti;
    class Waveshaper *lwshape, *rwshape, *lwshape2, *rwshape2;

    class FPreset *Fpre;

};


#endif
