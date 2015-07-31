#include <math.h>
#include "beattracker.h"

beattracker:: beattracker (double sample_rate, uint32_t intermediate_bufsize)
{

	interpbuf = new float[intermediate_bufsize];
    rmsfilter = new RBFilter (0, 15.0f, 0.15f, 1, sample_rate, interpbuf);
    peaklpfilter = new RBFilter (0, 25.0f, 0.5f, 0, sample_rate, interpbuf);
    peaklpfilter2 = new RBFilter (0, 25.0f, 0.5f, 0, sample_rate, interpbuf);
    peakhpfilter = new RBFilter (1, 45.0f, 0.5f, 0, sample_rate, interpbuf);

    index = (int *) malloc (sizeof (int) * intermediate_bufsize);

//Trigger Filter Settings
    fSAMPLE_RATE = sample_rate;
    peakpulse = peak = envrms = 0.0f;
    peakdecay = 10.0f/fSAMPLE_RATE;
    targatk = 12.0f/fSAMPLE_RATE;   ///for smoothing filter transition
    atk = 200.0f/fSAMPLE_RATE;
    trigtime = sample_rate/12; //time to take next peak
    onset = 0;
    trigthresh = 0.15f;

    tscntr = 0;
    tsidx = 0;

    oldbpm = 0.0f;
    oldmost = 0.0f;

    cleanup ();

};

beattracker::~beattracker ()
{
	delete rmsfilter;
	delete peaklpfilter;
	delete peaklpfilter2;
	delete peakhpfilter;
	delete[] interpbuf;
	free(index);
};

void
beattracker::cleanup ()
{

    peakpulse = peak = envrms = 0.0f;
    peakdecay = 10.0f/fSAMPLE_RATE;
    targatk = 12.0f/fSAMPLE_RATE;   ///for smoothing filter transition
    atk = 200.0f/fSAMPLE_RATE;
    trigtime = fSAMPLE_RATE/20; //time to take next peak
    onset = 0;
    trigthresh = 0.15f;
    oldbpm = 0.0f;
    oldmost = 0.0f;
    tscntr = 0;
    tsidx = 0;
    for( int i = 0; i<17; i++) {
        avbpm[i] = ( ((float) i) + 0.5f) * 10.0f;  //center the initial average in the bin
        statsbin[i] = 1.0f;  //set all to 1.0 and let fly from there
    }
    statsbin[12] = 1.1f;  //slightly bias to 120bpm
    maxptr = 12;
    oldbpm = 120.0f;
    bpm_change_cntr = 0;
};

void
beattracker::detect (float * smpsl, float * smpsr, uint32_t period)
{
    float tmp;
    int idx = 0;
    unsigned int i = 0;

    for ( i = 0; i < period; i++) { //Detect dynamics onset
        index[i] = 0; //initializes all elements to zero

        tmp = 15.0f*fabs(smpsl[i] + smpsr[i]);
        envrms = rmsfilter->filterout_s(tmp);
        if ( tmp > peak) peak =  atk + tmp;
        if ( envrms < peak) peak -= peakdecay;
        if(peak<0.0f) peak = 0.0f;

        peakpulse = peaklpfilter2->filterout_s(fabs(peakhpfilter->filterout_s(peak)));


        if( peakpulse > trigthresh ) {
            if (trigtimeout==0) {
                index[idx] = i;
                timeseries[tsidx] = tscntr;
                tsidx++;
                tsidx%=20;
                tscntr = 0;
                idx++;

                trigtimeout = trigtime;

                //calculate tempo on every beat detected
                calc_tempo();
            }

        } else {
            if (--trigtimeout<0) {
                trigtimeout = 0;
            }

        }
        tscntr++;
    }


};
/*
float
beattracker::get_tempo()  //returns tempo in float beats per minute
{
if((oldbpm>600.0f) || (oldbpm<0.0f)) oldbpm = 0.0f;

float time = 0;
if(tsidx>0) time = ((float) timeseries[tsidx-1])/fSAMPLE_RATE;
else  time = ((float) timeseries[19])/fSAMPLE_RATE;

//below really needs to be more clever to pull out 1/8, 1/4, 1/2, triplets or other time signatures.
//for now it's good enough to work with relatively steady patterns in 4/4
float divisor = 0;
int intdiv = 0;
if(time<1.0f) {
intdiv = lrintf(1.0f/time);
divisor = (float) intdiv;
time*=divisor;
}
else {
intdiv = lrintf(time);
divisor = 1.0f/((float) intdiv);
time*=divisor;
}

float bpm = 120.0f/time;

//below is some code to look at the outputs, a crude statistical analysis

//oldbpm = 0.95f*oldbpm + 0.05f*bpm;

/
//oldbpm = avg;
//avg = oldmost;

float low = avg*0.95f;
float high = avg*1.05f;
if ( (bpm>low) && (bpm<high) ) {
oldmost = 0.1f*oldmost + 0.9f*bpm;
}
//
//uncomment below to see what is happening
//printf("time: %f bpm: %f avg: %f most: %f time: 1/%f \n", time, bpm, oldbpm, oldmost,divisor);

return(bpm);

};
*/

void
beattracker::calc_tempo()  //returns tempo in float beats per minute
{
    if((oldbpm>600.0f) || (oldbpm<0.0f)) oldbpm = 0.0f;

    float time = 0;
    if(tsidx>0) time = ((float) timeseries[tsidx-1])/fSAMPLE_RATE;
    else  time = ((float) timeseries[19])/fSAMPLE_RATE;

    float bpm = 30.0f/time;

    while(bpm < 80.0f) bpm*=2.0f;
    while(bpm > 160.0f) bpm*=0.5f;  //normalize by powers of 2 to 80<bpm<160

    int binptr = lrintf(floorf(0.1f*bpm));  //get a bin address
    statsbin[binptr] += 1.0f;  //increase count
    avbpm[binptr] = 0.6f*avbpm[binptr] + 0.4f*bpm;
    if(avbpm[binptr] < 0.1f) avbpm[binptr] = 0.01f;  //create a floor to avoid denormal computations

    float maxbin = 0.0f;
    maxptr = binptr;
    for(int i = 8; i<15; i++) {
        statsbin[i] *= 0.9f;  //weight 80 through 160 bins
        if(statsbin[i] > maxbin) {
            maxbin = statsbin[i];
            maxptr = i;
        }

    }

    if(fabs(oldbpm - avbpm[maxptr]) > 10.0f) {//prevent bpm from changing too abruptly and frequently
        if( ++bpm_change_cntr > 4) {
            oldbpm = avbpm[maxptr];  //wait 4 beats before allowing change > 10 bpm
            bpm_change_cntr = 0;
        }
    } else oldbpm = avbpm[maxptr];

};

float
beattracker::get_tempo()
{

    return(oldbpm);
};
