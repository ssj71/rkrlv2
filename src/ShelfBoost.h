/*
  Rakarrack   Audio FX software
  ShelfBoost.h - Tone Booster
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

#ifndef SHELFBOOST_H
#define SHELFBOOST_H

#include "global.h"
#include "AnalogFilter.h"

class ShelfBoost
{
public:
    ShelfBoost (float * efxoutl_, float * efxoutr_, double sample_rate, uint32_t intermediate_bufsize);
    ~ShelfBoost ();
    void out (float * smpsl, float * smpr, uint32_t period);
    void setpreset (int npreset);
    void changepar (int npar, int value);
    int getpar (int npar);
    void cleanup ();


    int Ppreset;

    float outvolume;

    float *efxoutl;
    float *efxoutr;

private:

    void setvolume (int value);

    int Pvolume;
    int Plevel;
    int Pq1;
    int Pfreq1;
    int Pstereo;

    float freq1,q1,gain,u_gain;

    class AnalogFilter *RB1l, *RB1r;
    float* interpbuf; //buffer for filters

    class FPreset *Fpre;

};


#endif
