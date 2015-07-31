/*
  Rakarrack   Audio FX software
  Stompbox.C - stompbox modeler
  Modified for rakarrack by Ryan Billing

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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "StompBox.h"



StompBox::StompBox (float * efxoutl_, float * efxoutr_, double sample_rate, uint32_t intermediate_bufsize,
		int wave_res, int wave_upq, int wave_dnq)
{
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;


    //default values
    Ppreset = 0;
    Pvolume = 50;

    //left channel filters
    interpbuf = new float[intermediate_bufsize];
    linput = new AnalogFilter (1, 80.0f, 1.0f, 0, sample_rate, interpbuf);  //  AnalogFilter (unsigned char Ftype, float Ffreq, float Fq,unsigned char Fstages);
    lpre1 = new AnalogFilter (1, 630.0f, 1.0f, 0, sample_rate, interpbuf);   // LPF = 0, HPF = 1
    lpre2 = new AnalogFilter (1, 220.0f, 1.0f, 0, sample_rate, interpbuf);
    lpost = new AnalogFilter (0, 720.0f, 1.0f, 0, sample_rate, interpbuf);
    ltonehg = new AnalogFilter (1, 1500.0f, 1.0f, 0, sample_rate, interpbuf);
    ltonemd = new AnalogFilter (4, 1000.0f, 1.0f, 0, sample_rate, interpbuf);
    ltonelw = new AnalogFilter (0, 500.0f, 1.0, 0, sample_rate, interpbuf);

    //Right channel filters
    rinput = new AnalogFilter (1, 80.0f, 1.0f, 0, sample_rate, interpbuf);  //  AnalogFilter (unsigned char Ftype, float Ffreq, float Fq,unsigned char Fstages);
    rpre1 = new AnalogFilter (1, 630.0f, 1.0f, 0, sample_rate, interpbuf);   // , sample_rateLPF = 0, HPF = 1
    rpre2 = new AnalogFilter (1, 220.0f, 1.0f, 0, sample_rate, interpbuf);
    rpost = new AnalogFilter (0, 720.0f, 1.0f, 0, sample_rate, interpbuf);
    rtonehg = new AnalogFilter (1, 1500.0f, 1.0f, 0, sample_rate, interpbuf);
    rtonemd = new AnalogFilter (4, 1000.0f, 1.0f, 0, sample_rate, interpbuf);
    rtonelw = new AnalogFilter (0, 500.0f, 1.0f, 0, sample_rate, interpbuf);

    //Anti-aliasing for between stages
    ranti = new AnalogFilter (0, 6000.0f, 0.707f, 1, sample_rate, interpbuf);
    lanti = new AnalogFilter (0, 6000.0f, 0.707f, 1, sample_rate, interpbuf);

    rwshape = new Waveshaper(sample_rate,wave_res,wave_upq,wave_dnq,intermediate_bufsize);
    lwshape = new Waveshaper(sample_rate,wave_res,wave_upq,wave_dnq,intermediate_bufsize);
    rwshape2 = new Waveshaper(sample_rate,wave_res,wave_upq,wave_dnq,intermediate_bufsize);
    lwshape2 = new Waveshaper(sample_rate,wave_res,wave_upq,wave_dnq,intermediate_bufsize);

    cleanup ();

    setpreset (Ppreset);
};

StompBox::~StompBox ()
{
    delete linput;
    delete lpre1;
    delete lpre2;
    delete lpost;
    delete ltonehg;
    delete ltonemd;
    delete ltonelw;

    delete[] interpbuf;
    //Right channel filters
    delete rinput;
    delete rpre1;
    delete rpre2;
    delete rpost;
    delete rtonehg;
    delete rtonemd;
    delete rtonelw;

    //Anti-aliasing for between stages
    delete ranti;
    delete lanti;

    delete rwshape;
    delete lwshape;
    delete rwshape2;
    delete lwshape2;
};

/*
 * Cleanup the effect
 */
void
StompBox::cleanup ()
{
    linput->cleanup();
    lpre1->cleanup();
    lpre2->cleanup();
    lpost->cleanup();
    ltonehg->cleanup();
    ltonemd->cleanup();
    ltonelw->cleanup();

    //right channel filters
    rinput->cleanup();
    rpre1->cleanup();
    rpre2->cleanup();
    rpost->cleanup();
    rtonehg->cleanup();
    rtonemd->cleanup();
    rtonelw->cleanup();

    ranti->cleanup();
    lanti->cleanup();

    rwshape->cleanup();
    lwshape->cleanup();
    rwshape2->cleanup();
    lwshape2->cleanup();

};


