/* tuneit.c -- Detect fundamental frequency of a sound
 * Copyright (C) 2004, 2005  Mario Lang <mlang@delysid.org>
 *
 * Modified for rakarrack by Josep Andreu
 * MIDIConverter.C  MIDI Converter class
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software Foundation.
 * Please see the file COPYING for details.
 */
#include "MIDIConverter.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "global.h"


MIDIConverter::MIDIConverter (char *jname)
{

    velocity = 100;
    channel = 0;
    lanota = -1;
    preparada = 0;
    nota_actual = -1;
    TrigVal = .25f;
    hay = 0;
    ponla = 0;
    moutdatasize=0;
    ev_count=0;

    schmittBuffer = NULL;
    schmittPointer = NULL;
    static const char *englishNotes[12] =
    { "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#" };
    notes = englishNotes;
    note = 0;
    nfreq = 0;
    afreq = 0;
    schmittInit (32);


    // Open Alsa Seq


    int err = snd_seq_open (&port, "default", SND_SEQ_OPEN_OUTPUT, 0);
    if (err < 0)
        printf ("Cannot activate ALSA seq client\n");
    snd_seq_set_client_name (port, jname);
    snd_config_update_free_global ();



    char portname[50];

    // Create Alsa Seq Client

    sprintf (portname, "%s MC OUT",jname);
    snd_seq_create_simple_port (port, portname,
    SND_SEQ_PORT_CAP_READ |
    SND_SEQ_PORT_CAP_SUBS_READ,
    SND_SEQ_PORT_TYPE_APPLICATION);





};


MIDIConverter::~MIDIConverter ()
{
    snd_seq_close (port);
}


void
MIDIConverter::displayFrequency (float ffreq)
{
    int i;
    int noteoff = 0;
    int octave = 4;

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
    nfreq = freqs[note];
    while (nfreq / ffreq > D_NOTE_SQRT) {
        nfreq *= .5f;
        octave--;
        if (octave < -2) {
            noteoff = 1;
            break;
        }

    }
    while (ffreq / nfreq > D_NOTE_SQRT) {
        nfreq *= 2.0f;
        octave++;
        if (octave > 9) {
            noteoff = 1;
            break;
        }
    }


    cents = lrintf (1200.0f * (logf (ffreq / nfreq) / LOG_2));
    lanota = 24 + (octave * 12) + note - 3;


    if ((noteoff) & (hay)) {
        MIDI_Send_Note_Off (nota_actual);
        hay = 0;
        nota_actual = -1;
    }

    if ((preparada == lanota) && (lanota != nota_actual)) {

        hay = 1;
        if (nota_actual != -1) {
            MIDI_Send_Note_Off (nota_actual);
        }

        MIDI_Send_Note_On (lanota);
        nota_actual = lanota;
    }


    if ((lanota > 0 && lanota < 128) && (lanota != nota_actual))
        preparada = lanota;




};

void
MIDIConverter::schmittInit (int size)
{
    blockSize = SAMPLE_RATE / size;
    schmittBuffer =
    (signed short int *) malloc (blockSize * sizeof (signed short int));
    schmittPointer = schmittBuffer;
};



void
MIDIConverter::schmittS16LE (int nframes, signed short int *indata)
{
    int i, j;
    float trigfact = 0.6f;


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
            t2 = -lrintf ((float)A2 * trigfact + 0.5f);
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
MIDIConverter::schmittFree ()
{
    free (schmittBuffer);
};

void
MIDIConverter::schmittFloat (int nframes, float *indatal, float *indatar)
{
    int i;

    signed short int buf[nframes];
    for (i = 0; i < nframes; i++) {
        buf[i] =
        (short) ((TrigVal * indatal[i] + TrigVal * indatar[i]) * 32768);
    }
    schmittS16LE (nframes, buf);
};


void
MIDIConverter::MIDI_Send_Note_On (int nota)
{


    int k;
    int anota = nota + (Moctave * 12);
    if((anota<0) || (anota>127)) return;


    k = lrintf ((val_sum + 48) * 2);
    if ((k > 0) && (k < 127))
        velocity = lrintf((float)k * VelVal);

    if (velocity > 127)
        velocity = 127;
    if (velocity < 1)
        velocity = 1;


    snd_seq_event_t ev;
    snd_seq_ev_clear (&ev);
    snd_seq_ev_set_noteon (&ev,channel,anota,velocity);
    snd_seq_ev_set_subs (&ev);
    snd_seq_ev_set_direct (&ev);
    snd_seq_event_output_direct (port, &ev);

    ev_count++;
    Midi_event[ev_count].dataloc=&moutdata[moutdatasize];
    Midi_event[ev_count].time=0;
    Midi_event[ev_count].len=3;

    moutdata[moutdatasize]=144+channel;
    moutdatasize++;
    moutdata[moutdatasize]=anota;
    moutdatasize++;
    moutdata[moutdatasize]=velocity;
    moutdatasize++;



};


void
MIDIConverter::MIDI_Send_Note_Off (int nota)
{

    int anota = nota + ( Moctave * 12) ;
    if((anota<0) || (anota>127)) return;



    snd_seq_event_t ev;
    snd_seq_ev_clear (&ev);
    snd_seq_ev_set_noteoff (&ev, channel, anota, 0);
    snd_seq_ev_set_subs (&ev);
    snd_seq_ev_set_direct (&ev);
    snd_seq_event_output_direct (port, &ev);


    ev_count++;
    Midi_event[ev_count].dataloc=&moutdata[moutdatasize];
    Midi_event[ev_count].time=0;
    Midi_event[ev_count].len=2;

    moutdata[moutdatasize]=128+channel;
    moutdatasize++;
    moutdata[moutdatasize]=anota;
    moutdatasize++;

};


void
MIDIConverter::panic ()
{
    int i;

    for (i = 0; i < 127; i++)
        MIDI_Send_Note_Off (i);
    hay = 0;
    nota_actual = -1;
}


void
MIDIConverter::setmidichannel (int chan)
{
    channel = chan;

};


void
MIDIConverter::setTriggerAdjust (int val)
{

    TrigVal = 1.0f / (float)val;

};


void
MIDIConverter::setVelAdjust (int val)
{

    VelVal = 100.0f / (float)val;

};


