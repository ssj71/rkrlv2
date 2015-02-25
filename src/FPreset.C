/*
  Rakarrack   Audio FX software
  FPreset.C - Internal Preset Reader
  by Josep Andreu

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#include "FPreset.h"



FPreset::FPreset()
{
};

FPreset::~FPreset()
{
};

void
FPreset::ReadPreset(int eff, int num, int pdata[])
{

    FILE *fn;
    char tempfile[256];
    char buf[256];
    char *sbuf;
    int k=0;
    int reff=0;
    memset(tempfile,0,sizeof(tempfile));
    memset(pdata,0,sizeof(int)*50);
    sprintf (tempfile, "%s%s", getenv ("HOME"), "/.rkrintpreset");
    if (( fn = fopen (tempfile, "r")) != NULL) {
        while (fgets (buf, sizeof buf, fn) != NULL)

        {
            sbuf = buf;
            sscanf(buf,"%d",&reff);
            if(reff==eff) k++;
            if(k==num) {
                strsep(&sbuf,",");
                strsep(&sbuf,",");
                sscanf(sbuf,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d.%d.%d\n",
                       &pdata[0],&pdata[1],&pdata[2],&pdata[3],&pdata[4],&pdata[5],&pdata[6],&pdata[7],&pdata[8],&pdata[9],
                       &pdata[10],&pdata[11],&pdata[12],&pdata[13],&pdata[14],&pdata[15],&pdata[16],&pdata[17],&pdata[18],&pdata[19],
                       &pdata[20],&pdata[21],&pdata[22],&pdata[23],&pdata[24],&pdata[25],&pdata[26],&pdata[27],&pdata[28],&pdata[29]);
                break;
            }
        }


        fclose(fn);
    }



};


