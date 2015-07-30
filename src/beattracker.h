/*
  Rakarrack Guitar FX

  beattracker.h - Detect beats and compute tempo
  Copyright (C) 2010 Ryan Billing
  Author: Ryan Billing & Josep Andreu

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 3 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#ifndef BEAT_H
#define BEAT_H

#include "global.h"
#include "RBFilter.h"


/*
Using beattracker:
Call detect() function on a set of samples.
The array, index[], returns a number of samples processed before finding the beat onset.

For example, if after processing 20 samples into the stream it finds a beat, then index[0] will have the number 20.
Then if it finds another beat at 67 samples into the stream, then index[1] will be 67.

So you loop through index[] until index[n] = 0.

These numbers may be used directly to index the onset of a beat in the audio stream.

index[] is initialized to the length of PERIOD, but it is unlikely that more than 1 beat per PERIOD will ever be found, so maybe
I need to change the design...for now it will do the job.

Next is the tempo calculator, but that code has not yet been started. The plan is:
call beattracker
call get_tempo() whenever you want the most current tempo calculation.

*/

class beattracker
{
public:
    beattracker (double sample_rate, uint32_t intermediate_bufsize);
    ~beattracker ();
    void cleanup ();
    void detect (float * smpsl, float * smpsr, uint32_t period);
    float get_tempo();  //returns tempo in float beats per minute
    int *index;

private:

    long timeseries[20];
    int tsidx;
    long tscntr;
//Variables for TrigStepper detecting trigger state.
    float peakpulse, peak, envrms, peakdecay, trigthresh;
    int trigtimeout, trigtime, onset, atk;
    float targatk, lmod, rmod;

    class RBFilter *rmsfilter, *peaklpfilter, *peakhpfilter, *peaklpfilter2;

    float oldbpm, oldmost;
    float avbpm[17], statsbin[17];
    int maxptr;
    int bpm_change_cntr;

    float fSAMPLE_RATE;
    float* interpbuf;//buffer for filters

    void calc_tempo();   //called by detect() on every beat detected
};


#endif
