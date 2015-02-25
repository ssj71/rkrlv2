/*
  rakarrack - a guitar effects software

  RecChord.C  -  Recognize MIDI Chord
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


#include "RecChord.h"


RecChord::RecChord ()
{

    memset (Ch, 0, sizeof (Ch));


    Ch[0][0] = 1;
    Ch[6][0] = 1;
    Ch[12][0] = 1;
    Ch[18][0] = 1;
    Ch[24][0] = 1;
    Ch[31][0] = 1;
    Ch[0][2] = 2;
    Ch[6][2] = 1;
    Ch[12][2] = 1;
    Ch[18][2] = 2;
    Ch[24][2] = 2;
    Ch[31][7] = 1;
    Ch[0][4] = 1;
    Ch[6][4] = 1;
    Ch[12][3] = 1;
    Ch[18][3] = 1;
    Ch[24][4] = 1;
    Ch[0][7] = 1;
    Ch[6][7] = 1;
    Ch[12][5] = 2;
    Ch[18][5] = 2;
    Ch[24][7] = 1;
    Ch[0][9] = 2;
    Ch[6][9] = 1;
    Ch[12][7] = 1;
    Ch[18][6] = 1;
    Ch[24][9] = 1;
    Ch[30][0] = 1;
    Ch[6][11] = 2;
    Ch[18][8] = 2;
    Ch[24][10] = 1;
    Ch[18][9] = 1;
    Ch[18][11] = 2;
    Ch[1][0] = 1;
    Ch[7][0] = 1;
    Ch[13][0] = 1;
    Ch[19][0] = 1;
    Ch[25][0] = 1;
    Ch[32][0] = 1;
    Ch[1][2] = 2;
    Ch[7][2] = 2;
    Ch[13][2] = 1;
    Ch[19][4] = 1;
    Ch[25][1] = 1;
    Ch[32][2] = 2;
    Ch[1][4] = 1;
    Ch[7][4] = 1;
    Ch[13][3] = 1;
    Ch[19][7] = 1;
    Ch[25][4] = 1;
    Ch[32][5] = 1;
    Ch[1][7] = 1;
    Ch[7][6] = 2;
    Ch[13][5] = 2;
    Ch[19][10] = 1;
    Ch[25][7] = 1;
    Ch[32][7] = 1;
    Ch[1][9] = 1;
    Ch[7][8] = 1;
    Ch[13][7] = 1;
    Ch[25][10] = 1;
    Ch[1][11] = 2;
    Ch[7][10] = 2;
    Ch[13][10] = 1;


    Ch[2][0] = 1;
    Ch[8][0] = 1;
    Ch[14][0] = 1;
    Ch[20][0] = 1;
    Ch[26][0] = 1;
    Ch[33][0] = 1;
    Ch[2][2] = 2;
    Ch[8][2] = 2;
    Ch[14][2] = 2;
    Ch[20][5] = 1;
    Ch[26][1] = 2;
    Ch[33][2] = 1;
    Ch[2][4] = 1;
    Ch[8][3] = 1;
    Ch[14][3] = 1;
    Ch[20][7] = 1;
    Ch[26][4] = 1;
    Ch[33][4] = 2;
    Ch[2][7] = 1;
    Ch[8][5] = 2;
    Ch[14][5] = 1;
    Ch[20][10] = 1;
    Ch[26][5] = 2;
    Ch[33][7] = 1;
    Ch[2][9] = 2;
    Ch[8][7] = 1;
    Ch[14][7] = 1;
    Ch[26][7] = 1;
    Ch[33][9] = 2;
    Ch[2][11] = 1;
    Ch[14][10] = 1;
    Ch[26][8] = 1;
    Ch[26][10] = 1;
    Ch[3][0] = 1;
    Ch[9][0] = 1;
    Ch[15][0] = 1;
    Ch[21][0] = 1;
    Ch[27][0] = 1;
    Ch[3][2] = 2;
    Ch[9][2] = 2;
    Ch[15][2] = 2;
    Ch[21][2] = 2;
    Ch[27][1] = 2;
    Ch[3][4] = 1;
    Ch[9][3] = 1;
    Ch[15][3] = 1;
    Ch[21][4] = 1;
    Ch[27][3] = 1;
    Ch[3][6] = 1;
    Ch[9][5] = 2;
    Ch[15][5] = 2;
    Ch[21][6] = 1;
    Ch[27][4] = 1;
    Ch[3][7] = 1;
    Ch[9][7] = 1;
    Ch[15][7] = 1;
    Ch[21][10] = 1;
    Ch[27][6] = 2;
    Ch[3][9] = 2;
    Ch[9][9] = 1;
    Ch[15][11] = 1;
    Ch[27][7] = 1;
    Ch[3][11] = 1;
    Ch[27][9] = 2;
    Ch[27][10] = 1;
    Ch[4][0] = 1;
    Ch[10][0] = 1;
    Ch[16][0] = 1;
    Ch[22][0] = 1;
    Ch[28][0] = 1;
    Ch[4][2] = 1;
    Ch[10][3] = 1;
    Ch[16][2] = 1;
    Ch[22][2] = 1;
    Ch[28][2] = 2;
    Ch[4][4] = 1;
    Ch[10][5] = 2;
    Ch[16][3] = 1;
    Ch[22][4] = 1;
    Ch[28][4] = 1;
    Ch[4][7] = 1;
    Ch[10][7] = 1;
    Ch[16][5] = 2;
    Ch[22][7] = 1;
    Ch[28][5] = 2;
    Ch[4][9] = 2;
    Ch[10][10] = 1;
    Ch[16][7] = 1;
    Ch[22][9] = 2;
    Ch[28][8] = 1;
    Ch[16][11] = 1;
    Ch[22][10] = 1;
    Ch[28][9] = 2;
    Ch[28][11] = 1;
    Ch[5][0] = 1;
    Ch[11][0] = 1;
    Ch[17][0] = 1;
    Ch[23][0] = 1;
    Ch[29][0] = 1;
    Ch[5][2] = 1;
    Ch[11][1] = 2;
    Ch[17][3] = 1;
    Ch[23][2] = 2;
    Ch[29][2] = 2;
    Ch[5][4] = 1;
    Ch[11][3] = 1;
    Ch[17][5] = 2;
    Ch[23][4] = 1;
    Ch[29][4] = 1;
    Ch[5][7] = 1;
    Ch[11][5] = 2;
    Ch[17][6] = 1;
    Ch[23][6] = 1;
    Ch[29][6] = 2;
    Ch[5][9] = 2;
    Ch[11][6] = 1;
    Ch[17][8] = 2;
    Ch[23][7] = 1;
    Ch[29][8] = 1;
    Ch[5][11] = 1;
    Ch[11][8] = 2;
    Ch[23][9] = 2;
    Ch[29][10] = 1;
    Ch[11][10] = 1;
    Ch[23][10] = 1;



    memset (ChN, 0, sizeof (ChN));

    {
        static const char *stnom[] = {
            "", "6", "Maj7", "lyd", "Maj(9)", "Maj7(9)", "6/9", "+", "m",
            "m6", "m7", "m7(b5)", "m9", "m7(9)", "m7(11)", "m(Maj7)",
            "m(Maj7)(9)", "dim", "dim7", "7", "7(Sus4)", "7(b5)", "7(9)",
            "7(#11)", "7(13)", "7(b9)", "7(b13)", "7(#9)", "+Maj7", "+7",
            "1+8", "1+5", "(Sus4)", "2"
        };
        for (int i = 0; i <= 33; i++) {
            strcpy (ChN[i].Nom, stnom[i]);
            ChN[i].tipo = i;
        }
    }

    ChN[1].num = 1;
    ChN[6].num = 1;
    ChN[9].num = 1;
    ChN[32].num = 1;


    {
        static const char *stnom[] = {
            "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb",
            "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
        };

        for (int i = 0; i <= 11; i++)
            strcpy (NC[i].Nom, stnom[i]);
        for (int i = 0; i <= 23; i++)
            strcpy (NCE[i].Nom, stnom[i]);
    }
    NCE[0].note = 0;
    NCE[1].note = 1;
    NCE[2].note = 2;
    NCE[3].note = 3;
    NCE[4].note = 4;
    NCE[5].note = 5;
    NCE[6].note = 6;
    NCE[7].note = -5;
    NCE[8].note = -4;
    NCE[9].note = -3;
    NCE[10].note = -2;
    NCE[11].note = -1;
    NCE[12].note = 0;
    NCE[13].note = 1;
    NCE[14].note = 2;
    NCE[15].note = 3;
    NCE[16].note = 4;
    NCE[17].note = 5;
    NCE[18].note = 6;
    NCE[19].note = -5;
    NCE[20].note = -4;
    NCE[21].note = -3;
    NCE[22].note = -2;
    NCE[23].note = -1;


    IniciaChords ();
    memset (NombreAcorde, 0, sizeof (NombreAcorde));
}


RecChord::~RecChord()
{
}


void
RecChord::cleanup ()
{
    int i;
    memset (NombreAcorde, 0, sizeof (NombreAcorde));
    for (i = 0; i < POLY; i++) {
        note_active[i] = 0;
        rnote[i] = 0;
        gate[i] = 0;
    }

    cc = 1;

}

void
RecChord::IniciaChords ()
{
    int i, j;
    int notas = 1;
    int numno[6];

    NumChord3 = 0;
    NumChord4 = 0;
    NumChord5 = 0;

    memset (Chord3, 0, sizeof (Chord3));
    memset (Chord4, 0, sizeof (Chord4));
    memset (Chord5, 0, sizeof (Chord5));

    for (i = 0; i <= 33; i++) {
        notas = 1;
        memset (numno, 0, sizeof (numno));

        for (j = 1; j <= 11; j++) {
            if (Ch[i][j] == 1) {
                notas++;
                numno[notas] = j;
            }
        }



        switch (notas) {
        case 3:
            strcpy (Chord3[NumChord3].Nom, ChN[i].Nom);
            Chord3[NumChord3].di1 = numno[2];
            Chord3[NumChord3].di2 = numno[3] - numno[2];
            Chord3[NumChord3].fund = 1;
            Chord3[NumChord3].tipo = i;
            NumChord3++;
            if (ChN[i].num == 1)
                break;
            strcpy (Chord3[NumChord3].Nom, ChN[i].Nom);
            Chord3[NumChord3].di1 = numno[3] - numno[2];
            Chord3[NumChord3].di2 = 12 - numno[3];
            Chord3[NumChord3].fund = 3;
            Chord3[NumChord3].tipo = i;
            NumChord3++;
            strcpy (Chord3[NumChord3].Nom, ChN[i].Nom);
            Chord3[NumChord3].di1 = 12 - numno[3];
            Chord3[NumChord3].di2 = numno[2];
            Chord3[NumChord3].fund = 2;
            Chord3[NumChord3].tipo = i;
            NumChord3++;
            break;
        case 4:
            strcpy (Chord4[NumChord4].Nom, ChN[i].Nom);
            Chord4[NumChord4].di1 = numno[2];
            Chord4[NumChord4].di2 = numno[3] - numno[2];
            Chord4[NumChord4].di3 = numno[4] - numno[3];
            Chord4[NumChord4].fund = 1;
            Chord4[NumChord4].tipo = i;
            NumChord4++;
            if (ChN[i].num == 1)
                break;
            strcpy (Chord4[NumChord4].Nom, ChN[i].Nom);
            Chord4[NumChord4].di1 = numno[3] - numno[2];
            Chord4[NumChord4].di2 = numno[4] - numno[3];
            Chord4[NumChord4].di3 = 12 - numno[4];
            Chord4[NumChord4].fund = 4;
            Chord4[NumChord4].tipo = i;
            NumChord4++;
            strcpy (Chord4[NumChord4].Nom, ChN[i].Nom);
            Chord4[NumChord4].di1 = numno[4] - numno[3];
            Chord4[NumChord4].di2 = 12 - numno[4];
            Chord4[NumChord4].di3 = numno[2];
            Chord4[NumChord4].fund = 3;
            Chord4[NumChord4].tipo = i;
            NumChord4++;
            strcpy (Chord4[NumChord4].Nom, ChN[i].Nom);
            Chord4[NumChord4].di1 = 12 - numno[4];
            Chord4[NumChord4].di2 = numno[2];
            Chord4[NumChord4].di3 = numno[3] - numno[2];
            Chord4[NumChord4].fund = 2;
            Chord4[NumChord4].tipo = i;
            NumChord4++;
            strcpy (Chord4[NumChord4].Nom, ChN[i].Nom);
            Chord4[NumChord4].di1 = numno[2] + (12 - numno[4]);
            Chord4[NumChord4].di2 = numno[3] - numno[2];
            Chord4[NumChord4].di3 = 12 - numno[3];
            Chord4[NumChord4].fund = 4;
            Chord4[NumChord4].tipo = i;
            NumChord4++;
            break;

        case 5:
            strcpy (Chord5[NumChord5].Nom, ChN[i].Nom);
            Chord5[NumChord5].di1 = numno[2];
            Chord5[NumChord5].di2 = numno[3] - numno[2];
            Chord5[NumChord5].di3 = numno[4] - numno[3];
            Chord5[NumChord5].di4 = numno[5] - numno[4];
            Chord5[NumChord5].fund = 1;
            Chord5[NumChord5].tipo = i;
            NumChord5++;
            if (ChN[i].num == 1)
                break;
            strcpy (Chord5[NumChord5].Nom, ChN[i].Nom);
            Chord5[NumChord5].di1 = numno[3] - numno[2];
            Chord5[NumChord5].di2 = numno[4] - numno[3];
            Chord5[NumChord5].di3 = numno[5] - numno[4];
            Chord5[NumChord5].di4 = 12 - numno[5];

            Chord5[NumChord5].fund = 5;
            Chord5[NumChord5].tipo = i;
            NumChord5++;
            strcpy (Chord5[NumChord5].Nom, ChN[i].Nom);
            Chord5[NumChord5].di1 = numno[4] - numno[3];
            Chord5[NumChord5].di2 = numno[5] - numno[4];
            Chord5[NumChord5].di3 = 12 - numno[5];
            Chord5[NumChord5].di4 = numno[2];
            Chord5[NumChord5].fund = 4;
            Chord5[NumChord5].tipo = i;
            NumChord5++;
            strcpy (Chord5[NumChord5].Nom, ChN[i].Nom);
            Chord5[NumChord5].di1 = numno[5] - numno[4];
            Chord5[NumChord5].di2 = 12 - numno[5];
            Chord5[NumChord5].di3 = numno[2];
            Chord5[NumChord5].di4 = numno[3] - numno[2];
            Chord5[NumChord5].fund = 3;
            Chord5[NumChord5].tipo = i;
            NumChord5++;
            strcpy (Chord5[NumChord5].Nom, ChN[i].Nom);
            Chord5[NumChord5].di1 = 12 - numno[5];
            Chord5[NumChord5].di2 = numno[2];
            Chord5[NumChord5].di3 = numno[3] - numno[2];
            Chord5[NumChord5].di4 = numno[4] - numno[3];
            Chord5[NumChord5].fund = 2;
            Chord5[NumChord5].tipo = i;
            NumChord5++;
            strcpy (Chord5[NumChord5].Nom, ChN[i].Nom);
            Chord5[NumChord5].di1 = numno[2] + (12 - numno[5]);
            Chord5[NumChord5].di2 = numno[3] - numno[2];
            Chord5[NumChord5].di3 = numno[4] - numno[3];
            Chord5[NumChord5].di4 = 12 - numno[4];
            Chord5[NumChord5].fund = 5;
            Chord5[NumChord5].tipo = i;
            NumChord5++;
            break;

        }

    }



};



void
RecChord::MiraChord ()
{

    int i, j;
    int anote[POLY];
    int nnotes = 0;
    int temp;
    int di1, di2, di3, di4;
    int labaja;
    char AName[20];


    for (i = 0; i < POLY; i++) {
        if (note_active[i]) {
            anote[nnotes] = rnote[i];
            nnotes++;
        }
    }



    if ((nnotes < 3) || (nnotes > 5))
        return;

    labaja = anote[0];
    for (i = 1; i < nnotes - 1; i++)
        if (anote[i] < labaja)
            labaja = anote[i];
    for (i = 0; i < nnotes; i++)
        if (anote[i] - labaja > 12)
            anote[i] -= 12;
    for (i = 1; i < nnotes; i++) {
        for (j = 0; j < nnotes - 1; j++) {

            if (anote[j] > anote[j + 1]) {
                temp = anote[j];
                anote[j] = anote[j + 1];
                anote[j + 1] = temp;
            }
        }
    }



    bass = anote[0] % 12;

sigue:

    if (nnotes == 3) {

        di1 = anote[1] - anote[0];
        di2 = anote[2] - anote[1];

        j = -1;

        while (j <= NumChord3)

        {
            j++;
            if ((Chord3[j].di1 == di1) && (Chord3[j].di2 == di2)) {
                ctipo = Chord3[j].tipo;
                int elke = anote[Chord3[j].fund - 1];
                fundi = elke % 12;
                sprintf (AName, "%s%s", NC[fundi].Nom, Chord3[j].Nom);

                if (bass != fundi) {

                    memset (AName, 0, sizeof (AName));
                    sprintf (AName, "%s%s/%s", NC[fundi].Nom, Chord3[j].Nom,
                             NCE[bass + plus (fundi)].Nom);
                }

                if (strcmp (AName, NombreAcorde) != 0) {
                    strcpy (NombreAcorde, AName);
                    cc = 1;
                }
                return;
            }
        }

    }

    if (nnotes == 4) {

        di1 = anote[1] - anote[0];
        di2 = anote[2] - anote[1];
        di3 = anote[3] - anote[2];
        j = -1;

        while (j <= NumChord4)

        {
            j++;
            if ((Chord4[j].di1 == di1) && (Chord4[j].di2 == di2)
                    && (Chord4[j].di3 == di3)) {
                ctipo = Chord4[j].tipo;
                int elke = anote[Chord4[j].fund - 1];
                fundi = elke % 12;
                sprintf (AName, "%s%s", NC[fundi].Nom, Chord4[j].Nom);
                if (bass != fundi) {
                    memset (AName, 0, sizeof (AName));
                    sprintf (AName, "%s%s/%s", NC[fundi].Nom, Chord4[j].Nom,
                             NCE[bass + plus (fundi)].Nom);
                }

                if (strcmp (AName, NombreAcorde) != 0) {
                    strcpy (NombreAcorde, AName);
                    cc = 1;
                }
                return;
            }

        }



    }

    if (nnotes == 5) {

        di1 = anote[1] - anote[0];
        di2 = anote[2] - anote[1];
        di3 = anote[3] - anote[2];
        di4 = anote[4] - anote[3];
        j = -1;

        while (j < NumChord5)

        {
            j++;
            if ((Chord5[j].di1 == di1) && (Chord5[j].di2 == di2)
                    && (Chord5[j].di3 == di3) && (Chord5[j].di4 == di4)) {
                ctipo = Chord5[j].tipo;
                int elke = anote[Chord5[j].fund - 1];
                fundi = elke % 12;
                sprintf (AName, "%s%s", NC[fundi].Nom, Chord5[j].Nom);
                if (bass != fundi) {
                    memset (AName, 0, sizeof (AName));
                    sprintf (AName, "%s%s/%s", NC[fundi].Nom, Chord5[j].Nom,
                             NCE[bass + plus (fundi)].Nom);
                }

                if (strcmp (AName, NombreAcorde) != 0) {
                    strcpy (NombreAcorde, AName);
                    cc = 1;
                }
                return;

            }

        }

    }



    if (nnotes == 5) {
        bass = anote[0] % 12;

        for (i = 1; i <= 4; i++)
            anote[i - 1] = anote[i];
        nnotes = 4;
        goto sigue;
    }



    if (nnotes == 5) {
        nnotes = 4;
        goto sigue;
    }

    if (nnotes == 4) {
        nnotes = 3;
        goto sigue;
    }



};




int
RecChord::plus (int nota)
{
    switch (nota) {
    case 2:
        return (12);
        break;
    case 4:
        return (12);
        break;
    case 7:
        return (12);
        break;
    case 9:
        return (12);
        break;
    case 11:
        return (12);
        break;
    default:
        return (0);
    }

};


void
RecChord::Vamos (int voz, int interval, int reconota)
{

    int n1, h1;
    int hm1, hp1;
    int hm2, hp2;
    int hm3, hp3;
    int ninterval = interval;
    int tengo=0;
    int nota;
    int harmo;
    int typo;

    nota = reconota % 12;

    nota -= fundi;
    if (nota < 0)
        nota += 12;


    harmo = (12 + nota + interval) % 12;
    if (harmo > 12)
        harmo %= 12;



    hm1 = harmo - 1;
    if (hm1 < 0)
        hm1 += 12;
    hp1 = harmo + 1;
    if (hp1 > 12)
        hp1 %= 12;

    hm2 = harmo - 2;
    if (hm2 < 0)
        hm2 += 12;
    hp2 = harmo + 2;
    if (hp2 > 12)
        hp2 %= 12;

    hm3 = harmo - 3;
    if (hm3 < 0)
        hm3 += 12;
    hp3 = harmo + 3;
    if (hp3 > 12)
        hp3 %= 12;



    n1 = Ch[ctipo][nota];
    h1 = Ch[ctipo][harmo];

    typo = 3;

    if ((n1 == 1) && (h1 == 1))
        typo = 0;
    if ((n1 == 2) && (h1 == 2))
        typo = 1;
    if (h1 == 1)
        typo = 2;



    switch (typo)

    {
    case 0:
        ninterval = interval;
        break;
    case 1:
        ninterval = interval;
        break;
    case 2:
        ninterval = interval;
        break;
    case 3:
        tengo = 0;

        if ((Ch[ctipo][hp1] != 0) && (tengo == 0)) {
            ninterval = interval + 1;
            tengo = 1;
        }
        if ((Ch[ctipo][hm1] != 0) && (tengo == 0)) {
            ninterval = interval - 1;
            tengo = 1;
        }

        if ((Ch[ctipo][hp2] != 0) && (tengo == 0)) {
            ninterval = interval + 2;
            tengo = 1;
        }
        if ((Ch[ctipo][hm2] != 0) && (tengo == 0)) {
            ninterval = interval - 2;
            tengo = 1;
        }

        if ((Ch[ctipo][hp3] != 0) && (tengo == 0)) {
            ninterval = interval + 3;
            tengo = 1;
        }
        if ((Ch[ctipo][hm3] != 0) && (tengo == 0)) {
            ninterval = interval - 1;
            tengo = 1;
        }
        break;
    }

    if (ninterval < -12)
        ninterval += 12;
    if (ninterval > 12)
        ninterval %= 12;



    r__ratio[voz] = powf (2, ((float)ninterval / 12.0f));

}