/*
 * Effect output
 */
void
StompBox::out (float * smpsl, float * smpsr, uint32_t period)
{
    unsigned int i;

    float hfilter;  //temporary variables
    float mfilter;
    float lfilter;
    float tempr;
    float templ;

    switch (Pmode) {
    case 0:          //Odie

        lpre2->filterout(smpsl, period);
        rpre2->filterout(smpsr, period);
        rwshape->waveshapesmps (period, smpsl, 28, 20, 1);  //Valve2
        lwshape->waveshapesmps (period, smpsr, 28, 20, 1);
        ranti->filterout(smpsr, period);
        lanti->filterout(smpsl, period);
        lpre1->filterout(smpsl, period);
        rpre1->filterout(smpsr, period);
        rwshape2->waveshapesmps (period, smpsl, 28, Pgain, 1);  //Valve2
        lwshape2->waveshapesmps (period, smpsr, 28, Pgain, 1);

        lpost->filterout(smpsl, period);
        rpost->filterout(smpsr, period);

        for (i = 0; i<period; i++) {
            //left channel
            lfilter =  ltonelw->filterout_s(smpsl[i]);
            mfilter =  ltonemd->filterout_s(smpsl[i]);
            hfilter =  ltonehg->filterout_s(smpsl[i]);

            efxoutl[i] = 0.5f * volume * (smpsl[i] + lowb*lfilter + midb*mfilter + highb*hfilter);

            //Right channel
            lfilter =  rtonelw->filterout_s(smpsr[i]);
            mfilter =  rtonemd->filterout_s(smpsr[i]);
            hfilter =  rtonehg->filterout_s(smpsr[i]);

            efxoutr[i] = 0.5f * volume * (smpsr[i] + lowb*lfilter + midb*mfilter + highb*hfilter);

        }

        break;

    case 1:  //Grunge
    case 5:  //Death Metal
    case 6:  //Metal Zone
        linput->filterout(smpsl, period);
        rinput->filterout(smpsr, period);

        for (i = 0; i<period; i++) {
            templ = smpsl[i] * (gain * pgain + 0.01f);
            tempr = smpsr[i] * (gain * pgain + 0.01f);
            smpsl[i] += lpre1->filterout_s(templ);
            smpsr[i] += rpre1->filterout_s(tempr);
        }
        rwshape->waveshapesmps (period, smpsl, 24, 1, 1);  // Op amp limiting
        lwshape->waveshapesmps (period, smpsr, 24, 1, 1);

        ranti->filterout(smpsr, period);
        lanti->filterout(smpsl, period);

        rwshape2->waveshapesmps (period, smpsl, 23, Pgain, 1);  // hard comp
        lwshape2->waveshapesmps (period, smpsr, 23, Pgain, 1);


        for (i = 0; i<period; i++) {
            smpsl[i] = smpsl[i] + RGP2 * lpre2->filterout_s(smpsl[i]);
            smpsr[i] = smpsr[i] + RGP2 * rpre2->filterout_s(smpsr[i]);
            smpsl[i] = smpsl[i] + RGPST * lpost->filterout_s(smpsl[i]);
            smpsr[i] = smpsr[i] + RGPST * rpost->filterout_s(smpsr[i]);

            //left channel
            lfilter =  ltonelw->filterout_s(smpsl[i]);
            mfilter =  ltonemd->filterout_s(smpsl[i]);
            hfilter =  ltonehg->filterout_s(smpsl[i]);

            efxoutl[i] = 0.1f * volume * (smpsl[i] + lowb*lfilter + midb*mfilter + highb*hfilter);

            //Right channel
            lfilter =  rtonelw->filterout_s(smpsr[i]);
            mfilter =  rtonemd->filterout_s(smpsr[i]);
            hfilter =  rtonehg->filterout_s(smpsr[i]);

            efxoutr[i] = 0.1f * volume * (smpsr[i] + lowb*lfilter + midb*mfilter + highb*hfilter);

        }



        break;
    case 2:  //Rat
    case 3:  //Fat Cat  //Pre gain & filter freqs the only difference

        linput->filterout(smpsl, period);
        rinput->filterout(smpsr, period);

        for (i = 0; i<period; i++) {
            templ = smpsl[i];
            tempr = smpsr[i];
            smpsl[i] += lpre1->filterout_s(pre1gain*gain*templ);
            smpsr[i] += rpre1->filterout_s(pre1gain*gain*tempr);  //Low freq gain stage
            smpsl[i] += lpre2->filterout_s(pre2gain*gain*templ);
            smpsr[i] += rpre2->filterout_s(pre2gain*gain*tempr); //High freq gain stage

        }


        rwshape->waveshapesmps (period, smpsl, 24, 1, 1);  // Op amp limiting
        lwshape->waveshapesmps (period, smpsr, 24, 1, 1);

        ranti->filterout(smpsr, period);
        lanti->filterout(smpsl, period);

        rwshape2->waveshapesmps (period, smpsl, 23, 1, 0);  // hard comp
        lwshape2->waveshapesmps (period, smpsr, 23, 1, 0);


        for (i = 0; i<period; i++) {
            //left channel
            lfilter =  ltonelw->filterout_s(smpsl[i]);
            mfilter =  ltonemd->filterout_s(smpsl[i]);

            efxoutl[i] = 0.5f * ltonehg->filterout_s(volume * (smpsl[i] + lowb*lfilter + midb*mfilter));

            //Right channel
            lfilter =  rtonelw->filterout_s(smpsr[i]);
            mfilter =  rtonemd->filterout_s(smpsr[i]);

            efxoutr[i] = 0.5f * rtonehg->filterout_s(volume * (smpsr[i] + lowb*lfilter + midb*mfilter));

        }

        break;
    case 4:  //Dist+

        linput->filterout(smpsl, period);
        rinput->filterout(smpsr, period);

        for (i = 0; i<period; i++) {
            templ = smpsl[i];
            tempr = smpsr[i];
            smpsl[i] += lpre1->filterout_s(pre1gain*gain*templ);
            smpsr[i] += rpre1->filterout_s(pre1gain*gain*tempr);  //Low freq gain stage
        }


        rwshape->waveshapesmps (period, smpsl, 24, 1, 1);  // Op amp limiting
        lwshape->waveshapesmps (period, smpsr, 24, 1, 1);

        ranti->filterout(smpsr, period);
        lanti->filterout(smpsl, period);

        rwshape2->waveshapesmps (period, smpsl, 29, 1, 0);  // diode limit
        lwshape2->waveshapesmps (period, smpsr, 29, 1, 0);


        for (i = 0; i<period; i++) {
            //left channel
            lfilter =  ltonelw->filterout_s(smpsl[i]);
            mfilter =  ltonemd->filterout_s(smpsl[i]);

            efxoutl[i] = 0.5f * ltonehg->filterout_s(volume * (smpsl[i] + lowb*lfilter + midb*mfilter));

            //Right channel
            lfilter =  rtonelw->filterout_s(smpsr[i]);
            mfilter =  rtonemd->filterout_s(smpsr[i]);

            efxoutr[i] = 0.5f * rtonehg->filterout_s(volume * (smpsr[i] + lowb*lfilter + midb*mfilter));

        }

        break;

    case 7:          //Classic Fuzz

        lpre1->filterout(smpsl, period);
        rpre1->filterout(smpsr, period);
        linput->filterout(smpsl, period);
        rinput->filterout(smpsr, period);
        rwshape->waveshapesmps (period, smpsr, 19, 25, 1);  //compress
        lwshape->waveshapesmps (period, smpsl, 19, 25, 1);

        for (i = 0; i<period; i++) {

            //left channel
            mfilter =  ltonemd->filterout_s(smpsl[i]);

            templ = lpost->filterout_s(fabs(smpsl[i]));
            tempr = rpost->filterout_s(fabs(smpsr[i]));   //dynamic symmetry

            smpsl[i] += lowb*templ + midb*mfilter;      //In this case, lowb control tweaks symmetry

            //Right channel
            mfilter =  rtonemd->filterout_s(smpsr[i]);
            smpsr[i] += lowb*tempr + midb*mfilter;

        }

        ranti->filterout(smpsr, period);
        lanti->filterout(smpsl, period);
        rwshape2->waveshapesmps (period, smpsr, 25, Pgain, 1);  //JFET
        lwshape2->waveshapesmps (period, smpsl, 25, Pgain, 1);
        lpre2->filterout(smpsl, period);
        rpre2->filterout(smpsr, period);

        for (i = 0; i<period; i++) {
            //left channel
            lfilter =  ltonelw->filterout_s(smpsl[i]);
            hfilter =  ltonehg->filterout_s(smpsl[i]);

            efxoutl[i] = volume * ((1.0f - highb)*lfilter + highb*hfilter);  //classic BMP tone stack

            //Right channel
            lfilter =  rtonelw->filterout_s(smpsr[i]);
            hfilter =  rtonehg->filterout_s(smpsr[i]);

            efxoutr[i] = volume * ((1.0f - highb)*lfilter + highb*hfilter);

        }
        break;
    }



};


