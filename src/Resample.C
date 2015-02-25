/*

  Resample.C  -  Class
  Using Erik de Castro Lopo libsamplerate
  Copyright (C) 2008-2009 Josep Andreu (Holborn)
  Author: Josep Andreu

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2) for more details.

  You should have received a copy of the GNU General Public License
(version2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Resample.h"


Resample::Resample(int type)
{
    statel = src_new(type , 1 , &errorl);
    stater = src_new(type , 1 , &errorr);

}


Resample::~Resample()
{
	src_delete(statel);
	src_delete(stater);
};

void
Resample::cleanup()
{
    src_reset(statel);
    src_reset(stater);

};



void
Resample::out(float *inl, float *inr, float *outl, float *outr, int frames, double ratio)
{


    long int o_frames = lrint((double)frames*ratio);
    srcinfol.data_in = inl;
    srcinfol.input_frames = frames;
    srcinfol.data_out = outl;
    srcinfol.output_frames = o_frames;
    srcinfol.src_ratio = ratio;
    srcinfol.end_of_input = 0;

    srcinfor.data_in = inr;
    srcinfor.input_frames = frames;
    srcinfor.data_out = outr;
    srcinfor.output_frames = o_frames;
    srcinfor.src_ratio = ratio;
    srcinfor.end_of_input = 0;

    errorl = src_process(statel, &srcinfol);
    errorr = src_process(stater, &srcinfor);


}


void
Resample::mono_out(float *inl, float *outl, int frames, double ratio, int o_frames)
{

    srcinfol.data_in = inl;
    srcinfol.input_frames = frames;
    srcinfol.data_out = outl;
    srcinfol.output_frames = o_frames;
    srcinfol.src_ratio = ratio;
    srcinfol.end_of_input = 0;

    errorl = src_process(statel, &srcinfol);

}




