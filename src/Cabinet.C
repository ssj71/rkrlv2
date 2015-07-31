/*
  Rakarrack

  Cabinet.h - eq curves to approximate a guitar cab
  Copyright (C) 2015 Spencer Jackson
  Author: Spencer Jackson

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

//this really just wraps eq and has different presets

#include "Cabinet.h"

Cabinet::Cabinet(float * efxoutl_, float * efxoutr_, double sample_frequency, uint32_t intermediate_bufsize)
{
    eq = new EQ(efxoutl_, efxoutr_, sample_frequency, intermediate_bufsize);
    Cabinet_Preset = 0;
    eq->efxoutl = efxoutl;
    eq->efxoutr = efxoutr;
}


Cabinet::~Cabinet()
{
    delete eq;
}

void
Cabinet::cleanup()
{
	eq->cleanup();
}


void
Cabinet::out(float* smpsl, float* smpsr, uint32_t period)
{
    eq->efxoutl = efxoutl;
    eq->efxoutr = efxoutr;
    eq->out(smpsl,smpsr,period);
}

void
Cabinet::setpreset(int npreset)
{

    const int PRESET_SIZE = 81;
    const int NUM_PRESETS = 11;
    int presets[NUM_PRESETS][PRESET_SIZE] = {

        //Marshall-4-12
        {
            2, 1135, 0, 0, 0, 8, 116, 87, 71, 0, 7, 128, 77, 53, 0, 7, 825, 84, 64, 0,
            7, 1021, 49, 19, 0, 7, 2657, 75, 95, 0, 7, 3116, 72, 86, 0, 7, 10580, 31, 64, 0,
            7, 17068, 28, 68, 0, 3, 17068, 0, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0,
            67
        },


        //Celestion G12M
        {
            2, 704, 64, 64, 0, 7, 64, 90, 111, 0, 7, 151, 69, 59, 0, 7, 1021, 56, 25, 0,
            7, 1562, 56, 62, 0, 7, 2389, 71, 77, 0, 7, 5896, 53, 69, 0, 7, 6916, 59, 83, 0,
            7, 7691, 52, 102, 0, 7, 9021, 43, 64, 0, 7, 15347, 32, 59, 0, 3, 17068, 1, 58, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 67
        },


        //Jensen Alnico P12-N
        {
            2, 270, 0, 0, 0, 7, 414, 46, 56, 0, 7, 1647, 46, 84, 0, 7, 7293, 53, 69, 0,
            7, 10032, 43, 109, 0, 7, 12408, 41, 86, 0, 7, 14553, 41, 90, 0, 7, 17068, 35, 27, 0,
            3, 17068, 64, 61, 0, 7, 4065, 71, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 67
        },

        //Jensen Alnico P15-N
        {
            0, 600, 64, 64, 0, 2, 256, 64, 64, 0, 7, 414, 49, 43, 0, 7, 1832, 68, 58, 0,
            7, 4065, 46, 92, 0, 7, 8111, 52, 75, 0, 7, 11766, 43, 83, 0, 3, 15347, 64, 41, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 67
        },

        //Eminence Delta Demon
        {
            2, 68, 64, 64, 0, 7, 600, 55, 43, 0, 7, 1021, 72, 72, 0, 7, 1562, 68, 92, 0,
            7, 2519, 66, 80, 0, 7, 6218, 52, 66, 0, 7, 9513, 43, 81, 0, 3, 10580, 64, 66, 0,
            7, 12408, 58, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 67
        },

        //Celestion EVH12
        {
            2, 151, 64, 64, 0, 7, 1481, 56, 64, 0, 7, 2519, 66, 87, 0, 7, 3116, 66, 92, 0,
            7, 7293, 53, 87, 0, 7, 8554, 52, 87, 0, 7, 11157, 46, 59, 0, 3, 17068, 61, 30, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 67
        },

        //Eminence Copperhead

        {
            2, 301, 64, 64, 0, 7, 301, 58, 77, 0, 7, 1077, 68, 74, 0, 7, 2519, 71, 77, 0,
            7, 6558, 55, 87, 0, 7, 7293, 58, 37, 0, 7, 15347, 13, 75, 0, 3, 17068, 50, 43, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 67
        },

        //Mesa Boogie
        {
            2, 600, 64, 64, 0, 7, 128, 93, 52, 0, 7, 633, 58, 80, 0, 7, 1077, 52, 89, 0,
            7, 1647, 66, 89, 0, 7, 2037, 75, 86, 0, 7, 3466, 75, 90, 0, 7, 6218, 62, 52, 0,
            7, 11157, 58, 71, 0, 3, 1404, 64, 47, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 67
        },

        //Jazz-Chorus
        {
            4, 72, 0, 40, 0, 7, 72, 50, 43, 0, 7, 667, 38, 89, 0, 7, 3466, 77, 112, 0,
            7, 7293, 46, 93, 0, 7, 8111, 43, 105, 0, 7, 17068, 15, 40, 0, 3, 17068, 58, 55, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 67
        },

        //Vox-Bright
        {
            2, 1021, 64, 64, 0, 7, 243, 68, 72, 0, 7, 2657, 75, 41, 0, 7, 3466, 69, 96, 0,
            7, 4767, 74, 74, 0, 7, 6218, 44, 81, 0, 7, 8554, 52, 100, 0, 7, 13086, 46, 72, 0,
            7, 15347, 52, 62, 0, 3, 13800, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 67
        },

        //Marshall-I
        {
            2, 4287, 53, 64, 0, 7, 122, 80, 25, 0, 7, 633, 69, 86, 0, 7, 1021, 78, 59, 0,
            7, 1647, 75, 64, 0, 7, 2389, 86, 78, 0, 7, 3286, 95, 61, 0, 7, 6916, 61, 59, 0,
            7, 8554, 56, 84, 0, 7, 12408, 22, 18, 0, 3, 10032, 64, 61, 0, 0, 64, 64, 64, 0,
            0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 0, 64, 64, 64, 0, 67
        }


    };


    if (npreset > (NUM_PRESETS -1))
        npreset = 0;
    for (int n = 0; n < 16; n++) {
        eq->changepar (n * 5 + 10, presets[npreset][n * 5]);
        eq->changepar (n * 5 + 11, presets[npreset][n * 5 + 1]);
        eq->changepar (n * 5 + 12, presets[npreset][n * 5 + 2]);
        eq->changepar (n * 5 + 13, presets[npreset][n * 5 + 3]);
        eq->changepar (n * 5 + 14, presets[npreset][n * 5 + 4]);

    }

    Cabinet_Preset = npreset;
};

void 
Cabinet::changepar(int i, int val)
{
    if(i==0)
    {
        eq->changepar(0,val);
    }
}

int 
Cabinet::getpar(int i)
{
    if(i==0)
    {
        return eq->getpar(0);
    }
    return 0;
}