/*
 * Parameter control
 */
void StompBox::init_mode (int value)
{
    int tinput = 1;
    float finput = 80.0f;
    float qinput = 1.0f;
    int sinput = 0;

    int tpre1 = 1;
    float fpre1 = 708.0f;
    float qpre1 = 1.0f;
    int spre1 = 0;

    int tpre2 = 1;
    float fpre2 = 30.0f;
    float qpre2 = 1.0f;
    int spre2 = 0;

    int tpost = 0;
    float fpost = 720.0f;
    float qpost = 1.0f;
    int spost = 0;

    int ttonehg = 1;
    float ftonehg = 1500.0f;
    float qtonehg = 1.0f;
    int stonehg = 0;

    int ttonemd = 4;
    float ftonemd = 720.0f;
    float qtonemd = 1.0f;
    int stonemd = 0;

    int ttonelw = 0;
    float ftonelw = 500.0f;
    float qtonelw = 1.0f;
    int stonelw = 0;

    switch (value) {
    case 0:
        tinput = 1;
        finput = 80.0f;
        qinput = 1.0f;
        sinput = 0;

        tpre1 = 1;
        fpre1 = 630.0f;
        qpre1 = 1.0f;
        spre1 = 0;

        tpre2 = 1;
        fpre2 = 220.0f;
        qpre2 = 1.0f;
        spre2 = 0;

        tpost = 0;
        fpost = 720.0f;
        qpost = 1.0f;
        spost = 0;

        ttonehg = 1;
        ftonehg = 1500.0f;
        qtonehg = 1.0f;
        stonehg = 0;

        ttonemd = 4;
        ftonemd = 720.0f;
        qtonemd = 1.0f;
        stonemd = 0;

        ttonelw = 0;
        ftonelw = 500.0f;
        qtonelw = 1.0f;
        stonelw = 0;
        break;

    case 1: //Grunge
// Some key filter stages based upon a schematic for a grunge pedal
// Total gain up to 25,740/2 (91dB)
// Fc1 =  999.02  Gain = 110 = 40.8dB
// Q1 =  2.9502
// gain stage 1rst order HP @ 340 Hz, Gain = 21.3 ... 234 (26dB ... 47dB)
// Fc2 =  324.50
// Q2 =  4.5039
// Fc3 =  5994.1
// Q3 =  1.7701
// Fc4 =  127.80
// Q4 =  3.7739

        tinput = 4;         //Pre-Emphasis filter
        finput = 1000.0f;
        qinput = 2.95f;
        sinput = 0;
        pgain = 110.0f;

        tpre1 = 0;         //Gain stage reduce aliasing
        fpre1 = 6000.0f;
        qpre1 = 0.707f;
        spre1 = 2;

        tpre2 = 4;        //being used as a recovery filter, gain = 10
        fpre2 = 324.5f;
        qpre2 = 4.5f;
        spre2 = 0;
        RGP2 = 10.0f;

        tpost = 4;       //The other recovery filter, gain = 3
        fpost = 6000.0f;
        qpost = 1.77f;
        spost = 0;
        RGPST = 3.0f;

        ttonehg = 1;       //high shelf ranging 880 to 9700 Hz, gain 10
        ftonehg = 4000.0f;
        qtonehg = 1.0f;
        stonehg = 0;

        ttonemd = 4;       // Pedal has no mid filter so I'll make up my own
        ftonemd = 1000.0f;
        qtonemd = 2.0f;
        stonemd = 0;

        ttonelw = 4;       //Low Eq band, peaking type, gain = up to 22.
        ftonelw = 128.0f;
        qtonelw = 3.8f;
        stonelw = 0;
        break;

    case 2: //ProCo Rat Distortion emulation
// Some key filter stages based upon a schematic for a grunge pedal

        tinput = 0;         //Reduce some noise aliasing
        finput = 5000.0f;
        qinput = 1.0f;
        sinput = 3;

        tpre1 = 1;         //Gain stage high boost, gain = 1 ... 268 (max)
        fpre1 = 60.0f;
        qpre1 = 1.0f;
        spre1 = 0;
        pre1gain = 268.0f;

        tpre2 = 1;        //being used as a recovery filter, gain = 1 ... 3000
        fpre2 = 1539.0f;
        qpre2 = 1.0f;
        spre2 = 0;
        pre2gain = 3000.0f;

        tpost = 0;       //Not used...initialized to "something"
        fpost = 6000.0f;
        qpost = 1.77f;
        spost = 0;

        ttonehg = 0;       //frequency sweeping LPF
        ftonehg = 1000.0f;
        qtonehg = 1.0f;
        stonehg = 0;

        ttonemd = 4;       // Pedal has no mid filter so I'll make up my own
        ftonemd = 700.0f;
        qtonemd = 2.0f;
        stonemd = 0;

        ttonelw = 0;       //Pedal has no Low filter, so make up my own...Low Eq band, peaking type
        ftonelw = 328.0f;  //Mild low boost
        qtonelw = 0.50f;
        stonelw = 1;
        break;

    case 3: //Fat Cat Distortion emulation
    case 4: //MXR Dist+ emulation (many below filters unuse)
// Some key filter stages based upon a schematic for a grunge pedal

        tinput = 0;         //Reduce some noise aliasing
        finput = 5000.0f;
        qinput = 1.0f;
        sinput = 3;

        tpre1 = 1;         //Gain stage high boost, gain = 1 ... 100 (max)
        fpre1 = 33.0f;
        qpre1 = 1.0f;
        spre1 = 0;
        pre1gain = 100.0f;

        tpre2 = 1;        //being used as a recovery filter, gain = 1 ... 1700
        fpre2 = 861.0f;
        qpre2 = 1.0f;
        spre2 = 0;
        pre2gain = 1700.0f;

        tpost = 0;       //Not used...initialized to "something"
        fpost = 6000.0f;
        qpost = 1.77f;
        spost = 0;

        ttonehg = 0;       //frequency sweeping LPF
        ftonehg = 1000.0f;
        qtonehg = 1.0f;
        stonehg = 0;

        ttonemd = 4;       // Pedal has no mid filter so I'll make up my own
        ftonemd = 700.0f;
        qtonemd = 2.0f;
        stonemd = 0;

        ttonelw = 0;       //Pedal has no Low filter, so make up my own...Low Eq band, peaking type
        ftonelw = 328.0f;  //Mild low boost
        qtonelw = 0.50f;
        stonelw = 1;
        break;

    case 5: //Death Metal
// Some key filter stages based upon a schematic for a grunge pedal

        tinput = 4;         //Pre-Emphasis filter
        finput = 6735.4f;
        qinput = 0.43f;
        sinput = 0;
        pgain = 110.0f;

        tpre1 = 0;         //Gain stage reduce aliasing
        fpre1 = 6000.0f;
        qpre1 = 0.707f;
        spre1 = 2;

        tpre2 = 4;        //being used as a recovery filter, gain = 10
        fpre2 = 517.0f;
        qpre2 = 7.17f;
        spre2 = 0;
        RGP2 = 1.0f;

        tpost = 4;       //The other recovery filter, gain = 10
        fpost = 48.0f;
        qpost = 6.68f;
        spost = 0;
        RGPST = 10.0f;

        ttonehg = 1;       //high shelf ranging 880 to 9700 Hz, gain 11
        ftonehg = 4000.0f;
        qtonehg = 1.0f;
        stonehg = 0;
        HG = 11.0f;

        ttonemd = 4;       // Mid band EQ gain 11
        ftonemd = 1017.0f;
        qtonemd = 1.15f;
        stonemd = 0;
        MG = 11.0f;

        ttonelw = 4;       //Low Eq band, peaking type, gain = up to 22.
        ftonelw = 107.0f;
        qtonelw = 3.16f;
        stonelw = 0;
        LG = 22.0f;

        break;
    case 6: //Metal Zone
// Some key filter stages based upon a schematic for a grunge pedal

        tinput = 4;         //Pre-Emphasis filter
        finput = 952.53f;
        qinput = 2.8f;
        sinput = 0;
        pgain = 100.0f;

        tpre1 = 0;         //Gain stage reduce aliasing
        fpre1 = 6000.0f;
        qpre1 = 0.707f;
        spre1 = 2;

        tpre2 = 4;        //being used as a recovery filter, gain = 10
        fpre2 = 4894.0f;
        qpre2 = 2.16f;
        spre2 = 0;
        RGP2 = 3.3f;

        tpost = 4;       //The other recovery filter, gain = 10
        fpost = 105.0f;
        qpost = 14.62f;
        spost = 0;
        RGPST = 7.0f;

        ttonehg = 1;       //high shelf ranging 880 to 9700 Hz, gain 11
        ftonehg = 4000.0f;
        qtonehg = 1.0f;
        stonehg = 0;
        HG = 10.0f;

        ttonemd = 4;       // Mid band EQ gain 11
        ftonemd = 1017.0f;
        qtonemd = 1.15f;
        stonemd = 0;
        MG = 11.0f;

        ttonelw = 4;       //Low Eq band, peaking type, gain = up to 22.
        ftonelw = 105.50f;
        qtonelw = 3.11f;
        stonelw = 0;
        LG = 3.33f;

        break;

    case 7:  //Classic Fuzz
        tinput = 1;
        finput = 80.0f;
        qinput = 1.0f;
        sinput = 0;

        tpre1 = 0;
        fpre1 = 4500.0f;
        qpre1 = 1.0f;
        spre1 = 1;

        tpre2 = 1;
        fpre2 = 40.0f;
        qpre2 = 1.0f;
        spre2 = 0;

        tpost = 0;
        fpost = 2.0f;
        qpost = 1.0f;
        spost = 0;

        ttonehg = 1;
        ftonehg = 397.0f;
        qtonehg = 1.0f;
        stonehg = 0;

        ttonemd = 4;
        ftonemd = 515.0f;  //sort of like a stuck wahwah
        qtonemd = 4.0f;
        stonemd = 0;

        ttonelw = 0;
        ftonelw = 295.0f;
        qtonelw = 1.0f;
        stonelw = 0;
        break;
    }

    //left channel filters
    //  AnalogFilter (unsigned char Ftype, float Ffreq, float Fq,unsigned char Fstages);
    // LPF = 0, HPF = 1
    linput->settype(tinput);
    linput->setfreq_and_q(finput, qinput);
    linput->setstages(sinput);

    lpre1->settype(tpre1);
    lpre1->setfreq_and_q(fpre1, qpre1);
    lpre1->setstages(spre1);

    lpre2->settype(tpre2);
    lpre2->setfreq_and_q(fpre2, qpre2);
    lpre2->setstages(spre2);

    lpost->settype(tpost);
    lpost->setfreq_and_q(fpost, qpost);
    lpost->setstages(spost);

    ltonehg->settype(ttonehg);
    ltonehg->setfreq_and_q(ftonehg,qtonehg);
    ltonehg->setstages(stonehg);

    ltonemd->settype(ttonemd);
    ltonemd->setfreq_and_q(ftonemd,qtonemd);
    ltonemd->setstages(stonemd);

    ltonelw->settype(ttonelw);
    ltonelw->setfreq_and_q(ftonelw, qtonelw);
    ltonelw->setstages(stonelw);

    //right channel filters

    rinput->settype(tinput);
    rinput->setfreq_and_q(finput, qinput);
    rinput->setstages(sinput);

    rpre1->settype(tpre1);
    rpre1->setfreq_and_q(fpre1, qpre1);
    rpre1->setstages(spre1);

    rpre2->settype(tpre2);
    rpre2->setfreq_and_q(fpre2, qpre2);
    rpre2->setstages(spre2);

    rpost->settype(tpost);
    rpost->setfreq_and_q(fpost, qpost);
    rpost->setstages(spost);

    rtonehg->settype(ttonehg);
    rtonehg->setfreq_and_q(ftonehg,qtonehg);
    rtonehg->setstages(stonehg);

    rtonemd->settype(ttonemd);
    rtonemd->setfreq_and_q(ftonemd,qtonemd);
    rtonemd->setstages(stonemd);

    rtonelw->settype(ttonelw);
    rtonelw->setfreq_and_q(ftonelw, qtonelw);
    rtonelw->setstages(stonelw);

};

