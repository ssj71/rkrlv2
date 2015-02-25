/*
  rakarrack - a guitar effects software

  RecChord.h  -  Chord Recognizer definitions
  Copyright (C) 2008 Josep Andreu
  Author: Josep Andreu

 This program is free software; you can redistribute it and/or modify
 it under the terms of version 2 of the GNU General Public License
 as published by the Free Software Foundation.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License (version 2) for more details.

 You should have received a copy of the GNU General Public License
 (version2)  along with this program; if not, write to the Free Software
 Foundation,
 Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#ifndef RECCHORD_H
#define RECCHORD_H

#include "global.h"

class RecChord
{

public:
    RecChord ();
    ~RecChord ();
    void MiraChord ();
    void IniciaChords ();
    void Vamos (int voz, int interval, int reconota);
    void cleanup ();

    int cc;
    int fundi;
    int bass;
    int ctipo;

    char NombreAcorde[20];

    struct TipoCh {
        int tipo;
        int num;
        char Nom[16];
    } ChN[34];

    struct NomChord {
        char Nom[3];
    } NC[13];

    struct NomChordExtended {
        char Nom[3];
        int note;
    } NCE[25];


    int note_active[POLY];
    int rnote[POLY];
    int gate[POLY];
    float r__ratio[POLY];

private:

    int plus (int nota);

    int NumChord3;
    int NumChord4;
    int NumChord5;

    int Ch[34][13];


    struct Ch3 {
        int tipo;
        int fund;
        int di1;
        int di2;
        char Nom[10];
    } Chord3[17];

    struct Ch4 {
        int tipo;
        int fund;
        int di1;
        int di2;
        int di3;
        char Nom[10];
    } Chord4[62];

    struct Ch5 {
        int tipo;
        int fund;
        int di1;
        int di2;
        int di3;
        int di4;
        char Nom[16];
    } Chord5[67];



};

#endif
