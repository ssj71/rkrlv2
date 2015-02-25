/* tuneit.c -- Detect fundamental frequency of a sound
 * Copyright (C) 2004, 2005  Mario Lang <mlang@delysid.org>
 *
 * Modified for rakarrack by Daniel Vidal & Josep Andreu
 * tune.h  Tuner definitions
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software Foundation.
 * Please see the file COPYING for details.
 */



#ifndef TUNER_H_
#define TUNER_H_

#include <math.h>
#include <stdlib.h>


class Tuner
{
public:
    Tuner ();
    ~Tuner ();
    void schmittFloat (int nframes, float *indatal, float *indatar);

    int note;
    int preparada;
    int note_actual;
    int cents;
    signed short int *schmittBuffer;
    signed short int *schmittPointer;
    const char **notes;
    float nfreq, afreq, freq;
    float *efxoutl;
    float *efxoutr;

private:

    void displayFrequency (float freq);
    void schmittInit (int size);
    void schmittS16LE (int nframes, signed short int *indata);
    void schmittFree ();

    int blockSize;

};

#endif /*TUNER_H_ */