void StompBox::init_tone ()
{
    float varf;
    switch (Pmode) {
    case 0:
        varf = 2533.0f + highb*1733.0f;  //High tone ranges from 800 to 6000Hz
        rtonehg->setfreq(varf);
        ltonehg->setfreq(varf);
        if (highb > 0.0f) highb = ((float) Phigh)/8.0f;
        break;

    case 1:
        varf = 3333.0f + highb*2500.0f;  //High tone ranges from 833 to 8333Hz
        rtonehg->setfreq(varf);
        ltonehg->setfreq(varf);

        if (highb > 0.0f) highb = ((float) Phigh)/16.0f;
        if (lowb > 0.0f) lowb = ((float) Plow)/18.0f;
        break;

    case 2:
    case 3:
        varf = 3653.0f + highb*3173.0f;  //High tone ranges from ~480 to 10k
        rtonehg->setfreq(varf);
        ltonehg->setfreq(varf);
        break;
    case 4:
        varf = gain*700.0f + 20.0f;
        rpre1->setfreq(varf);
        lpre1->setfreq(varf);
        pre1gain = 212.0f;
        varf = 3653.0f + highb*3173.0f;  //High tone ranges from ~480 to 10k
        rtonehg->setfreq(varf);
        ltonehg->setfreq(varf);
        break;
    case 5: //Death Metal
    case 6: //Mid Elves Own
        varf = 3653.0f + highb*3173.0f;  //High tone ranges from ~480 to 10k
        rtonehg->setfreq(varf);
        ltonehg->setfreq(varf);

        if (highb > 0.0f) highb = HG * ((float) Phigh)/64.0f;
        if (lowb > 0.0f) lowb = LG * ((float) Plow)/64.0f;
        if (midb > 0.0f) midb = MG * ((float) Pmid)/64.0f;
        break;

    case 7:
        highb = ((float) Phigh + 64)/127.0f;
        varf = 40.0f + gain * 200.0f;
        linput->setfreq(varf);
        rinput->setfreq(varf);
        if (midb > 0.0f) midb = ((float) Pmid)/8.0f;
        lowb = ((float) Plow)/64.0f;

        varf = 1085.0f - lowb * 1000.0f;
        lpre1->setfreq(varf);
        rpre1->setfreq(varf);
        break;


    }

};


