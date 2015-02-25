/*  rakarrack - Audio effects software

  fileio.C  -  File Input/Output functions
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
 (version2)  along with this program; if not, write to the Free Software
 Foundation,
 Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#include <errno.h>
#include "global.h"

void RKR::putbuf(char *buf, int j)
{
    char *cfilename;
    cfilename = (char *) malloc (sizeof (char) * 128);

    switch (j) {

    case 8:
        //Reverb
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[0][0], &lv[0][1], &lv[0][2], &lv[0][3], &lv[0][4],
                &lv[0][5], &lv[0][6], &lv[0][7], &lv[0][8], &lv[0][9],
                &lv[0][10], &lv[0][11], &Reverb_B);
        break;

    case 4:
        //Echo
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[1][0], &lv[1][1], &lv[1][2], &lv[1][3], &lv[1][4],
                &lv[1][5], &lv[1][6], &lv[1][7], &lv[1][8],&Echo_B);
        break;

    case 5:
        //Chorus
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[2][0], &lv[2][1], &lv[2][2], &lv[2][3], &lv[2][4],
                &lv[2][5], &lv[2][6], &lv[2][7], &lv[2][8], &lv[2][9],
                &lv[2][10], &lv[2][11], &lv[2][12], &Chorus_B);
        break;

    case 7:
        //Flanger
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[3][0], &lv[3][1], &lv[3][2], &lv[3][3], &lv[3][4],
                &lv[3][5], &lv[3][6], &lv[3][7], &lv[3][8], &lv[3][9],
                &lv[3][10], &lv[3][11], &lv[3][12], &Flanger_B);
        break;

    case 6:
        //Phaser
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[4][0], &lv[4][1], &lv[4][2], &lv[4][3], &lv[4][4],
                &lv[4][5], &lv[4][6], &lv[4][7], &lv[4][8], &lv[4][9],
                &lv[4][10], &lv[4][11], &Phaser_B);
        break;

    case 3:
        //Overdrive
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[5][0], &lv[5][1], &lv[5][2], &lv[5][3], &lv[5][4],
                &lv[5][5], &lv[5][6], &lv[5][7], &lv[5][8], &lv[5][9],
                &lv[5][10], &lv[5][11], &lv[5][12],&Overdrive_B);
        break;

    case 2:
        //Distorsion
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[6][0], &lv[6][1], &lv[6][2], &lv[6][3], &lv[6][4],
                &lv[6][5], &lv[6][6], &lv[6][7], &lv[6][8], &lv[6][9],
                &lv[6][10], &lv[6][11], &lv[6][12], &Distorsion_B);
        break;

    case 0:
        //EQ1
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[7][0], &lv[7][1], &lv[7][2], &lv[7][3], &lv[7][4],
                &lv[7][5], &lv[7][6], &lv[7][7], &lv[7][8], &lv[7][9],
                &lv[7][10], &lv[7][11], &EQ1_B);
        break;

    case 9:
        //EQ2
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[8][0], &lv[8][1], &lv[8][2], &lv[8][3], &lv[8][4],
                &lv[8][5], &lv[8][6], &lv[8][7], &lv[8][8], &lv[8][9],
                &EQ2_B);
        break;

    case 1:
        //Compressor
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[9][0], &lv[9][1], &lv[9][2], &lv[9][3], &lv[9][4],
                &lv[9][5], &lv[9][6], &lv[9][7], &lv[9][8], &Compressor_B);
        break;

    case 10:
        //WhaWha
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[11][0], &lv[11][1], &lv[11][2], &lv[11][3], &lv[11][4],
                &lv[11][5], &lv[11][6], &lv[11][7], &lv[11][8], &lv[11][9],
                &lv[11][10], &WhaWha_B);
        break;

    case 11:
        //Alienwah
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[12][0], &lv[12][1], &lv[12][2], &lv[12][3], &lv[12][4],
                &lv[12][5], &lv[12][6], &lv[12][7], &lv[12][8], &lv[12][9],
                &lv[12][10], &Alienwah_B);
        break;

    case 12:
        //Cabinet
        sscanf (buf, "%d,%d,%d\n", &lv[13][0], &lv[13][1], &Cabinet_B);
        break;

    case 13:
        //Pan
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[14][0], &lv[14][1], &lv[14][2], &lv[14][3], &lv[14][4],
                &lv[14][5], &lv[14][6], &lv[14][7], &lv[14][8],&Pan_B);
        break;

    case 14:
        //Harmonizer
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[15][0], &lv[15][1], &lv[15][2], &lv[15][3], &lv[15][4],
                &lv[15][5], &lv[15][6], &lv[15][7], &lv[15][8], &lv[15][9],
                &lv[15][10], &Harmonizer_B);
        break;

    case 15:
        //Musical Delay
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[16][0], &lv[16][1], &lv[16][2], &lv[16][3], &lv[16][4],
                &lv[16][5], &lv[16][6], &lv[16][7], &lv[16][8], &lv[16][9],
                &lv[16][10], &lv[16][11], &lv[16][12], &MusDelay_B);
        break;

    case 16:
        //NoiseGate
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[17][0], &lv[17][1], &lv[17][2], &lv[17][3], &lv[17][4],
                &lv[17][5], &lv[17][6], &Gate_B);

        break;

    case 17:
        //NewDist
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[18][0], &lv[18][1], &lv[18][2], &lv[18][3], &lv[18][4],
                &lv[18][5], &lv[18][6], &lv[18][7], &lv[18][8], &lv[18][9],
                &lv[18][10], &lv[18][11], &NewDist_B);
        break;

    case 18:
        //Analog Phaser
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[19][0], &lv[19][1], &lv[19][2], &lv[19][3], &lv[19][4],
                &lv[19][5], &lv[19][6], &lv[19][7], &lv[19][8], &lv[19][9],
                &lv[19][10], &lv[19][11], &APhaser_B);
        break;

    case 19:
        //Valve
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[20][0], &lv[20][1], &lv[20][2], &lv[20][3], &lv[20][4],
                &lv[20][5], &lv[20][6], &lv[20][7], &lv[20][8], &lv[20][9],
                &lv[20][10],&lv[20][11],&lv[20][12], &Valve_B);
        break;

    case 20:
        //Dual Flnage
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[21][0], &lv[21][1], &lv[21][2], &lv[21][3], &lv[21][4],
                &lv[21][5], &lv[21][6], &lv[21][7], &lv[21][8], &lv[21][9],
                &lv[21][10], &lv[21][11], &lv[21][12], &lv[21][13], &lv[21][14],
                &DFlange_B);
        break;

    case 21:
        //Ring
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[22][0], &lv[22][1], &lv[22][2], &lv[22][3], &lv[22][4],
                &lv[22][5], &lv[22][6], &lv[22][7], &lv[22][8], &lv[22][9],
                &lv[22][10], &lv[22][11], &lv[22][12],&Ring_B);
        break;

    case 22:
        //Exciter
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[23][0], &lv[23][1], &lv[23][2], &lv[23][3], &lv[23][4],
                &lv[23][5], &lv[23][6], &lv[23][7], &lv[23][8], &lv[23][9],
                &lv[23][10], &lv[23][11], &lv[23][12],&Exciter_B);
        break;

    case 23:
        //MBDist
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[24][0], &lv[24][1], &lv[24][2], &lv[24][3], &lv[24][4],
                &lv[24][5], &lv[24][6], &lv[24][7], &lv[24][8], &lv[24][9],
                &lv[24][10], &lv[24][11], &lv[24][12], &lv[24][13], &lv[24][14],
                &MBDist_B);
        break;

    case 24:
        //Arpie
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[25][0], &lv[25][1], &lv[25][2], &lv[25][3], &lv[25][4],
                &lv[25][5], &lv[25][6], &lv[25][7], &lv[25][8], &lv[25][9],
                &lv[25][10],&Arpie_B);
        break;

    case 25:
        //Expander
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[26][0], &lv[26][1], &lv[26][2], &lv[26][3], &lv[26][4],
                &lv[26][5], &lv[26][6], &Expander_B);

        break;

    case 26:
        //Shuffle
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[27][0], &lv[27][1], &lv[27][2], &lv[27][3], &lv[27][4],
                &lv[27][5], &lv[27][6], &lv[27][7], &lv[27][8], &lv[27][9],
                &lv[27][10],&Shuffle_B);
        break;

    case 27:
        //Synthfilter
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[28][0], &lv[28][1], &lv[28][2], &lv[28][3], &lv[28][4],
                &lv[28][5], &lv[28][6], &lv[28][7], &lv[28][8], &lv[28][9],
                &lv[28][10], &lv[28][11], &lv[28][12], &lv[28][13], &lv[28][14],
                &lv[28][15],&Synthfilter_B);
        break;

    case 28:
        //MBVvol
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[29][0], &lv[29][1], &lv[29][2], &lv[29][3], &lv[29][4],
                &lv[29][5], &lv[29][6], &lv[29][7], &lv[29][8], &lv[29][9],
                &lv[29][10],&MBVvol_B);
        break;

    case 29:
        //Convolotron
        memset(efx_Convol->Filename,0, sizeof(efx_Convol->Filename));
        memset(cfilename,0, sizeof(cfilename));
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s\n",
                &lv[30][0], &lv[30][1], &lv[30][2], &lv[30][3], &lv[30][4],
                &lv[30][5], &lv[30][6], &lv[30][7], &lv[30][8], &lv[30][9],
                &lv[30][10],&Convol_B,cfilename);
        strcpy(efx_Convol->Filename,cfilename);
        break;

    case 30:
        //Looper
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[31][0], &lv[31][1], &lv[31][2], &lv[31][3], &lv[31][4],
                &lv[31][5], &lv[31][6], &lv[31][7], &lv[31][8], &lv[31][9],
                &lv[31][10],&lv[31][11],&lv[31][12],&lv[31][13],&Looper_B);
        break;

    case 31:
        //RyanWah
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[32][0], &lv[32][1], &lv[32][2], &lv[32][3], &lv[32][4],
                &lv[32][5], &lv[32][6], &lv[32][7], &lv[32][8], &lv[32][9],
                &lv[32][10], &lv[32][11], &lv[32][12], &lv[32][13], &lv[32][14],
                &lv[32][15], &lv[32][16], &lv[32][17],&RyanWah_B);
        break;

    case 32:
        //Echoverse
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[33][0], &lv[33][1], &lv[33][2], &lv[33][3], &lv[33][4],
                &lv[33][5], &lv[33][6], &lv[33][7], &lv[33][8], &lv[33][9], &RBEcho_B);
        break;


    case 33:
        //CoilCrafter
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[34][0], &lv[34][1], &lv[34][2], &lv[34][3], &lv[34][4],
                &lv[34][5], &lv[34][6], &lv[34][7], &lv[34][8],&CoilCrafter_B);
        break;

    case 34:
        //CoilCrafter
        sscanf (buf, "%d,%d,%d,%d,%d,%d\n",
                &lv[35][0], &lv[35][1], &lv[35][2], &lv[35][3], &lv[35][4],
                &ShelfBoost_B);
        break;

    case 35:
        //Vocoder
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[36][0], &lv[36][1], &lv[36][2], &lv[36][3], &lv[36][4],
                &lv[36][5], &lv[36][6], &Vocoder_B);
        break;

    case 36:
        //Sustainer
        sscanf (buf, "%d,%d,%d\n",
                &lv[37][0], &lv[37][1], &Sustainer_B);
        break;

    case 37:
        //Sequence
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[38][0], &lv[38][1], &lv[38][2], &lv[38][3], &lv[38][4],
                &lv[38][5], &lv[38][6], &lv[38][7], &lv[38][8], &lv[38][9],
                &lv[38][10],&lv[38][11],&lv[38][12],&lv[38][13],&lv[38][14],&Looper_B);
        break;

    case 38:
        //Shifter
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[39][0], &lv[39][1], &lv[39][2], &lv[39][3], &lv[39][4],
                &lv[39][5], &lv[39][6], &lv[39][7], &lv[39][8], &lv[39][9], &Shifter_B);
        break;

    case 39:
        //StompBox
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d\n",
                &lv[40][0], &lv[40][1], &lv[40][2], &lv[40][3], &lv[40][4],
                &lv[40][5], &StompBox_B);
        break;

    case 40:
        //Reverbtron
        memset(efx_Reverbtron->Filename,0, sizeof(efx_Reverbtron->Filename));
        memset(cfilename,0, sizeof(cfilename));
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s\n",
                &lv[41][0], &lv[41][1], &lv[41][2], &lv[41][3], &lv[41][4],
                &lv[41][5], &lv[41][6], &lv[41][7], &lv[41][8], &lv[41][9],
                &lv[41][10],&lv[41][11],&lv[41][12], &lv[41][13], &lv[41][14],&lv[41][15],
                &Reverbtron_B,
                cfilename);
        strcpy(efx_Reverbtron->Filename,cfilename);
        break;

    case 41:
        //Echotron
        memset(efx_Echotron->Filename,0, sizeof(efx_Echotron->Filename));
        memset(cfilename,0, sizeof(cfilename));
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s\n",
                &lv[42][0], &lv[42][1], &lv[42][2], &lv[42][3], &lv[42][4],
                &lv[42][5], &lv[42][6], &lv[42][7], &lv[42][8], &lv[42][9],
                &lv[42][10],&lv[42][11],&lv[42][12], &lv[42][13], &lv[42][14],&lv[42][15],
                &Echotron_B,
                cfilename);
        strcpy(efx_Echotron->Filename,cfilename);
        break;

    case 42:
        //StereoHarm
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[43][0], &lv[43][1], &lv[43][2], &lv[43][3], &lv[43][4],
                &lv[43][5], &lv[43][6], &lv[43][7], &lv[43][8], &lv[43][9],
                &lv[43][10], &lv[43][11], &StereoHarm_B);
        break;

    case 43:
        //CompBand
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[44][0], &lv[44][1], &lv[44][2], &lv[44][3], &lv[44][4],
                &lv[44][5], &lv[44][6], &lv[44][7], &lv[44][8], &lv[44][9],
                &lv[44][10], &lv[44][11], &lv[44][12], &CompBand_B);
        break;

    case 44:
        //Opticaltrem
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d\n",
                &lv[45][0], &lv[45][1], &lv[45][2], &lv[45][3], &lv[45][4],
                &lv[45][5], &Opticaltrem_B);
        break;

    case 45:
        //Vibe
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[46][0], &lv[46][1], &lv[46][2], &lv[46][3], &lv[46][4],
                &lv[46][5], &lv[46][6], &lv[46][7], &lv[46][8], &lv[46][9], &lv[46][10],
                &Vibe_B);
        break;

    case 46:
        //Infinity
        sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                &lv[47][0], &lv[47][1], &lv[47][2], &lv[47][3], &lv[47][4],
                &lv[47][5], &lv[47][6], &lv[47][7], &lv[47][8], &lv[47][9],
                &lv[47][10], &lv[47][11], &lv[47][12], &lv[47][13], &lv[47][14],
                &lv[47][15], &lv[47][16], &lv[47][17], &Infinity_B);
        break;



    }


    free(cfilename);


}


void RKR::getbuf(char *buf, int j)
{

    switch (j) {
    case 8:
        //Reverb
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Rev->getpar (0), efx_Rev->getpar (1),
                 efx_Rev->getpar (2), efx_Rev->getpar (3),
                 efx_Rev->getpar (4), efx_Rev->getpar (5),
                 efx_Rev->getpar (6), efx_Rev->getpar (7),
                 efx_Rev->getpar (8), efx_Rev->getpar (9),
                 efx_Rev->getpar (10), efx_Rev->getpar (11), Reverb_Bypass);
        break;

    case 4:
        //Echo
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Echo->getpar (0), efx_Echo->getpar (1),
                 efx_Echo->getpar (2), efx_Echo->getpar (3),
                 efx_Echo->getpar (4), efx_Echo->getpar (5),
                 efx_Echo->getpar (6), efx_Echo->getpar(7),
                 efx_Echo->getpar (8), Echo_Bypass);
        break;

    case 5:
        //Chorus
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Chorus->getpar (0), efx_Chorus->getpar (1),
                 efx_Chorus->getpar (2), efx_Chorus->getpar (3),
                 efx_Chorus->getpar (4), efx_Chorus->getpar (5),
                 efx_Chorus->getpar (6), efx_Chorus->getpar (7),
                 efx_Chorus->getpar (8), efx_Chorus->getpar (9),
                 efx_Chorus->getpar (10), efx_Chorus->getpar (11),
                 efx_Chorus->getpar (12), Chorus_Bypass);
        break;

    case 7:
        //Flanger
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Flanger->getpar (0), efx_Flanger->getpar (1),
                 efx_Flanger->getpar (2), efx_Flanger->getpar (3),
                 efx_Flanger->getpar (4), efx_Flanger->getpar (5),
                 efx_Flanger->getpar (6), efx_Flanger->getpar (7),
                 efx_Flanger->getpar (8), efx_Flanger->getpar (9),
                 efx_Flanger->getpar (10), efx_Flanger->getpar (11),
                 efx_Flanger->getpar (12), Flanger_Bypass);
        break;

    case 6:
        //Phaser
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Phaser->getpar (0), efx_Phaser->getpar (1),
                 efx_Phaser->getpar (2), efx_Phaser->getpar (3),
                 efx_Phaser->getpar (4), efx_Phaser->getpar (5),
                 efx_Phaser->getpar (6), efx_Phaser->getpar (7),
                 efx_Phaser->getpar (8), efx_Phaser->getpar (9),
                 efx_Phaser->getpar (10), efx_Phaser->getpar (11),
                 Phaser_Bypass);
        break;

    case 3:
        //Overdrive
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Overdrive->getpar (0), efx_Overdrive->getpar (1),
                 efx_Overdrive->getpar (2), efx_Overdrive->getpar (3),
                 efx_Overdrive->getpar (4), efx_Overdrive->getpar (5),
                 efx_Overdrive->getpar (6), efx_Overdrive->getpar (7),
                 efx_Overdrive->getpar (8), efx_Overdrive->getpar (9),
                 efx_Overdrive->getpar (10), efx_Overdrive->getpar (11),
                 efx_Overdrive->getpar (12),Overdrive_Bypass);
        break;

    case 2:
        //Distorsion
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Distorsion->getpar (0), efx_Distorsion->getpar (1),
                 efx_Distorsion->getpar (2), efx_Distorsion->getpar (3),
                 efx_Distorsion->getpar (4), efx_Distorsion->getpar (5),
                 efx_Distorsion->getpar (6), efx_Distorsion->getpar (7),
                 efx_Distorsion->getpar (8), efx_Distorsion->getpar (9),
                 efx_Distorsion->getpar (10), efx_Distorsion->getpar (11),
                 efx_Distorsion->getpar(12),Distorsion_Bypass);
        break;

    case 0:
        //EQ1
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_EQ1->getpar (12), efx_EQ1->getpar (5 + 12),
                 efx_EQ1->getpar (10 + 12), efx_EQ1->getpar (15 + 12),
                 efx_EQ1->getpar (20 + 12), efx_EQ1->getpar (25 + 12),
                 efx_EQ1->getpar (30 + 12), efx_EQ1->getpar (35 + 12),
                 efx_EQ1->getpar (40 + 12), efx_EQ1->getpar (45 + 12),
                 efx_EQ1->getpar (0), efx_EQ1->getpar (13), EQ1_Bypass);
        break;

    case 9:
        //EQ2
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_EQ2->getpar (11), efx_EQ2->getpar (12),
                 efx_EQ2->getpar (13), efx_EQ2->getpar (5 + 11),
                 efx_EQ2->getpar (5 + 12), efx_EQ2->getpar (5 + 13),
                 efx_EQ2->getpar (10 + 11), efx_EQ2->getpar (10 + 12),
                 efx_EQ2->getpar (10 + 13), efx_EQ2->getpar (0),
                 EQ2_Bypass);
        break;

    case 1:
        // Compressor
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Compressor->getpar (1), efx_Compressor->getpar (2),
                 efx_Compressor->getpar (3), efx_Compressor->getpar (4),
                 efx_Compressor->getpar (5), efx_Compressor->getpar (6),
                 efx_Compressor->getpar (7), efx_Compressor->getpar (8),
                 efx_Compressor->getpar (9), Compressor_Bypass);
        break;


    case 10:
        //WhaWha
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_WhaWha->getpar (0), efx_WhaWha->getpar (1),
                 efx_WhaWha->getpar (2), efx_WhaWha->getpar (3),
                 efx_WhaWha->getpar (4), efx_WhaWha->getpar (5),
                 efx_WhaWha->getpar (6), efx_WhaWha->getpar (7),
                 efx_WhaWha->getpar (8), efx_WhaWha->getpar (9),
                 efx_WhaWha->Ppreset, WhaWha_Bypass);
        break;

    case 11:
        //Alienwah
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Alienwah->getpar (0), efx_Alienwah->getpar (1),
                 efx_Alienwah->getpar (2), efx_Alienwah->getpar (3),
                 efx_Alienwah->getpar (4), efx_Alienwah->getpar (5),
                 efx_Alienwah->getpar (6), efx_Alienwah->getpar (7),
                 efx_Alienwah->getpar (8), efx_Alienwah->getpar (9),
                 efx_Alienwah->getpar (10), Alienwah_Bypass);
        break;

    case 12:
        //Cabinet
        sprintf (buf, "%d,%d,%d\n",
                 Cabinet_Preset, efx_Cabinet->getpar (0), Cabinet_Bypass);
        break;

    case 13:
        //Pan
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Pan->getpar (0), efx_Pan->getpar (1),
                 efx_Pan->getpar (2), efx_Pan->getpar (3),
                 efx_Pan->getpar (4), efx_Pan->getpar (5),
                 efx_Pan->getpar (6), efx_Pan->getpar (7),
                 efx_Pan->getpar (8), Pan_Bypass);
        break;

    case 14:
        //Harmonizer
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Har->getpar (0), efx_Har->getpar (1),
                 efx_Har->getpar (2), efx_Har->getpar (3),
                 efx_Har->getpar (4), efx_Har->getpar (5),
                 efx_Har->getpar (6), efx_Har->getpar (7),
                 efx_Har->getpar (8), efx_Har->getpar (9),
                 efx_Har->getpar (10), Harmonizer_Bypass);
        break;

    case 15:
        //MusicalDelay
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_MusDelay->getpar (0), efx_MusDelay->getpar (1),
                 efx_MusDelay->getpar (2), efx_MusDelay->getpar (3),
                 efx_MusDelay->getpar (4), efx_MusDelay->getpar (5),
                 efx_MusDelay->getpar (6), efx_MusDelay->getpar (7),
                 efx_MusDelay->getpar (8), efx_MusDelay->getpar (9),
                 efx_MusDelay->getpar (10), efx_MusDelay->getpar (11),
                 efx_MusDelay->getpar (12), MusDelay_Bypass);
        break;

    case 16:
        //NoiseGate
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Gate->getpar (1), efx_Gate->getpar (2),
                 efx_Gate->getpar (3), efx_Gate->getpar (4),
                 efx_Gate->getpar (5), efx_Gate->getpar (6),
                 efx_Gate->getpar (7), Gate_Bypass);
        break;

    case 17:
        //NewDist
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_NewDist->getpar (0), efx_NewDist->getpar (1),
                 efx_NewDist->getpar (2), efx_NewDist->getpar (3),
                 efx_NewDist->getpar (4), efx_NewDist->getpar (5),
                 efx_NewDist->getpar (6), efx_NewDist->getpar (7),
                 efx_NewDist->getpar (8), efx_NewDist->getpar (9),
                 efx_NewDist->getpar (10), efx_NewDist->getpar (11),
                 efx_NewDist->getpar(11),NewDist_Bypass);
        break;

    case 18:
        //Analog Phaser
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_APhaser->getpar (0), efx_APhaser->getpar (1),
                 efx_APhaser->getpar (2), efx_APhaser->getpar (3),
                 efx_APhaser->getpar (4), efx_APhaser->getpar (5),
                 efx_APhaser->getpar (6), efx_APhaser->getpar (7),
                 efx_APhaser->getpar (8), efx_APhaser->getpar (9),
                 efx_APhaser->getpar (10), efx_APhaser->getpar (11),
                 APhaser_Bypass);
        break;

    case 19:
        //Valve
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Valve->getpar (0), efx_Valve->getpar (1),
                 efx_Valve->getpar (2), efx_Valve->getpar (3),
                 efx_Valve->getpar (4), efx_Valve->getpar (5),
                 efx_Valve->getpar (6), efx_Valve->getpar (7),
                 efx_Valve->getpar (8), efx_Valve->getpar (9),
                 efx_Valve->getpar (10), efx_Valve->getpar (11),
                 efx_Valve->getpar (12), Valve_Bypass);
        break;

    case 20:
        //Dual_Flange
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_DFlange->getpar (0), efx_DFlange->getpar (1),
                 efx_DFlange->getpar (2), efx_DFlange->getpar (3),
                 efx_DFlange->getpar (4), efx_DFlange->getpar (5),
                 efx_DFlange->getpar (6), efx_DFlange->getpar (7),
                 efx_DFlange->getpar (8), efx_DFlange->getpar (9),
                 efx_DFlange->getpar (10), efx_DFlange->getpar (11),
                 efx_DFlange->getpar (12), efx_DFlange->getpar (13),
                 efx_DFlange->getpar (14),DFlange_Bypass);
        break;

    case 21:
        //Ring
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Ring->getpar (0), efx_Ring->getpar (1),
                 efx_Ring->getpar (2), efx_Ring->getpar (3),
                 efx_Ring->getpar (4), efx_Ring->getpar (5),
                 efx_Ring->getpar (6), efx_Ring->getpar (7),
                 efx_Ring->getpar (8), efx_Ring->getpar (9),
                 efx_Ring->getpar (10), efx_Ring->getpar (11),
                 efx_Ring->getpar (12),  Ring_Bypass);
        break;

    case 22:
        //Exciter
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Exciter->getpar (0), efx_Exciter->getpar (1),
                 efx_Exciter->getpar (2), efx_Exciter->getpar (3),
                 efx_Exciter->getpar (4), efx_Exciter->getpar (5),
                 efx_Exciter->getpar (6), efx_Exciter->getpar (7),
                 efx_Exciter->getpar (8), efx_Exciter->getpar (9),
                 efx_Exciter->getpar (10), efx_Exciter->getpar (11),
                 efx_Exciter->getpar (12),  Exciter_Bypass);
        break;

    case 23:
        //MBDist
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_MBDist->getpar (0), efx_MBDist->getpar (1),
                 efx_MBDist->getpar (2), efx_MBDist->getpar (3),
                 efx_MBDist->getpar (4), efx_MBDist->getpar (5),
                 efx_MBDist->getpar (6), efx_MBDist->getpar (7),
                 efx_MBDist->getpar (8), efx_MBDist->getpar (9),
                 efx_MBDist->getpar (10), efx_MBDist->getpar (11),
                 efx_MBDist->getpar (12), efx_MBDist->getpar (13),
                 efx_MBDist->getpar (14), MBDist_Bypass);
        break;

    case 24:
        //Arpie
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Arpie->getpar (0), efx_Arpie->getpar (1),
                 efx_Arpie->getpar (2), efx_Arpie->getpar (3),
                 efx_Arpie->getpar (4), efx_Arpie->getpar (5),
                 efx_Arpie->getpar (6), efx_Arpie->getpar (7),
                 efx_Arpie->getpar (8), efx_Arpie->getpar (9),
                 efx_Arpie->getpar (10), Arpie_Bypass);
        break;

    case 25:
        //Expander
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Expander->getpar (1), efx_Expander->getpar (2),
                 efx_Expander->getpar (3), efx_Expander->getpar (4),
                 efx_Expander->getpar (5), efx_Expander->getpar (6),
                 efx_Expander->getpar (7), Expander_Bypass);
        break;

    case 26:
        //Shuffle
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Shuffle->getpar (0), efx_Shuffle->getpar (1),
                 efx_Shuffle->getpar (2), efx_Shuffle->getpar (3),
                 efx_Shuffle->getpar (4), efx_Shuffle->getpar (5),
                 efx_Shuffle->getpar (6), efx_Shuffle->getpar (7),
                 efx_Shuffle->getpar (8), efx_Shuffle->getpar (9),
                 efx_Shuffle->getpar(10), Shuffle_Bypass);
        break;

    case 27:
        //Synthfilter
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Synthfilter->getpar (0), efx_Synthfilter->getpar (1),
                 efx_Synthfilter->getpar (2), efx_Synthfilter->getpar (3),
                 efx_Synthfilter->getpar (4), efx_Synthfilter->getpar (5),
                 efx_Synthfilter->getpar (6), efx_Synthfilter->getpar (7),
                 efx_Synthfilter->getpar (8), efx_Synthfilter->getpar (9),
                 efx_Synthfilter->getpar (10), efx_Synthfilter->getpar (11),
                 efx_Synthfilter->getpar (12), efx_Synthfilter->getpar (13),
                 efx_Synthfilter->getpar(14), efx_Synthfilter->getpar(15),
                 Synthfilter_Bypass);
        break;

    case 28:
        //MBVvol
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_MBVvol->getpar (0), efx_MBVvol->getpar (1),
                 efx_MBVvol->getpar (2), efx_MBVvol->getpar (3),
                 efx_MBVvol->getpar (4), efx_MBVvol->getpar (5),
                 efx_MBVvol->getpar (6), efx_MBVvol->getpar (7),
                 efx_MBVvol->getpar (8), efx_MBVvol->getpar (9),
                 efx_MBVvol->getpar (10), MBVvol_Bypass);
        break;

    case 29:
        //Convolotron
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s\n",
                 efx_Convol->getpar (0), efx_Convol->getpar (1),
                 efx_Convol->getpar (2), efx_Convol->getpar (3),
                 efx_Convol->getpar (4), efx_Convol->getpar (5),
                 efx_Convol->getpar (6), efx_Convol->getpar (7),
                 efx_Convol->getpar (8), efx_Convol->getpar (9),
                 efx_Convol->getpar (10), Convol_Bypass, efx_Convol->Filename);
        break;

    case 30:
        //Looper
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Looper->getpar (0), efx_Looper->getpar (1),
                 efx_Looper->getpar (2), efx_Looper->getpar (3),
                 efx_Looper->getpar (4), efx_Looper->getpar (5),
                 efx_Looper->getpar (6), efx_Looper->getpar (7),
                 efx_Looper->getpar (8), efx_Looper->getpar (9),
                 efx_Looper->getpar (10), efx_Looper->getpar (11),
                 efx_Looper->getpar (12), efx_Looper->getpar (13),Looper_Bypass);
        break;

    case 31:
        //RyanWah
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_RyanWah->getpar (0), efx_RyanWah->getpar (1),
                 efx_RyanWah->getpar (2), efx_RyanWah->getpar (3),
                 efx_RyanWah->getpar (4), efx_RyanWah->getpar (5),
                 efx_RyanWah->getpar (6), efx_RyanWah->getpar (7),
                 efx_RyanWah->getpar (8), efx_RyanWah->getpar (9),
                 efx_RyanWah->getpar (10), efx_RyanWah->getpar (11),
                 efx_RyanWah->getpar (12), efx_RyanWah->getpar (13),
                 efx_RyanWah->getpar(14), efx_RyanWah->getpar(15),
                 efx_RyanWah->getpar(16), efx_RyanWah->getpar(17),
                 efx_RyanWah->getpar(18), RyanWah_Bypass);

        break;
    case 32:
        //Echoverse
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_RBEcho->getpar (0), efx_RBEcho->getpar (1),
                 efx_RBEcho->getpar (2), efx_RBEcho->getpar (3),
                 efx_RBEcho->getpar (4), efx_RBEcho->getpar (5),
                 efx_RBEcho->getpar (6), efx_RBEcho->getpar (7),
                 efx_RBEcho->getpar (8), efx_RBEcho->getpar (9),RBEcho_Bypass);
        break;



    case 33:
        //CoilCrafter
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_CoilCrafter->getpar (0), efx_CoilCrafter->getpar (1),
                 efx_CoilCrafter->getpar (2), efx_CoilCrafter->getpar (3),
                 efx_CoilCrafter->getpar (4), efx_CoilCrafter->getpar (5),
                 efx_CoilCrafter->getpar (6), efx_CoilCrafter->getpar (7),
                 efx_CoilCrafter->getpar (8), CoilCrafter_Bypass);
        break;

    case 34:
        //ShelfBoost
        sprintf (buf, "%d,%d,%d,%d,%d,%d\n",
                 efx_ShelfBoost->getpar (0), efx_ShelfBoost->getpar (1),
                 efx_ShelfBoost->getpar (2), efx_ShelfBoost->getpar (3),
                 efx_ShelfBoost->getpar (4), ShelfBoost_Bypass);
        break;

    case 35:
        //Vocoder
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Vocoder->getpar (0), efx_Vocoder->getpar (1),
                 efx_Vocoder->getpar (2), efx_Vocoder->getpar (3),
                 efx_Vocoder->getpar (4), efx_Vocoder->getpar (5),
                 efx_Vocoder->getpar (6), Vocoder_Bypass);
        break;

    case 36:
        //Sustainer
        sprintf (buf, "%d,%d,%d\n",
                 efx_Sustainer->getpar (0), efx_Sustainer->getpar (1),
                 Sustainer_Bypass);
        break;

    case 37:
        //Sequence
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Sequence->getpar (0), efx_Sequence->getpar (1),
                 efx_Sequence->getpar (2), efx_Sequence->getpar (3),
                 efx_Sequence->getpar (4), efx_Sequence->getpar (5),
                 efx_Sequence->getpar (6), efx_Sequence->getpar (7),
                 efx_Sequence->getpar (8), efx_Sequence->getpar (9),
                 efx_Sequence->getpar (10), efx_Sequence->getpar (11),
                 efx_Sequence->getpar (12), efx_Sequence->getpar (13),
                 efx_Sequence->getpar (14), Sequence_Bypass);
        break;

    case 38:
        //Shifter
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Shifter->getpar (0), efx_Shifter->getpar (1),
                 efx_Shifter->getpar (2), efx_Shifter->getpar (3),
                 efx_Shifter->getpar (4), efx_Shifter->getpar (5),
                 efx_Shifter->getpar (6), efx_Shifter->getpar (7),
                 efx_Shifter->getpar (8), efx_Shifter->getpar (9),Shifter_Bypass);
        break;


    case 39:
        //StompBox
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d\n",
                 efx_StompBox->getpar (0), efx_StompBox->getpar (1),
                 efx_StompBox->getpar (2), efx_StompBox->getpar (3),
                 efx_StompBox->getpar (4), efx_StompBox->getpar (5),
                 StompBox_Bypass);
        break;

    case 40:
        //Reverbtron
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s\n",
                 efx_Reverbtron->getpar (0), efx_Reverbtron->getpar (1),
                 efx_Reverbtron->getpar (2), efx_Reverbtron->getpar (3),
                 efx_Reverbtron->getpar (4), efx_Reverbtron->getpar (5),
                 efx_Reverbtron->getpar (6), efx_Reverbtron->getpar (7),
                 efx_Reverbtron->getpar (8), efx_Reverbtron->getpar (9),
                 efx_Reverbtron->getpar (10), efx_Reverbtron->getpar (11),
                 efx_Reverbtron->getpar (12), efx_Reverbtron->getpar (13),
                 efx_Reverbtron->getpar (14), efx_Reverbtron->getpar (15),
                 Reverbtron_Bypass, efx_Reverbtron->Filename);
        break;

    case 41:
        //Echotron
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s\n",
                 efx_Echotron->getpar (0), efx_Echotron->getpar (1),
                 efx_Echotron->getpar (2), efx_Echotron->getpar (3),
                 efx_Echotron->getpar (4), efx_Echotron->getpar (5),
                 efx_Echotron->getpar (6), efx_Echotron->getpar (7),
                 efx_Echotron->getpar (8), efx_Echotron->getpar (9),
                 efx_Echotron->getpar (10), efx_Echotron->getpar (11),
                 efx_Echotron->getpar (12), efx_Echotron->getpar (13),
                 efx_Echotron->getpar (14), efx_Echotron->getpar (15),
                 Echotron_Bypass, efx_Echotron->Filename);
        break;
    case 42:
        //StereoHarm
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_StereoHarm->getpar (0), efx_StereoHarm->getpar (1),
                 efx_StereoHarm->getpar (2), efx_StereoHarm->getpar (3),
                 efx_StereoHarm->getpar (4), efx_StereoHarm->getpar (5),
                 efx_StereoHarm->getpar (6), efx_StereoHarm->getpar (7),
                 efx_StereoHarm->getpar (8), efx_StereoHarm->getpar (9),
                 efx_StereoHarm->getpar (10), efx_StereoHarm->getpar (11),
                 StereoHarm_Bypass);
        break;

    case 43:
        //CompBand
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_CompBand->getpar (0), efx_CompBand->getpar (1),
                 efx_CompBand->getpar (2), efx_CompBand->getpar (3),
                 efx_CompBand->getpar (4), efx_CompBand->getpar (5),
                 efx_CompBand->getpar (6), efx_CompBand->getpar (7),
                 efx_CompBand->getpar (8), efx_CompBand->getpar (9),
                 efx_CompBand->getpar (10), efx_CompBand->getpar (11),
                 efx_CompBand->getpar (12),CompBand_Bypass);
        break;

    case 44:
        //Opticaltrem
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Opticaltrem->getpar (0), efx_Opticaltrem->getpar (1),
                 efx_Opticaltrem->getpar (2), efx_Opticaltrem->getpar (3),
                 efx_Opticaltrem->getpar (4), efx_Opticaltrem->getpar (5),
                 efx_Opticaltrem->getpar (6), Opticaltrem_Bypass);
        break;


    case 45:
        //Vibe
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Vibe->getpar (0), efx_Vibe->getpar (1),
                 efx_Vibe->getpar (2), efx_Vibe->getpar (3),
                 efx_Vibe->getpar (4), efx_Vibe->getpar (5),
                 efx_Vibe->getpar (6), efx_Vibe->getpar (7),
                 efx_Vibe->getpar (8), efx_Vibe->getpar (9),efx_Vibe->getpar (10),
                 Vibe_Bypass);
        break;

    case 46:
        //Infinity
        sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                 efx_Infinity->getpar (0), efx_Infinity->getpar (1),
                 efx_Infinity->getpar (2), efx_Infinity->getpar (3),
                 efx_Infinity->getpar (4), efx_Infinity->getpar (5),
                 efx_Infinity->getpar (6), efx_Infinity->getpar (7),
                 efx_Infinity->getpar (8), efx_Infinity->getpar (9),
                 efx_Infinity->getpar (10), efx_Infinity->getpar (11),
                 efx_Infinity->getpar (12), efx_Infinity->getpar (13),
                 efx_Infinity->getpar (14), efx_Infinity->getpar (15),
                 efx_Infinity->getpar (16), efx_Infinity->getpar (17),
                 Infinity_Bypass);
        break;


    }


}

void
RKR::savefile (char *filename)
{

    int i, j;
    FILE *fn;
    char buf[256];
    fn = fopen (filename, "w");
    if(errno == EACCES) {
        Error_Handle(3);
        fclose(fn);
        return;
    }

    memset (buf, 0, sizeof (buf));
    sprintf (buf, "%s\n", VERSION);
    fputs (buf, fn);


    //Autor

    memset (buf, 0, sizeof (buf));
    if (strlen (Author) != 0)
        sprintf (buf, "%s\n", Author);
    else {
        if (UserRealName != NULL)
            sprintf (buf, "%s\n", UserRealName);
        else
            sprintf (buf, "%s\n", getenv ("USER"));
    }
    fputs (buf, fn);

    //Preset Name

    memset (buf, 0, sizeof (buf));
    fputs (Preset_Name, fn);
    fputs ("\n", fn);


    //General
    memset (buf, 0, sizeof (buf));
    sprintf (buf, "%f,%f,%f,%d\n", Input_Gain, Master_Volume, Fraction_Bypass, Bypass);
    fputs (buf, fn);


    for (i = 0; i < 10; i++) {
        j = efx_order[i];
        memset (buf, 0, sizeof (buf));
        getbuf(buf,j);
        fputs (buf, fn);

    }



    // Order
    memset (buf, 0, sizeof (buf));
    sprintf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
             efx_order[0], efx_order[1], efx_order[2], efx_order[3],
             efx_order[4], efx_order[5], efx_order[6], efx_order[7],
             efx_order[8], efx_order[9]);

    fputs (buf, fn);


    for(i=0; i<128; i++) {
        memset(buf,0, sizeof(buf));
        sprintf(buf,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                XUserMIDI[i][0], XUserMIDI[i][1], XUserMIDI[i][2], XUserMIDI[i][3], XUserMIDI[i][4],
                XUserMIDI[i][5], XUserMIDI[i][6], XUserMIDI[i][7], XUserMIDI[i][8], XUserMIDI[i][9],
                XUserMIDI[i][10], XUserMIDI[i][10], XUserMIDI[i][12], XUserMIDI[i][13], XUserMIDI[i][14],
                XUserMIDI[i][15], XUserMIDI[i][16], XUserMIDI[i][17], XUserMIDI[i][18], XUserMIDI[i][19]);

        fputs (buf, fn);

    }





    fclose (fn);

}




void
RKR::loadfile (char *filename)
{

    int i, j;
    int l[10];
    FILE *fn;
    float in_vol, out_vol;
    float balance=1.0f;
    char buf[256];

    if ((fn = fopen (filename, "r")) == NULL)
        return;


    New();

    for (i = 0; i < 14; i++) {
        memset (buf, 0, sizeof (buf));
        fgets (buf, sizeof buf, fn);

    }

    //Order
    memset (buf, 0, sizeof (buf));
    fgets (buf, sizeof buf, fn);
    sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
            &l[0], &l[1], &l[2], &l[3], &l[4], &l[5], &l[6], &l[7], &l[8],
            &l[9]);


    fclose (fn);


    if ((fn = fopen (filename, "r")) == NULL) {
        return;
    }
    //Version
    memset (buf, 0, sizeof (buf));
    fgets (buf, sizeof buf, fn);


    //Author

    memset (Author,0, 64);
    memset (buf, 0, sizeof (buf));
    fgets (buf, sizeof buf, fn);

    for (i = 0; i < 64; i++)
        if (buf[i] > 20)
            Author[i] = buf[i];


    // Preset Name

    memset (Preset_Name, 0,64);
    memset(buf, 0, sizeof (buf));
    fgets (buf, sizeof buf, fn);

    for (i = 0; i < 64; i++)
        if (buf[i] > 20)
            Preset_Name[i] = buf[i];

    //General

    memset (buf, 0, sizeof (buf));
    fgets (buf, sizeof buf, fn);
    sscanf (buf, "%f,%f,%f,%d\n", &in_vol, &out_vol, &balance, &Bypass_B);

    if ((actuvol == 0) || (needtoloadstate)) {
        Fraction_Bypass = balance;
        Input_Gain = in_vol;
        Master_Volume = out_vol;
    }


    for (i = 0; i < 10; i++) {
        j = l[i];

        memset (buf, 0, sizeof (buf));
        fgets (buf, sizeof buf, fn);
        putbuf(buf,j);



    }

    //Order
    memset (buf, 0, sizeof (buf));
    fgets (buf, sizeof buf, fn);
    sscanf (buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
            &lv[10][0], &lv[10][1], &lv[10][2], &lv[10][3], &lv[10][4],
            &lv[10][5], &lv[10][6], &lv[10][7], &lv[10][8], &lv[10][9]);



    for(i=0; i<128; i++) {
        memset(buf,0, sizeof(buf));
        fgets (buf, sizeof buf, fn);

        sscanf(buf,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
               &XUserMIDI[i][0], &XUserMIDI[i][1], &XUserMIDI[i][2], &XUserMIDI[i][3], &XUserMIDI[i][4],
               &XUserMIDI[i][5], &XUserMIDI[i][6], &XUserMIDI[i][7], &XUserMIDI[i][8], &XUserMIDI[i][9],
               &XUserMIDI[i][10], &XUserMIDI[i][10], &XUserMIDI[i][12], &XUserMIDI[i][13], &XUserMIDI[i][14],
               &XUserMIDI[i][15], &XUserMIDI[i][16], &XUserMIDI[i][17], &XUserMIDI[i][18], &XUserMIDI[i][19]);

    }



    fclose (fn);
    Actualizar_Audio ();

}


void
RKR::Actualizar_Audio ()
{
    int i,j;


    Bypass = 0;
    for (i = 0; i < 12; i++)
        efx_order[i] = lv[10][i];
    Harmonizer_Bypass=0;
    Ring_Bypass = 0;
    StereoHarm_Bypass = 0;


    for (j=0; j<10; j++) {
        switch(efx_order[j]) {
        case 0: //EQ1
            EQ1_Bypass = 0;
            efx_EQ1->cleanup();
            for (i = 0; i < 10; i++) {
                efx_EQ1->changepar (i * 5 + 12, lv[7][i]);
                efx_EQ1->changepar (i * 5 + 13, lv[7][11]);
            }
            efx_EQ1->changepar (0, lv[7][10]);
            EQ1_Bypass = EQ1_B;
            break;

        case 1:// Compressor
            Compressor_Bypass = 0;
            efx_Compressor->cleanup();
            for (i = 0; i <= 9; i++)
                efx_Compressor->Compressor_Change (i + 1, lv[9][i]);
            Compressor_Bypass = Compressor_B;
            break;

        case 2://Distortion

            Distorsion_Bypass = 0;
            efx_Distorsion->cleanup();
            for (i = 0; i <= 12; i++)
                efx_Distorsion->changepar (i, lv[6][i]);
            Distorsion_Bypass = Distorsion_B;
            break;

        case 3://Overdrive

            Overdrive_Bypass = 0;
            efx_Overdrive->cleanup();
            for (i = 0; i <= 12; i++)
                efx_Overdrive->changepar (i, lv[5][i]);
            Overdrive_Bypass = Overdrive_B;
            break;

        case 4://Echo

            Echo_Bypass = 0;
            efx_Echo->cleanup();
            for (i = 0; i <= 8; i++)
                efx_Echo->changepar (i, lv[1][i]);
            Echo_Bypass = Echo_B;
            break;

        case 5://Chorus

            Chorus_Bypass = 0;
            efx_Chorus->cleanup();
            for (i = 0; i <= 12; i++)
                efx_Chorus->changepar (i, lv[2][i]);
            Chorus_Bypass = Chorus_B;
            break;

        case 6://Phaser

            Phaser_Bypass = 0;
            efx_Phaser->cleanup();
            for (i = 0; i <= 11; i++)
                efx_Phaser->changepar (i,lv[4][i]);
            Phaser_Bypass = Phaser_B;
            break;

        case 7://Flanger

            Flanger_Bypass = 0;
            efx_Flanger->cleanup();
            for (i = 0; i <= 12; i++)
                efx_Flanger->changepar (i, lv[3][i]);
            Flanger_Bypass = Flanger_B;
            break;

        case 8://Reverb

            Reverb_Bypass = 0;
            efx_Rev->cleanup();
            for (i = 0; i <= 11; i++)
                efx_Rev->changepar (i, lv[0][i]);
            Reverb_Bypass = Reverb_B;
            break;

        case 9://EQ2

            EQ2_Bypass = 0;
            efx_EQ2->cleanup();
            for (i = 0; i < 3; i++) {
                efx_EQ2->changepar (i * 5 + 11, lv[8][0 + i * 3]);
                efx_EQ2->changepar (i * 5 + 12, lv[8][1 + i * 3]);
                efx_EQ2->changepar (i * 5 + 13, lv[8][2 + i * 3]);
            }
            efx_EQ2->changepar (0, lv[8][9]);
            EQ2_Bypass = EQ2_B;
            break;

        case 10://WhaWha

            WhaWha_Bypass = 0;
            efx_WhaWha->cleanup();
            efx_WhaWha->setpreset (lv[11][10]);
            for (i = 0; i <= 9; i++)
                efx_WhaWha->changepar (i, lv[11][i]);
            WhaWha_Bypass = WhaWha_B;
            break;

        case 11://Alienwah

            Alienwah_Bypass = 0;
            efx_Alienwah->cleanup();
            for (i = 0; i <= 10; i++)
                efx_Alienwah->changepar (i, lv[12][i]);
            Alienwah_Bypass = Alienwah_B;
            break;

        case 12://Cabinet

            Cabinet_Bypass = 0;
            efx_Cabinet->cleanup();
            Cabinet_setpreset (lv[13][0]);
            efx_Cabinet->changepar (0,lv[13][1]);
            Cabinet_Bypass = Cabinet_B;
            break;

        case 13://Pan

            Pan_Bypass = 0;
            efx_Pan->cleanup();
            for (i = 0; i <= 8; i++)
                efx_Pan->changepar (i, lv[14][i]);
            Pan_Bypass = Pan_B;
            break;

        case 14://Harmonizer

            Harmonizer_Bypass = 0;
            efx_Har->cleanup();
            for (i = 0; i <= 10; i++)
                efx_Har->changepar (i, lv[15][i]);
            Harmonizer_Bypass = Harmonizer_B;
            break;

        case 15://MusDelay

            MusDelay_Bypass = 0;
            efx_MusDelay->cleanup();
            for (i = 0; i <= 12; i++)
                efx_MusDelay->changepar (i, lv[16][i]);
            MusDelay_Bypass = MusDelay_B;
            break;

        case 16://Gate

            Gate_Bypass = 0;
            efx_Gate->cleanup();
            for (i = 0; i <= 6; i++)
                efx_Gate->Gate_Change (i + 1,lv[17][i]);
            Gate_Bypass = Gate_B;
            break;

        case 17://NewDist

            NewDist_Bypass = 0;
            efx_NewDist->cleanup();
            for (i = 0; i <= 11; i++)
                efx_NewDist->changepar (i, lv[18][i]);
            NewDist_Bypass = NewDist_B;
            break;

        case 18://APhaser

            APhaser_Bypass = 0;
            efx_APhaser->cleanup();
            for (i = 0; i <= 12; i++)
                efx_APhaser->changepar (i, lv[19][i]);
            APhaser_Bypass = APhaser_B;
            break;

        case 19://Valve

            Valve_Bypass = 0;
            efx_Valve->cleanup();
            for (i = 0; i <= 12; i++)
                efx_Valve->changepar (i, lv[20][i]);
            Valve_Bypass = Valve_B;
            break;

        case 20://DFlange

            DFlange_Bypass = 0;
            efx_DFlange->cleanup();
            for (i = 0; i <= 14; i++)
                efx_DFlange->changepar (i, lv[21][i]);
            DFlange_Bypass = DFlange_B;
            break;

        case 21://Ring

            Ring_Bypass = 0;
            efx_Ring->cleanup();
            for (i = 0; i <= 12; i++)
                efx_Ring->changepar (i, lv[22][i]);
            Ring_Bypass = Ring_B;
            break;

        case 22://Exciter

            Exciter_Bypass = 0;
            efx_Exciter->cleanup();
            for (i = 0; i <= 12; i++)
                efx_Exciter->changepar (i, lv[23][i]);
            Exciter_Bypass = Exciter_B;
            break;

        case 23://MBDist

            MBDist_Bypass = 0;
            efx_MBDist->cleanup();
            for (i = 0; i <= 14; i++)
                efx_MBDist->changepar (i, lv[24][i]);
            MBDist_Bypass = MBDist_B;
            break;

        case 24://Arpie

            Arpie_Bypass = 0;
            efx_Arpie->cleanup();
            for (i = 0; i <= 10; i++)
                efx_Arpie->changepar (i, lv[25][i]);
            Arpie_Bypass = Arpie_B;
            break;

        case 25://Expander

            Expander_Bypass = 0;
            efx_Expander->cleanup();
            for (i = 0; i <= 6; i++)
                efx_Expander->Expander_Change (i + 1,lv[26][i]);
            Expander_Bypass = Expander_B;
            break;

        case 26://Shuffle

            Shuffle_Bypass = 0;
            efx_Shuffle->cleanup();
            for (i = 0; i <= 10; i++)
                efx_Shuffle->changepar (i, lv[27][i]);
            Shuffle_Bypass = Shuffle_B;
            break;

        case 27://Synthfilter

            Synthfilter_Bypass = 0;
            efx_Synthfilter->cleanup();
            for (i = 0; i <= 15; i++)
                efx_Synthfilter->changepar (i, lv[28][i]);
            Synthfilter_Bypass = Synthfilter_B;
            break;

        case 28://MBVvol

            MBVvol_Bypass = 0;
            efx_MBVvol->cleanup();
            for (i = 0; i <= 10; i++)
                efx_MBVvol->changepar (i, lv[29][i]);
            MBVvol_Bypass = MBVvol_B;
            break;

        case 29://Convolotron

            Convol_Bypass = 0;
            efx_Convol->cleanup();
            for (i = 0; i <= 10; i++)
                efx_Convol->changepar (i, lv[30][i]);
            Convol_Bypass = Convol_B;
            break;

        case 30://Looper

            Looper_Bypass = 0;
            // efx_Looper->cleanup();
            for (i = 0; i <= 13; i++)
                efx_Looper->loadpreset(i, lv[31][i]);
            Looper_Bypass = Looper_B;
            break;

        case 31://RyanWah

            RyanWah_Bypass = 0;
            efx_RyanWah->cleanup();
            for (i = 0; i <= 18; i++)
                efx_RyanWah->changepar (i, lv[32][i]);
            RyanWah_Bypass = RyanWah_B;
            break;

        case 32://RBEcho

            RBEcho_Bypass = 0;
            efx_RBEcho->cleanup();
            for (i = 0; i <= 9; i++)
                efx_RBEcho->changepar (i, lv[33][i]);
            RBEcho_Bypass= RBEcho_B;
            break;

        case 33://CoilCrafter

            CoilCrafter_Bypass = 0;
            efx_CoilCrafter->cleanup();
            for (i = 0; i <= 8; i++)
                efx_CoilCrafter->changepar (i, lv[34][i]);
            CoilCrafter_Bypass = CoilCrafter_B;
            break;

        case 34://ShelfBoost

            ShelfBoost_Bypass = 0;
            efx_ShelfBoost->cleanup();
            for (i = 0; i <= 4; i++)
                efx_ShelfBoost->changepar (i, lv[35][i]);
            ShelfBoost_Bypass = ShelfBoost_B;
            break;

        case 35://Vocoder

            Vocoder_Bypass = 0;
            efx_Vocoder->cleanup();
            for (i = 0; i <= 6; i++)
                efx_Vocoder->changepar (i, lv[36][i]);
            Vocoder_Bypass = Vocoder_B;
            break;

        case 36://Sustainer

            Sustainer_Bypass = 0;
            efx_Sustainer->cleanup();
            for (i = 0; i <= 1; i++)
                efx_Sustainer->changepar (i, lv[37][i]);
            Sustainer_Bypass = Sustainer_B;
            break;

        case 37://Sequence

            Sequence_Bypass = 0;
            efx_Sequence->cleanup();
            for (i = 0; i <= 14; i++)
                efx_Sequence->changepar (i, lv[38][i]);
            Sequence_Bypass = Sequence_B;
            break;

        case 38://Shifter

            Shifter_Bypass = 0;
            efx_Shifter->cleanup();
            for (i = 0; i <= 9; i++)
                efx_Shifter->changepar (i, lv[39][i]);
            Shifter_Bypass = Shifter_B;
            break;

        case 39://StompBox

            StompBox_Bypass = 0;
            efx_StompBox->cleanup();
            for (i = 0; i <= 5; i++)
                efx_StompBox->changepar (i, lv[40][i]);
            StompBox_Bypass = StompBox_B;
            break;

        case 40://Reverbtron

            Reverbtron_Bypass = 0;
            efx_Reverbtron->cleanup();
            for (i = 0; i <= 15; i++)
                efx_Reverbtron->changepar (i, lv[41][i]);
            Reverbtron_Bypass = Reverbtron_B;
            break;

        case 41://Echotron

            Echotron_Bypass = 0;
            efx_Echotron->cleanup();
            efx_Echotron->Pchange=1;
            for (i = 0; i <= 15; i++)
                efx_Echotron->changepar (i, lv[42][i]);
            efx_Echotron->Pchange=0;
            Echotron_Bypass = Echotron_B;
            break;

        case 42://StereoHarm

            StereoHarm_Bypass = 0;
            efx_StereoHarm->cleanup();
            for (i = 0; i <= 11; i++)
                efx_StereoHarm->changepar (i, lv[43][i]);
            if (lv[43][10]) RC->cleanup ();
            StereoHarm_Bypass = StereoHarm_B;
            break;

        case 43://CompBand

            CompBand_Bypass = 0;
            efx_CompBand->cleanup();
            for (i = 0; i <= 12; i++)
                efx_CompBand->changepar (i, lv[44][i]);
            CompBand_Bypass = CompBand_B;
            break;

        case 44://OpticalTrem

            Opticaltrem_Bypass = 0;
            efx_Opticaltrem->cleanup();
            for (i = 0; i <= 6; i++)
                efx_Opticaltrem->changepar (i, lv[45][i]);
            Opticaltrem_Bypass = Opticaltrem_B;
            break;

        case 45://Vibe

            Vibe_Bypass = 0;
            efx_Vibe->cleanup();
            for (i = 0; i <= 10; i++)
                efx_Vibe->changepar (i, lv[46][i]);
            Vibe_Bypass = Vibe_B;
            break;

        case 46://Infinity

            Infinity_Bypass = 0;
            efx_Infinity->cleanup();
            for (i = 0; i <= 17; i++)
                efx_Infinity->changepar (i, lv[47][i]);
            Infinity_Bypass = Infinity_B;
            break;

        }
    }



    Bypass = Bypass_B;
    if(needtoloadstate) {
        calculavol(1);
        calculavol(2);
    }

}


void
RKR::loadnames()
{
    int j,k;
    FILE *fn;
    char temp[128];

    memset(B_Names,0,sizeof(B_Names));


    for(k=0; k<4; k++) {

        switch(k) {

        case 0:
            memset (temp, 0, sizeof (temp));
            sprintf (temp, "%s/Default.rkrb", DATADIR);
            break;

        case 1:
            memset (temp, 0, sizeof (temp));
            sprintf (temp, "%s/Extra.rkrb", DATADIR);
            break;

        case 2:
            memset (temp, 0, sizeof (temp));
            sprintf (temp, "%s/Extra1.rkrb", DATADIR);
            break;

        case 3:
            memset (temp, 0, sizeof (temp));
            sprintf (temp, "%s",BankFilename);
            break;

        }



        if ((fn = fopen (temp, "rb")) != NULL) {
            New_Bank();
            while (!feof (fn)) {
                fread (&Bank, sizeof (Bank), 1, fn);
                for(j=1; j<=60; j++) strcpy(B_Names[k][j].Preset_Name,Bank[j].Preset_Name);
            }
            fclose (fn);
        }

    }

}

int
RKR::loadbank (char *filename)
{

    int err_message=1;
    char meslabel[64];
    FILE *fn;


    memset(meslabel,0, sizeof(meslabel));
    sprintf(meslabel, "%s %s",jackcliname,VERSION);


    err_message = CheckOldBank(filename);

    switch(err_message) {
    case 0:
        break;
    case 1:
        Message(1, meslabel, "Can not load this Bank file because is from a old rakarrack version,\n please use 'Convert Old Bank' menu entry in the Bank window.");
        return(0);
        break;
    case 2:
        Message(1, meslabel, "Can not load this Bank file\n");
        return(0);
        break;
    case 3:
        Message(1, meslabel, "Can not load this Bank file because is from a old rakarrack git version,\n please use rakgit2new utility to convert.");
        return(0);
        break;

    }


    if ((fn = fopen (filename, "rb")) != NULL) {
        New_Bank();
        while (!feof (fn)) {
            fread (&Bank, sizeof (Bank), 1, fn);
        }
        fclose (fn);
        if(BigEndian()) fix_endianess();
        convert_IO();
        modified=0;
        new_bank_loaded=1;
        return (1);
    }
    return (0);
};


int
RKR::savebank (char *filename)
{

    FILE *fn;

    if ((fn = fopen (filename, "wb")) != NULL) {
        copy_IO();
        if(BigEndian()) fix_endianess();
        fwrite (&Bank, sizeof (Bank), 1, fn);
        if(BigEndian()) fix_endianess();
        fclose (fn);
        modified=0;
        return(1);
    }

    if(errno==EACCES) Error_Handle(3);
    return (0);
};

void
RKR::New ()
{

    int j, k;

    int presets[48][16] = {
//Reverb
        {80, 64, 63, 24, 0, 0, 0, 85, 5, 83, 1, 64, 0, 0, 0, 0},
//Echo
        {67, 64, 35, 64, 30, 59, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//Chorus
        {64, 64, 33, 0, 0, 90, 40, 85, 64, 119, 0, 0, 0, 0, 0, 0},
//Flanger
        {64, 64, 39, 0, 0, 60, 23, 3, 62, 0, 0, 0, 0, 0, 0, 0},
//Phaser
        {64, 64, 11, 0, 0, 64, 110, 64, 1, 0, 0, 20, 0, 0, 0, 0},
//Overdrive
        {84, 64, 35, 56, 40, 0, 0, 6703, 21, 0, 0, 0, 0, 0, 0, 0},
//Distorsion
        {0, 64, 0, 87, 14, 6, 0, 3134, 157, 0, 1, 0, 0, 0, 0, 0},
//EQ1
        {64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 0, 0, 0, 0},
//EQ2
        {24, 64, 64, 75, 64, 64, 113, 64, 64, 64, 0, 0, 0, 0, 0, 0},
//Compressor
        {-13, 2, -6, 20, 120, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//Order
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
//WahWah
        {64, 64, 138, 0, 0, 64, 20, 90, 0, 60, 0, 0, 0, 0, 0, 0},
//AlienWah1
        {64, 64, 80, 0, 0, 62, 60, 105, 25, 0, 64, 0, 0, 0, 0, 0},
//Cabinet
        {0, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//Pan
        {64, 64, 26, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
//Harmonizer
        {64, 64, 64, 12, 6000, 0, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0},
//MusicDelay
        {64, 0, 2, 7, 0, 59, 0, 127, 4, 59, 106, 75, 75, 0, 0, 0},
//NoiseGate
        {0, 0, 1, 2, 6703, 76, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//NewDist
        {0, 64, 64, 83, 15, 15, 0, 2437, 169, 68, 0, 0, 0, 0, 0, 0},
//APhaser
        {64, 20, 14, 0, 1, 64, 110, 40, 4, 10, 0, 64, 1, 0, 0, 0},
//Valve
        {0, 64, 64, 127, 64, 0, 5841, 61, 1, 0, 69, 1, 80 ,0 ,0 ,0},
//Dual Flange
        {-32, 0, 0, 110, 800, 10, -27, 16000, 1, 0, 24, 64, 1, 10, 0, 0},
//Ring
        {-64, 0, -64, 64, 35, 1, 0, 20, 0, 40, 0, 64, 1, 0, 0 ,0},
//Exciter
        {127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20000, 20, 0, 0, 0 },
//MBDist
        {0, 64, 64, 56, 40, 0, 0, 0, 29, 35, 100, 0, 450, 1500, 1, 0},
//Arpie
        {67, 64, 35, 64, 30, 59, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0},
//Expander
        {-50, 20, 50, 50, 3134, 76, 0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
//Shuffle 1
        {64, 10, 0, 0, 0, 600, 1200, 2000, 6000,-14, 1, 0, 0 ,0 ,0, 0},
//Synthfilter
        {0, 20, 14, 0, 1, 64, 110, -40, 6, 0, 0, 32, -32, 500, 100, 0},
//MBVvol
        {0, 40, 0, 64, 80, 0, 0, 500, 2500, 5000, 0, 0, 0, 0, 0, 0},
//Convolotron
        {67, 64, 1, 100, 0, 64, 30, 20, 0, 0, 0, 0, 0, 0, 0, 0},
//Looper
        {64, 0, 1, 0, 1, 0, 64, 1, 0, 0, 64, 0, 0, 0, 0, 0},
//RyanWah
        {16, 10, 60, 0, 0, 64, 0, 0, 10, 7, -16, 40, -3, 1, 2000, 450},
//Echoverse
        {64, 64, 90, 64, 64, 64, 64, 0, 1, 96, 0, 0, 0, 0, 0, 0},
//CoilCrafter
        {32, 6, 1, 3300, 16, 4400, 42, 20, 0, 0, 0, 0, 0, 0, 0, 0},
//ShelfBoost
        {127, 64, 16000, 1, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//Vocoder
        {0, 64, 10, 70, 70, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//Systainer
        {67, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//Sequence
        {20, 100, 10, 50, 25, 120, 60, 127, 0, 90, 40, 0, 0, 0, 3, 0},
//Shifter
        {0, 64, 64, 200, 200, -20, 2, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0},
//StompBox
        {48, 32, 0, 32, 65, 0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
//Reverbtron
        {64, 0, 1, 1500, 0, 0, 60, 18, 4, 0, 0, 64, 0 ,0 ,0 ,0},
//Echotron
        {64, 45, 34, 4, 0, 76, 3, 41, 0, 96, -13, 64, 1, 1, 1, 1},
//StereoHarm
        {64, 64, 12, 0, 64, 12, 0, 0, 0, 0, 0, 64, 0, 0, 0, 0},
//CompBand
        {0, 16, 16, 16, 16, 0, 0, 0, 0, 1000, 5000, 10000, 48, 0, 0, 0},
//Opticaltrem
        {127, 260, 10, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//Vibe
        {35, 120, 10, 0, 64, 64, 64, 64, 3, 64, 0, 0, 0, 0, 0, 0},
//Infinity
        {64, 64, 64, 64, 64, 64, 64, 64, 64, 700, 20, 80, 60, 0, 1, 0}







    };


    for (j=0; j<10; j++) active[j]=0;

    memset(Preset_Name, 0,sizeof (Preset_Name));
    memset(efx_Convol->Filename,0,sizeof(efx_Convol->Filename));
    memset(efx_Reverbtron->Filename,0,sizeof(efx_Reverbtron->Filename));
    memset(efx_Echotron->Filename,0,sizeof(efx_Echotron->Filename));
    memset (Author,0, sizeof (Author));
    strcpy(Author,UserRealName);
    Input_Gain = .5f;
    Master_Volume = .5f;
    Fraction_Bypass = 1.0f;
    Bypass = 0;
    memset(lv, 0 , sizeof(lv));

    for (j = 0; j < NumEffects; j++) {
        for (k = 0; k < 16; k++) {
            lv[j][k] = presets[j][k];

        }

        lv[j][19] = 0;

    }


    for (k = 0; k < 12; k++)
        efx_order[k] = presets[10][k];



    Reverb_B = 0;
    Echo_B = 0;
    Chorus_B = 0;
    Flanger_B = 0;
    Phaser_B = 0;
    Overdrive_B = 0;
    Distorsion_B = 0;
    EQ1_B = 0;
    EQ2_B = 0;
    Compressor_B = 0;
    WhaWha_B = 0;
    Alienwah_B = 0;
    Cabinet_B = 0;
    Pan_B = 0;
    Harmonizer_B = 0;
    MusDelay_B = 0;
    Gate_B = 0;
    NewDist_B = 0;
    APhaser_B = 0;
    Valve_B = 0;
    DFlange_B = 0;
    Ring_B = 0;
    Exciter_B = 0;
    MBDist_B = 0;
    Arpie_B = 0;
    Expander_B = 0;
    Shuffle_B = 0;
    Synthfilter_B = 0;
    MBVvol_B = 0;
    Convol_B = 0;
    Looper_B = 0;
    RyanWah_B = 0;
    RBEcho_B = 0;
    CoilCrafter_B = 0;
    ShelfBoost_B = 0;
    Vocoder_B = 0;
    Sustainer_B = 0;
    Sequence_B = 0;
    Shifter_B = 0;
    StompBox_B = 0;
    Reverbtron_B = 0;
    Echotron_B = 0;
    StereoHarm_B = 0;
    CompBand_B = 0;
    Opticaltrem_B = 0;
    Vibe_B = 0;
    Infinity_B=0;

    Bypass_B = 0;


    memset(XUserMIDI,0,sizeof(XUserMIDI));





    Actualizar_Audio ();


};



void
RKR::New_Bank ()
{

    int i, j, k;

    int presets[48][16] = {
//Reverb
        {80, 64, 63, 24, 0, 0, 0, 85, 5, 83, 1, 64, 0, 0, 0, 0},
//Echo
        {67, 64, 35, 64, 30, 59, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//Chorus
        {64, 64, 33, 0, 0, 90, 40, 85, 64, 119, 0, 0, 0, 0, 0, 0},
//Flanger
        {64, 64, 39, 0, 0, 60, 23, 3, 62, 0, 0, 0, 0, 0, 0, 0},
//Phaser
        {64, 64, 11, 0, 0, 64, 110, 64, 1, 0, 0, 20, 0, 0, 0, 0},
//Overdrive
        {84, 64, 35, 56, 40, 0, 0, 6703, 21, 0, 0, 0, 0, 0, 0, 0},
//Distorsion
        {0, 64, 0, 87, 14, 6, 0, 3134, 157, 0, 1, 0, 0, 0, 0, 0},
//EQ1
        {64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 0, 0, 0, 0},
//EQ2
        {24, 64, 64, 75, 64, 64, 113, 64, 64, 64, 0, 0, 0, 0, 0, 0},
//Compressor
        {-30, 2, -6, 20, 120, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//Order
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
//WahWah
        {64, 64, 138, 0, 0, 64, 20, 90, 0, 60, 0, 0, 0, 0, 0, 0},
//AlienWah1
        {64, 64, 80, 0, 0, 62, 60, 105, 25, 0, 64, 0, 0, 0, 0, 0},
//Cabinet
        {0, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//Pan
        {64, 64, 26, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
//Harmonizer
        {64, 64, 64, 12, 6000, 0, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0},
//MusicDelay
        {64, 0, 2, 7, 0, 59, 0, 127, 4, 59, 106, 75, 75, 0, 0, 0},
//NoiseGate
        {0, 0, 1, 2, 6703, 76, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//NewDist
        {0, 64, 64, 83, 65, 15, 0, 2437, 169, 68, 0, 0, 0, 0, 0, 0},
//APhaser
        {64, 20, 14, 0, 1, 64, 110, 40, 4, 10, 0, 64, 1, 0, 0, 0},
//Valve
        {0, 64, 64, 127, 64, 0, 5841, 61, 1, 0, 69, 1, 80 ,0 ,0 ,0},
//Dual Flange
        {-32, 0, 0, 110, 800, 10, -27, 16000, 1, 0, 24, 64, 1, 10, 0, 0},
//Ring
        {-64, 0, -64, 64, 35, 1, 0, 20, 0, 40, 0, 64, 1, 0, 0 ,0},
//Exciter
        {127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20000, 20, 0, 0, 0 },
//MBDist
        {0, 64, 64, 56, 40, 0, 0, 0, 29, 35, 100, 0, 450, 1500, 1, 0},
//Arpie
        {67, 64, 35, 64, 30, 59, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0},
//Expander
        {-50, 20, 50, 50, 3134, 76, 0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
//Shuffle 1
        {64, 10, 0, 0, 0, 600, 1200, 2000, 6000,-14, 1, 0, 0 ,0 ,0, 0},
//Synthfilter
        {0, 20, 14, 0, 1, 64, 110, -40, 6, 0, 0, 32, -32, 500, 100, 0},
//MBVvol
        {0, 40, 0, 64, 80, 0, 0, 500, 2500, 5000, 0, 0, 0, 0, 0, 0},
//Convolotron 1
        {67, 64, 1, 100, 0, 64, 30, 20, 0, 0, 0, 0, 0, 0, 0, 0},
//Looper
        {64, 0, 1, 0, 1, 0, 64, 1, 0, 0, 64, 0, 0, 0, 0, 0},
//RyanWah
        {16, 10, 60, 0, 0, 64, 0, 0, 10, 7, -16, 40, -3, 1, 2000, 450},
//Echoverse
        {64, 64, 90, 64, 64, 64, 64, 0, 1, 96, 0, 0, 0, 0, 0, 0},
//CoilCrafter
        {32, 6, 1, 3300, 16, 4400, 42, 20, 0, 0, 0, 0, 0, 0, 0, 0},
//ShelfBoost
        {127, 64, 16000, 1, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//Vocoder
        {0, 64, 10, 70, 70, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//Systainer
        {67, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//Sequence
        {20, 100, 10, 50, 25, 120, 60, 127, 0, 90, 40, 0, 0, 0, 3, 0},
//Shifter
        {0, 64, 64, 200, 200, -20, 2, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0},
//StompBox
        {48, 32, 0, 32, 65, 0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
//Reverbtron
        {64, 0, 1, 1500, 0, 0, 60, 18, 4, 0, 0, 64, 0 ,0 ,0 ,0},
//Echotron
        {64, 45, 34, 4, 0, 76, 3, 41, 0, 96, -13, 64, 1, 1, 1, 1},
//StereoHarm
        {64, 64, 12, 0, 64, 12, 0, 0, 0, 0, 0, 64, 0, 0, 0, 0},
//CompBand
        {0, 16, 16, 16, 16, 0, 0, 0, 0, 1000, 5000, 10000, 48, 0, 0, 0},
//Opticaltrem
        {127, 260, 10, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//Vibe
        {35, 120, 10, 0, 64, 64, 64, 64, 3, 64, 0, 0, 0, 0, 0, 0},
//Infinity
        {64, 64, 64, 64, 64, 64, 64, 64, 64, 700, 20, 80, 60, 0, 1, 0}








    };




    for (i = 0; i < 62; i++) {
        memset(Bank[i].Preset_Name, 0, sizeof (Bank[i].Preset_Name));
        memset(Bank[i].Author, 0, sizeof (Bank[i].Author));
        strcpy(Bank[i].Author,UserRealName);
        memset(Bank[i].ConvoFiname,0, sizeof(Bank[i].ConvoFiname));
        memset(Bank[i].RevFiname,0,sizeof(Bank[i].RevFiname));
        memset(Bank[i].EchoFiname,0,sizeof(Bank[i].EchoFiname));

        Bank[i].Input_Gain = .5f;
        Bank[i].Master_Volume = .5f;
        Bank[i].Balance = 1.0f;
        Bank[i].Bypass = 0;
        memset(Bank[i].lv , 0 , sizeof(Bank[i].lv));

        for (j = 0; j < NumEffects; j++) {
            for (k = 0; k < 16; k++) {
                Bank[i].lv[j][k] = presets[j][k];
            }
            Bank[i].lv[j][19] =0;

        }

        memset(Bank[i].XUserMIDI, 0, sizeof(Bank[i].XUserMIDI));

    }




};


void
RKR::Bank_to_Preset (int i)
{

    int j, k;


    memset(Preset_Name, 0,sizeof (Preset_Name));
    strcpy (Preset_Name, Bank[i].Preset_Name);
    memset(Author, 0,sizeof (Author));
    strcpy (Author, Bank[i].Author);
    memset(efx_Convol->Filename, 0, sizeof (efx_Convol->Filename));
    strcpy (efx_Convol->Filename,Bank[i].ConvoFiname);
    memset(efx_Reverbtron->Filename, 0, sizeof (efx_Reverbtron->Filename));
    strcpy (efx_Reverbtron->Filename,Bank[i].RevFiname);
    memset(efx_Echotron->Filename, 0, sizeof (efx_Echotron->Filename));
    strcpy (efx_Echotron->Filename,Bank[i].EchoFiname);


    for (j = 0; j <=NumEffects; j++) {
        for (k = 0; k < 20; k++) {
            lv[j][k] = Bank[i].lv[j][k];
        }
    }


    for (k = 0; k < 12; k++)
        efx_order[k] = Bank[i].lv[10][k];


    Reverb_B = Bank[i].lv[0][19];
    Echo_B = Bank[i].lv[1][19];
    Chorus_B = Bank[i].lv[2][19];
    Flanger_B = Bank[i].lv[3][19];
    Phaser_B = Bank[i].lv[4][19];
    Overdrive_B = Bank[i].lv[5][19];
    Distorsion_B = Bank[i].lv[6][19];
    EQ1_B = Bank[i].lv[7][19];
    EQ2_B = Bank[i].lv[8][19];
    Compressor_B = Bank[i].lv[9][19];
    WhaWha_B = Bank[i].lv[11][19];
    Alienwah_B = Bank[i].lv[12][19];
    Cabinet_B = Bank[i].lv[13][19];
    Pan_B = Bank[i].lv[14][19];
    Harmonizer_B = Bank[i].lv[15][19];
    MusDelay_B = Bank[i].lv[16][19];
    Gate_B = Bank[i].lv[17][19];
    NewDist_B = Bank[i].lv[18][19];
    APhaser_B = Bank[i].lv[19][19];
    Valve_B = Bank[i].lv[20][19];
    DFlange_B = Bank[i].lv[21][19];
    Ring_B = Bank[i].lv[22][19];
    Exciter_B = Bank[i].lv[23][19];
    MBDist_B = Bank[i].lv[24][19];
    Arpie_B = Bank[i].lv[25][19];
    Expander_B = Bank[i].lv[26][19];
    Shuffle_B = Bank[i].lv[27][19];
    Synthfilter_B = Bank[i].lv[28][19];
    MBVvol_B = Bank[i].lv[29][19];
    Convol_B = Bank[i].lv[30][19];
    Looper_B = Bank[i].lv[31][19];
    RyanWah_B = Bank[i].lv[32][19];
    RBEcho_B = Bank[i].lv[33][19];
    CoilCrafter_B = Bank[i].lv[34][19];
    ShelfBoost_B = Bank[i].lv[35][19];
    Vocoder_B = Bank[i].lv[36][19];
    Sustainer_B = Bank[i].lv[37][19];
    Sequence_B = Bank[i].lv[38][19];
    Shifter_B = Bank[i].lv[39][19];
    StompBox_B = Bank[i].lv[40][19];
    Reverbtron_B = Bank[i].lv[41][19];
    Echotron_B = Bank[i].lv[42][19];
    StereoHarm_B = Bank[i].lv[43][19];
    CompBand_B = Bank[i].lv[44][19];
    Opticaltrem_B = Bank[i].lv[45][19];
    Vibe_B = Bank[i].lv[46][19];
    Infinity_B = Bank[i].lv[47][19];


    Bypass_B = Bypass;


    memcpy(XUserMIDI, Bank[i].XUserMIDI, sizeof(XUserMIDI));



    Actualizar_Audio ();

    if (actuvol == 0) {
        Input_Gain = Bank[i].Input_Gain;
        Master_Volume = Bank[i].Master_Volume;
        Fraction_Bypass = Bank[i].Balance;
    }

    if((Tap_Updated) && (Tap_Bypass) && (Tap_TempoSet>0) && (Tap_TempoSet<601)) Update_tempo();

};


void
RKR::Preset_to_Bank (int i)
{


    int j, k;
    memset(Bank[i].Preset_Name, 0, sizeof (Bank[i].Preset_Name));
    strcpy (Bank[i].Preset_Name, Preset_Name);
    memset(Bank[i].Author, 0, sizeof (Bank[i].Author));
    strcpy (Bank[i].Author, Author);
    memset(Bank[i].ConvoFiname,0, sizeof(Bank[i].ConvoFiname));
    strcpy(Bank[i].ConvoFiname, efx_Convol->Filename);
    memset(Bank[i].RevFiname, 0, sizeof(Bank[i].RevFiname));
    strcpy(Bank[i].RevFiname, efx_Reverbtron->Filename);
    memset(Bank[i].EchoFiname, 0, sizeof(Bank[i].EchoFiname));
    strcpy(Bank[i].EchoFiname, efx_Echotron->Filename);


    Bank[i].Input_Gain = Input_Gain;
    Bank[i].Master_Volume = Master_Volume;
    Bank[i].Balance = Fraction_Bypass;


    for (j = 0; j <= 11; j++)
        lv[0][j] = efx_Rev->getpar (j);
    for (j = 0; j <= 8; j++)
        lv[1][j] = efx_Echo->getpar (j);
    for (j = 0; j <= 12; j++)
        lv[2][j] = efx_Chorus->getpar (j);
    for (j = 0; j <= 12; j++)
        lv[3][j] = efx_Flanger->getpar (j);
    for (j = 0; j <= 11; j++)
        lv[4][j] = efx_Phaser->getpar (j);
    for (j = 0; j <= 12; j++)
        lv[5][j] = efx_Overdrive->getpar (j);
    for (j = 0; j <= 12; j++)
        lv[6][j] = efx_Distorsion->getpar (j);
    for (j = 0; j <= 8; j++)
        lv[9][j] = efx_Compressor->getpar (j + 1);
    for (j = 0; j <= 9; j++)
        lv[11][j] = efx_WhaWha->getpar (j);
    for (j = 0; j <= 10; j++)
        lv[12][j] = efx_Alienwah->getpar (j);
    for (j = 0; j <= 8; j++)
        lv[14][j] = efx_Pan->getpar (j);
    for (j = 0; j <= 10; j++)
        lv[15][j] = efx_Har->getpar (j);
    for (j = 0; j <= 12; j++)
        lv[16][j] = efx_MusDelay->getpar (j);
    for (j = 0; j <= 6; j++)
        lv[17][j] = efx_Gate->getpar (j + 1);
    for (j = 0; j <= 11; j++)
        lv[18][j] = efx_NewDist->getpar (j);
    for (j = 0; j <= 12; j++)
        lv[19][j] = efx_APhaser->getpar(j);
    for (j = 0; j <= 12; j++)
        lv[20][j] = efx_Valve->getpar(j);
    for (j = 0; j <= 14; j++)
        lv[21][j] = efx_DFlange->getpar(j);
    for (j = 0; j <= 12; j++)
        lv[22][j] = efx_Ring->getpar(j);
    for (j = 0; j <= 12; j++)
        lv[23][j] = efx_Exciter->getpar(j);
    for (j = 0; j <= 14; j++)
        lv[24][j] = efx_MBDist->getpar(j);
    for (j = 0; j <= 10; j++)
        lv[25][j] = efx_Arpie->getpar(j);
    for (j = 0; j <= 6; j++)
        lv[26][j] = efx_Expander->getpar(j+1);
    for (j = 0; j <= 10; j++)
        lv[27][j] = efx_Shuffle->getpar(j);
    for (j = 0; j <= 15; j++)
        lv[28][j] = efx_Synthfilter->getpar(j);
    for (j = 0; j <= 10; j++)
        lv[29][j] = efx_MBVvol->getpar(j);
    for (j = 0; j <= 10; j++)
        lv[30][j] = efx_Convol->getpar(j);
    for (j = 0; j <= 13; j++)
        lv[31][j] = efx_Looper->getpar(j);
    for (j = 0; j <= 18; j++)
        lv[32][j] = efx_RyanWah->getpar(j);
    for (j = 0; j <= 9; j++)
        lv[33][j] = efx_RBEcho->getpar(j);
    for (j = 0; j <= 8; j++)
        lv[34][j] = efx_CoilCrafter->getpar(j);
    for (j = 0; j <= 4; j++)
        lv[35][j] = efx_ShelfBoost->getpar(j);
    for (j = 0; j <= 6; j++)
        lv[36][j] = efx_Vocoder->getpar(j);
    for (j = 0; j <= 1; j++)
        lv[37][j] = efx_Sustainer->getpar(j);
    for (j = 0; j <= 14; j++)
        lv[38][j] = efx_Sequence->getpar(j);
    for (j = 0; j <= 9; j++)
        lv[39][j] = efx_Shifter->getpar(j);
    for (j = 0; j <= 5; j++)
        lv[40][j] = efx_StompBox->getpar(j);
    for (j = 0; j <= 15; j++)
        lv[41][j] = efx_Reverbtron->getpar(j);
    for (j = 0; j <= 15; j++)
        lv[42][j] = efx_Echotron->getpar(j);
    for (j = 0; j <= 11; j++)
        lv[43][j] = efx_StereoHarm->getpar(j);
    for (j = 0; j <= 12; j++)
        lv[44][j] = efx_CompBand->getpar(j);
    for (j = 0; j <= 6; j++)
        lv[45][j] = efx_Opticaltrem->getpar(j);
    for (j = 0; j <= 10; j++)
        lv[46][j] = efx_Vibe->getpar(j);
    for (j = 0; j <= 17; j++)
        lv[47][j] = efx_Infinity->getpar(j);


    for (j = 0; j <= 12; j++)
        lv[10][j] = efx_order[j];

    for (j = 0; j < 10; j++)
        lv[7][j] = efx_EQ1->getpar (j * 5 + 12);
    lv[7][10] = efx_EQ1->getpar (0);
    lv[7][11] = efx_EQ1->getpar (13);

    for (j = 0; j < 3; j++) {
        lv[8][0 + j * 3] = efx_EQ2->getpar (j * 5 + 11);
        lv[8][1 + j * 3] = efx_EQ2->getpar (j * 5 + 12);
        lv[8][2 + j * 3] = efx_EQ2->getpar (j * 5 + 13);
    }
    lv[8][9] = efx_EQ2->getpar (0);

    lv[13][0] = Cabinet_Preset;
    lv[13][1] = efx_Cabinet->getpar (0);



    for (j = 0; j <= NumEffects; j++) {
        for (k = 0; k < 19; k++) {
            Bank[i].lv[j][k] = lv[j][k];
        }
    }

    Bank[i].lv[11][10] = efx_WhaWha->Ppreset;


    Bank[i].lv[0][19] = Reverb_Bypass;
    Bank[i].lv[1][19] = Echo_Bypass;
    Bank[i].lv[2][19] = Chorus_Bypass;
    Bank[i].lv[3][19] = Flanger_Bypass;
    Bank[i].lv[4][19] = Phaser_Bypass;
    Bank[i].lv[5][19] = Overdrive_Bypass;
    Bank[i].lv[6][19] = Distorsion_Bypass;
    Bank[i].lv[7][19] = EQ1_Bypass;
    Bank[i].lv[8][19] = EQ2_Bypass;
    Bank[i].lv[9][19] = Compressor_Bypass;
    Bank[i].lv[11][19] = WhaWha_Bypass;
    Bank[i].lv[12][19] = Alienwah_Bypass;
    Bank[i].lv[13][19] = Cabinet_Bypass;
    Bank[i].lv[14][19] = Pan_Bypass;
    Bank[i].lv[15][19] = Harmonizer_Bypass;
    Bank[i].lv[16][19] = MusDelay_Bypass;
    Bank[i].lv[17][19] = Gate_Bypass;
    Bank[i].lv[18][19] = NewDist_Bypass;
    Bank[i].lv[19][19] = APhaser_Bypass;
    Bank[i].lv[20][19] = Valve_Bypass;
    Bank[i].lv[21][19] = DFlange_Bypass;
    Bank[i].lv[22][19] = Ring_Bypass;
    Bank[i].lv[23][19] = Exciter_Bypass;
    Bank[i].lv[24][19] = MBDist_Bypass;
    Bank[i].lv[25][19] = Arpie_Bypass;
    Bank[i].lv[26][19] = Expander_Bypass;
    Bank[i].lv[27][19] = Shuffle_Bypass;
    Bank[i].lv[28][19] = Synthfilter_Bypass;
    Bank[i].lv[29][19] = MBVvol_Bypass;
    Bank[i].lv[30][19] = Convol_Bypass;
    Bank[i].lv[31][19] = Looper_Bypass;
    Bank[i].lv[32][19] = RyanWah_Bypass;
    Bank[i].lv[33][19] = RBEcho_Bypass;
    Bank[i].lv[34][19] = CoilCrafter_Bypass;
    Bank[i].lv[35][19] = ShelfBoost_Bypass;
    Bank[i].lv[36][19] = Vocoder_Bypass;
    Bank[i].lv[37][19] = Sustainer_Bypass;
    Bank[i].lv[38][19] = Sequence_Bypass;
    Bank[i].lv[39][19] = Shifter_Bypass;
    Bank[i].lv[40][19] = StompBox_Bypass;
    Bank[i].lv[41][19] = Reverbtron_Bypass;
    Bank[i].lv[42][19] = Echotron_Bypass;
    Bank[i].lv[43][19] = StereoHarm_Bypass;
    Bank[i].lv[44][19] = CompBand_Bypass;
    Bank[i].lv[45][19] = Opticaltrem_Bypass;
    Bank[i].lv[46][19] = Vibe_Bypass;
    Bank[i].lv[47][19] = Infinity_Bypass;


    memcpy(Bank[i].XUserMIDI,XUserMIDI,sizeof(XUserMIDI));


};



int
RKR::BigEndian()
{
    long one= 1;
    return !(*((char *)(&one)));
}

void
RKR::copy_IO()
{

    int i;

    for(i=0; i<62; i++) {
        memset(Bank[i].cInput_Gain, 0, sizeof(Bank[i].cInput_Gain));
        sprintf(Bank[i].cInput_Gain, "%f", Bank[i].Input_Gain);
        memset(Bank[i].cMaster_Volume, 0,sizeof(Bank[i].cMaster_Volume));
        sprintf(Bank[i].cMaster_Volume, "%f", Bank[i].Master_Volume);
        memset(Bank[i].cBalance, 0, sizeof(Bank[i].cBalance));
        sprintf(Bank[i].cBalance, "%f", Bank[i].Balance);



    }



}

void
RKR::convert_IO()
{

    int i;

    for(i=0; i<62; i++) {
        sscanf(Bank[i].cInput_Gain, "%f", &Bank[i].Input_Gain);
        if(Bank[i].Input_Gain == 0.0) Bank[i].Input_Gain=0.5f;

        sscanf(Bank[i].cMaster_Volume, "%f", &Bank[i].Master_Volume);
        if(Bank[i].Master_Volume == 0.0) Bank[i].Master_Volume=0.5f;

        sscanf(Bank[i].cBalance, "%f", &Bank[i].Balance);
        if(Bank[i].Balance == 0.0) Bank[i].Balance=1.0f;



    }




}

void
RKR::fix_endianess()
{

    int i,j,k;
    unsigned int data;

    for(i=0; i<62; i++) {

        data = Bank[i].Bypass;
        data = SwapFourBytes(data);
        Bank[i].Bypass=data;

        for(j=0; j<70; j++) {
            for(k=0; k<20; k++) {
                data = Bank[i].lv[j][k];
                data = SwapFourBytes(data);
                Bank[i].lv[j][k]=data;
            }

        }

        for(j=0; j<128; j++) {
            for(k=0; k<20; k++) {
                data = Bank[i].XUserMIDI[j][k];
                data = SwapFourBytes(data);
                Bank[i].XUserMIDI[j][k]=data;
            }

        }






    }



}


void
RKR::saveskin (char *filename)
{


    FILE *fn;
    char buf[256];
    fn = fopen (filename, "w");
    if(errno == EACCES) {
        Error_Handle(3);
        fclose(fn);
        return;
    }


    memset (buf, 0, sizeof (buf));
    sprintf (buf, "%d,%d\n", resolution,sh);
    fputs (buf, fn);

    memset (buf, 0, sizeof (buf));
    sprintf (buf, "%d,%d,%d,%d\n", sback_color,sfore_color,slabel_color,sleds_color);
    fputs (buf, fn);


    memset (buf, 0, sizeof (buf));
    sprintf (buf, "%s", BackgroundImage);
    fputs (buf, fn);
    fputs ("\n",fn);

    memset(buf, 0, sizeof (buf));
    sprintf (buf, "%d,%d\n", relfontsize,font);
    fputs (buf, fn);

    memset(buf, 0, sizeof (buf));
    sprintf (buf, "%d\n", sschema);
    fputs (buf, fn);

    fclose (fn);

}



void
RKR::loadskin (char *filename)
{
    unsigned int i;
    char buf[256];
    FILE *fn;

    if ((fn = fopen (filename, "r")) == NULL)
        return;

    memset (buf, 0, sizeof (buf));
    fgets (buf, sizeof buf, fn);
    sscanf (buf, "%d,%d\n", &resolution, &sh);

    memset (buf, 0, sizeof (buf));
    fgets (buf, sizeof buf, fn);
    sscanf (buf, "%d,%d,%d,%d\n", &sback_color,&sfore_color,&slabel_color,&sleds_color);

    memset (BackgroundImage, 0, sizeof(BackgroundImage));
    memset (buf, 0, sizeof (buf));
    fgets (buf, sizeof buf, fn);

    for(i=0; i<256; i++) if(buf[i]>20) BackgroundImage[i]=buf[i];

    memset (buf, 0, sizeof (buf));
    fgets (buf, sizeof buf, fn);
    sscanf (buf, "%d,%d\n", &relfontsize,&font);

    memset (buf, 0, sizeof (buf));
    fgets (buf, sizeof buf, fn);
    sscanf (buf, "%d\n", &sschema);


    fclose(fn);

}

void
RKR::dump_preset_names (void)
{
    int i;

    for (i = 0; i < 62; i++) {
        fprintf(stderr,
                "RKR_BANK_NAME:%d:%s\n",
                i,
                Bank[i].Preset_Name);
    }

}


int
RKR::CheckOldBank(char *filename)
{

    long Length;
    FILE *fs;

    if ((fs = fopen (filename, "r")) != NULL) {
        ftell(fs);
        fseek(fs, 0L, SEEK_END);
        Length = ftell(fs);
        fclose(fs);
        if (Length == 993488) return(3);
        if (Length != 1092688) return (1);
        else return(0);
    }

    return(2);
}


void
RKR::ConvertOldFile(char * filename)
{

    char buff[255];
    memset(buff,0,sizeof(buff));
    sprintf(buff,"rakconvert -c '%s'",filename);
    system(buff);

}

void
RKR::ConvertReverbFile(char * filename)
{
    char buff[255];
    memset(buff,0, sizeof(buff));
    sprintf(buff,"rakverb -i '%s'",filename);
    printf("%s\n",buff);
    system(buff);
}


void
RKR::SaveIntPreset(int num,char *name)
{
    FILE *fn;
    char tempfile[256];
    char buf[256];
    char sbuf[256];
    memset(tempfile,0,sizeof(tempfile));
    sprintf (tempfile, "%s%s", getenv ("HOME"), "/.rkrintpreset");

    if (( fn = fopen (tempfile, "a")) != NULL) {
        memset(buf,0,sizeof(buf));
        getbuf(buf,num);
        memset(sbuf,0,sizeof(sbuf));
        sprintf(sbuf,"%d,%s,%s",num,name,buf);
        fputs(sbuf,fn);
        fclose(fn);
    }
}





void
RKR::DelIntPreset(int num, char *name)
{
    FILE *fn;
    FILE *fs;
    char *rname;
    int eff=0;
    char orden[1024];
    char tempfile[256];
    char tempfile2[256];
    char buf[256];
    char rbuf[256];

    char *sbuf;
    memset(tempfile,0,sizeof(tempfile));
    memset(tempfile2,0,sizeof(tempfile2));
    memset(orden,0,sizeof(orden));


    sprintf (tempfile, "%s%s", getenv ("HOME"), "/.rkrintpreset");
    if (( fs = fopen (tempfile, "r")) == NULL) return;

    sprintf (tempfile2, "%s%s", getenv ("HOME"), "/.rkrtemp");
    if (( fn = fopen (tempfile2, "w")) != NULL) {
        memset(buf,0,sizeof(buf));
        while (fgets (buf, sizeof buf, fs) != NULL) {
            sbuf = buf;
            memset(rbuf,0,sizeof(rbuf));
            sprintf(rbuf,"%s",buf);
            sscanf(buf,"%d",&eff);
            rname = strsep(&sbuf,",");
            rname = strsep(&sbuf,",");
            if((eff==num)&&(strcmp(rname,name)==0)) {
                continue;
            } else fputs(rbuf,fn);
            memset(buf,0,sizeof(buf));

        }
    }
    fclose(fs);
    fclose(fn);

    sprintf(orden,"mv %s %s\n",tempfile2,tempfile);
    system(orden);

}

void
RKR::MergeIntPreset(char *filename)
{

    char orden[1024];
    char tempfile[256];
    char tempfile2[256];

    memset(tempfile,0,sizeof(tempfile));
    memset(tempfile2,0,sizeof(tempfile2));
    memset(orden,0,sizeof(orden));


    sprintf (tempfile, "%s%s", getenv ("HOME"), "/.rkrintpreset");
    sprintf (tempfile2, "%s%s", getenv ("HOME"), "/.rkrtemp");


    sprintf(orden,"cat %s %s > %s\n",tempfile,filename,tempfile2);
    system(orden);

    memset(orden,0,sizeof(orden));

    sprintf(orden,"mv %s %s\n",tempfile2,tempfile);
    system(orden);


}

void
RKR::savemiditable(char *filename)
{

    int i;
    FILE *fn;
    char buf[256];
    fn = fopen (filename, "w");
    if(errno == EACCES) {
        Error_Handle(3);
        fclose(fn);
        return;
    }

    for(i=0; i<128; i++) {
        memset (buf, 0, sizeof (buf));
        sprintf (buf, "%d,%d\n", M_table[i].bank,M_table[i].preset);
        fputs (buf, fn);
    }

    fclose(fn);
}


void
RKR::loadmiditable (char *filename)
{
    int i;
    char buf[256];
    FILE *fn;

    if ((fn = fopen (filename, "r")) == NULL)
        return;

    for(i=0; i<128; i++) {
        memset (buf, 0, sizeof (buf));
        fgets (buf, sizeof buf, fn);
        sscanf (buf, "%d,%d\n", &M_table[i].bank, &M_table[i].preset);
    }
    fclose(fn);

}

