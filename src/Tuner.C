/* tuneit.c -- Detect fundamental frequency of a sound
 * Copyright (C) 2004, 2005  Mario Lang <mlang@delysid.org>
 *
 * Modified for rakarrack by Daniel Vidal & Josep Andreu
 * Tuner.C Tuner class
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software Foundation.
 * Please see the file COPYING for details.
 */
#include "Tuner.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "global.h"


Tuner::Tuner ()
{

    schmittBuffer = NULL;
    schmittPointer = NULL;
    static const char *englishNotes[12] =
    { "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#" };
    preparada = -1;
    note_actual = 0;
    notes = englishNotes;
    note = 0;
    nfreq = 0;
    afreq = 0;
    schmittInit (2);

};



Tuner::~Tuner ()
{
}




void
Tuner::displayFrequency (float ffreq)
{
    int i;

    float ldf, mldf;
    float lfreq;

    if (ffreq < 1E-15)
        ffreq = 1E-15f;
    lfreq = logf (ffreq);
    while (lfreq < lfreqs[0] - LOG_D_NOTE * .5f)
        lfreq += LOG_2;
    while (lfreq >= lfreqs[0] + LOG_2 - LOG_D_NOTE * .5f)
        lfreq -= LOG_2;
    mldf = LOG_D_NOTE;
    for (i = 0; i < 12; i++) {
        ldf = fabsf (lfreq - lfreqs[i]);
        if (ldf < mldf) {
            mldf = ldf;
            note = i;
        }
    }

    if (preparada == note) {
        note_actual = note;
        nfreq = freqs[note];
        while (nfreq / ffreq > D_NOTE_SQRT)
            nfreq *= .5f;
        while (ffreq / nfreq > D_NOTE_SQRT)
            nfreq *= 2.0f;
        cents = lrintf (1200 * (logf (ffreq / nfreq) / LOG_2));
    }

    preparada = note;

};

void
Tuner::schmittInit (int size)
{
    blockSize = SAMPLE_RATE / size;
    schmittBuffer =
    (signed short int *) malloc (blockSize * sizeof (signed short int));
    schmittPointer = schmittBuffer;
};



void
Tuner::schmittS16LE (int nframes, signed short int *indata)
{
    int i, j;
    float trigfact = 0.5f;

    for (i = 0; i < nframes; i++) {
        *schmittPointer++ = indata[i];
        if (schmittPointer - schmittBuffer >= blockSize) {
            int endpoint, startpoint, t1, t2, A1, A2, tc, schmittTriggered;

            schmittPointer = schmittBuffer;

            for (j = 0, A1 = 0, A2 = 0; j < blockSize; j++) {
                if (schmittBuffer[j] > 0 && A1 < schmittBuffer[j])
                    A1 = schmittBuffer[j];
                if (schmittBuffer[j] < 0 && A2 < -schmittBuffer[j])
                    A2 = -schmittBuffer[j];
            }
            t1 = lrintf ((float)A1 * trigfact + 0.5f);
            t2 = - lrintf ((float)A2 * trigfact + 0.5f);
            startpoint = 0;
            for (j = 1; schmittBuffer[j] <= t1 && j < blockSize; j++);
            for (; !(schmittBuffer[j] >= t2 &&
            schmittBuffer[j + 1] < t2) && j < blockSize; j++);
            startpoint = j;
            schmittTriggered = 0;
            endpoint = startpoint + 1;
            for (j = startpoint, tc = 0; j < blockSize; j++) {
                if (!schmittTriggered) {
                    schmittTriggered = (schmittBuffer[j] >= t1);
                } else if (schmittBuffer[j] >= t2 && schmittBuffer[j + 1] < t2) {
                    endpoint = j;
                    tc++;
                    schmittTriggered = 0;
                }
            }
            if (endpoint > startpoint) {
                afreq =
                fSAMPLE_RATE *((float)tc / (float) (endpoint - startpoint));
                displayFrequency (afreq);

            }
        }
    }
};

void
Tuner::schmittFree ()
{
    free (schmittBuffer);
};

void
Tuner::schmittFloat (int nframes, float *indatal, float *indatar)
{
    int i;

    signed short int buf[nframes];
    for (i = 0; i < nframes; i++) {
        buf[i] = (short) ((indatal[i] + indatar[i]) * 32768);
    }
    schmittS16LE (nframes, buf);
};