void
StompBox::setvolume (int value)
{
    Pvolume = value;
    volume = (float)Pvolume / 127.0f;
};

void
StompBox::setpreset (int npreset)
{
    const int PRESET_SIZE = 6;
    const int NUM_PRESETS = 8;
    int pdata[PRESET_SIZE];
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Odie
        {80, 32, 0, 32, 10, 0},
        //Grunger
        {48, 10, -6, 55, 85, 1},
        //Hard Dist.
        {48, -22, -6, 38, 12, 1},
        //Ratty
        {48, -20, 0, 0, 70, 2},
        //Classic Dist
        {50, 64, 0, 0, 110, 4},
        //Morbid Impalement
        {38, 6, 6, 6, 105, 5},
        //Mid Elve
        {48, 0, -12, 0, 127, 6},
        //Fuzz
        {48, 0, 0, 0, 127, 7}
    };

    if(npreset>NUM_PRESETS-1) {
        Fpre->ReadPreset(39,npreset-NUM_PRESETS+1,pdata);
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, pdata[n]);
    } else {
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, presets[npreset][n]);
    }
    Ppreset = npreset;
    cleanup ();

};


void
StompBox::changepar (int npar, int value)
{
    switch (npar) {
    case 0:
        setvolume (value);
        break;
    case 1:
        Phigh = value;
        if( value < 0) highb = ((float) value)/64.0f;
        if( value > 0) highb = ((float) value)/32.0f;
        break;
    case 2:
        Pmid = value;
        if( value < 0) midb = ((float) value)/64.0f;
        if( value > 0) midb = ((float) value)/32.0f;
        break;
    case 3:
        Plow = value;
        if( value < 0) lowb = ((float) value)/64.0f;
        if( value > 0) lowb = ((float) value)/32.0f;
        break;
    case 4:
        Pgain = value;
        gain = dB2rap(50.0f * ((float)value)/127.0f  - 50.0f);
        break;
    case 5:
        Pmode = value;
        init_mode (Pmode);
        break;

    };
    init_tone ();
};

int
StompBox::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
        return (Phigh);
        break;
    case 2:
        return (Pmid);
        break;
    case 3:
        return (Plow);
        break;
    case 4:
        return (Pgain);
        break;
    case 5:
        return (Pmode);
        break;

    };
    return (0);			//in case of bogus parameter number
};

