/*
  rakarrack - a guitar efects software

  jack.C  -   jack I/O
  Copyright (C) 2008-2010 Josep Andreu
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

#include <jack/jack.h>
#include <jack/midiport.h>
#include <jack/transport.h>
#include "jack.h"
#include "global.h"


RKR *JackOUT;

jack_client_t *jackclient;
jack_port_t *outport_left, *outport_right;
jack_port_t *inputport_left, *inputport_right, *inputport_aux;
jack_port_t *jack_midi_in, *jack_midi_out;
void *dataout;
int jackprocess (jack_nframes_t nframes, void *arg);
#ifdef JACK_SESSION
jack_session_event_t *s_event;
void session_callback (jack_session_event_t *event, void *arg);
#endif

int
JACKstart (RKR * rkr_, jack_client_t * jackclient_)
{

    JackOUT = rkr_;
    jackclient = jackclient_;

    jack_set_sync_callback(jackclient, timebase, NULL);
    jack_set_process_callback (jackclient, jackprocess, 0);

#ifdef JACK_SESSION
    jack_set_session_callback (jackclient, session_callback, NULL);
#endif

    jack_on_shutdown (jackclient, jackshutdown, 0);



    inputport_left =
        jack_port_register (jackclient, "in_1", JACK_DEFAULT_AUDIO_TYPE,
                            JackPortIsInput, 0);
    inputport_right =
        jack_port_register (jackclient, "in_2", JACK_DEFAULT_AUDIO_TYPE,
                            JackPortIsInput, 0);

    inputport_aux =
        jack_port_register (jackclient, "aux", JACK_DEFAULT_AUDIO_TYPE,
                            JackPortIsInput, 0);

    outport_left =
        jack_port_register (jackclient, "out_1", JACK_DEFAULT_AUDIO_TYPE,
                            JackPortIsOutput, 0);
    outport_right =
        jack_port_register (jackclient, "out_2", JACK_DEFAULT_AUDIO_TYPE,
                            JackPortIsOutput, 0);

    jack_midi_in =
        jack_port_register(jackclient, "in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);

    jack_midi_out =
        jack_port_register(jackclient, "MC out", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);


    if (jack_activate (jackclient)) {
        fprintf (stderr, "Cannot activate jack client.\n");
        return (2);
    };

    if ((JackOUT->aconnect_JA) && (!needtoloadstate)) {

        for (int i = 0; i < JackOUT->cuan_jack; i += 2) {
            jack_connect (jackclient, jack_port_name (outport_left),
                          JackOUT->jack_po[i].name);
            jack_connect (jackclient, jack_port_name (outport_right),
                          JackOUT->jack_po[i + 1].name);
        }
    }

    if ((JackOUT->aconnect_JIA) && (!needtoloadstate)) {

        if(JackOUT->cuan_ijack == 1) {
            jack_connect (jackclient,JackOUT->jack_poi[0].name,jack_port_name(inputport_left));
            jack_connect (jackclient,JackOUT->jack_poi[0].name, jack_port_name(inputport_right));
        }

        else {
            for (int i = 0; i < JackOUT->cuan_ijack; i += 2) {
                jack_connect (jackclient,JackOUT->jack_poi[i].name, jack_port_name (inputport_left));
                jack_connect (jackclient,JackOUT->jack_poi[i + 1].name,jack_port_name (inputport_right));
            }
        }

    }



    return 3;

};



int
jackprocess (jack_nframes_t nframes, void *arg)
{

    int i,count;
    jack_midi_event_t midievent;
    jack_position_t pos;
    jack_transport_state_t astate;

    jack_default_audio_sample_t *outl = (jack_default_audio_sample_t *)
                                        jack_port_get_buffer (outport_left, nframes);
    jack_default_audio_sample_t *outr = (jack_default_audio_sample_t *)
                                        jack_port_get_buffer (outport_right, nframes);


    jack_default_audio_sample_t *inl = (jack_default_audio_sample_t *)
                                       jack_port_get_buffer (inputport_left, nframes);
    jack_default_audio_sample_t *inr = (jack_default_audio_sample_t *)
                                       jack_port_get_buffer (inputport_right, nframes);

    jack_default_audio_sample_t *aux = (jack_default_audio_sample_t *)
                                       jack_port_get_buffer (inputport_aux, nframes);


    JackOUT->cpuload = jack_cpu_load(jackclient);


    if((JackOUT->Tap_Bypass) && (JackOUT->Tap_Selection == 2)) {
        astate = jack_transport_query(jackclient, &pos);
        if(astate >0) {
            if (JackOUT->jt_tempo != pos.beats_per_minute)
                actualiza_tap(pos.beats_per_minute);
        }

        if(JackOUT->Looper_Bypass) {
            if((astate != JackOUT->jt_state) && (astate==0)) {
                JackOUT->jt_state=astate;
                JackOUT->efx_Looper->changepar(2,1);
                stecla=5;
            }

            if((astate != JackOUT->jt_state) && (astate == 3)) {
                JackOUT->jt_state=astate;
                JackOUT->efx_Looper->changepar(1,1);
                stecla=5;
            }

        }
    }



    int jnumpi = jack_port_connected(inputport_left) + jack_port_connected(inputport_right );
    if(jnumpi != JackOUT->numpi) {
        JackOUT->numpi=jnumpi;
        JackOUT->numpc = 1;
    }
    int jnumpo = jack_port_connected(outport_left) + jack_port_connected(outport_right );
    if(jnumpo != JackOUT->numpo) {
        JackOUT->numpo = jnumpo;
        JackOUT->numpc = 1;
    }
    int jnumpa = jack_port_connected(inputport_aux);
    if(jnumpa != JackOUT->numpa) {
        JackOUT->numpa = jnumpa;
        JackOUT->numpc = 1;
    }

    int jnumpmi = jack_port_connected(jack_midi_in);
    if(jnumpmi != JackOUT->numpmi) {
        JackOUT->numpmi = jnumpmi;
        JackOUT->numpc = 1;
    }

    int jnumpmo = jack_port_connected(jack_midi_out);
    if(jnumpmo != JackOUT->numpmo) {
        JackOUT->numpmo = jnumpmo;
        JackOUT->numpc = 1;
    }





    float *data = (float *)jack_port_get_buffer(jack_midi_in, nframes);
    count = jack_midi_get_event_count(data);

    dataout = jack_port_get_buffer(jack_midi_out, nframes);
    jack_midi_clear_buffer(dataout);


    for (i = 0; i < count; i++) {
        jack_midi_event_get(&midievent, data, i);
        JackOUT->jack_process_midievents(&midievent);
    }

    for (i=0; i<=JackOUT->efx_MIDIConverter->ev_count; i++) {
        jack_midi_event_write(dataout,
                              JackOUT->efx_MIDIConverter->Midi_event[i].time,
                              JackOUT->efx_MIDIConverter->Midi_event[i].dataloc,
                              JackOUT->efx_MIDIConverter->Midi_event[i].len);
    }

    JackOUT->efx_MIDIConverter->moutdatasize = 0;
    JackOUT->efx_MIDIConverter->ev_count = 0;



    memcpy (JackOUT->efxoutl, inl,
            sizeof (jack_default_audio_sample_t) * nframes);
    memcpy (JackOUT->efxoutr, inr,
            sizeof (jack_default_audio_sample_t) * nframes);
    memcpy (JackOUT->auxdata, aux,
            sizeof (jack_default_audio_sample_t) * nframes);




    JackOUT->Alg (JackOUT->efxoutl, JackOUT->efxoutr, inl, inr ,0);


    memcpy (outl, JackOUT->efxoutl,
            sizeof (jack_default_audio_sample_t) * nframes);
    memcpy (outr, JackOUT->efxoutr,
            sizeof (jack_default_audio_sample_t) * nframes);



    return 0;

};


void
JACKfinish ()
{

    jack_client_close (jackclient);
    usleep (1000);
};



void
jackshutdown (void *arg)
{
    if (gui == 0)
        printf ("Jack Shut Down, sorry.\n");
    else
        JackOUT->jshut=1;

};



int
timebase(jack_transport_state_t state, jack_position_t *pos, void *arg)
{

    JackOUT->jt_state=state;


    if((JackOUT->Tap_Bypass) && (JackOUT->Tap_Selection == 2)) {
        if((state > 0) && (pos->beats_per_minute > 0)) {
            JackOUT->jt_tempo=pos->beats_per_minute;
            JackOUT->Tap_TempoSet = lrint(JackOUT->jt_tempo);
            JackOUT->Update_tempo();
            JackOUT->Tap_Display=1;
            if((JackOUT->Looper_Bypass) && (state==3)) {
                JackOUT->efx_Looper->changepar(1,1);
                stecla=5;
            }
        }
    }

    return(1);

}

void
actualiza_tap(double val)
{
    JackOUT->jt_tempo=val;
    JackOUT->Tap_TempoSet = lrint(JackOUT->jt_tempo);
    JackOUT->Update_tempo();
    JackOUT->Tap_Display=1;
}


#ifdef JACK_SESSION
void session_callback(jack_session_event_t *event, void *arg)
{
    char filename[256];
    char command[256];

    s_event = event;
    snprintf( filename, sizeof(filename), "%srackstate.rkr", s_event->session_dir );
    snprintf( command, sizeof(command), "rakarrack -u %s ${SESSION_DIR}rackstate.rkr", s_event->client_uuid);


    s_event->command_line = strdup( command );
    jack_session_reply( jackclient, s_event );

    if (s_event->type == JackSessionSave) {
        JackOUT->savefile(filename);
    }



    if (s_event->type == JackSessionSaveAndQuit) {
        JackOUT->savefile(filename);
        needtoloadstate=1;
        Pexitprogram=1;
    }

    jack_session_event_free (s_event);

}

#endif




