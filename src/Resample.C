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


Resample::Resample(unsigned int quality, double from, double to)
{
	rl = new Resampler();
	rr = new Resampler();
	unsigned int a,b;
	a = lrint(from);
	b = lrint(to);
	rl->setup(a,b,1,quality+16);
	rr->setup(a,b,1,quality+16);
}


Resample::~Resample()
{
	delete rl;
	delete rr;
};

void
Resample::cleanup()
{
    rl->reset();
    rr->reset();
};



void
Resample::out(float *inl, float *inr, float *outl, float *outr, int i_frames, int o_frames)
{
	rl->inp_count = i_frames;
	rl->inp_data = inl;
	rl->out_data = outl;
	rl->out_count = o_frames;
	rl->process();

	rr->inp_count = i_frames;
	rr->inp_data = inl;
	rr->out_data = outl;
	rr->out_count = o_frames;
	rr->process();

}


void
Resample::mono_out(float *inl, float *outl, int i_frames, int o_frames)
{
	rl->inp_count = i_frames;
	rl->inp_data = inl;
	rl->out_data = outl;
	rl->out_count = o_frames;
	rl->process();
}
