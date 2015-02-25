/* This is the FFT routine taken from PureData, a great piece of
software by Miller S. Puckette.
http://crca.ucsd.edu/~msp/software.html */

/*
** FFT and FHT routines
**  Copyright 1988, 1993; Ron Mayer
**
**  mayer_fht(fz,n);
**      Does a hartley transform of "n" points in the array "fz".
**  mayer_fft(n,real,imag)
**      Does a fourier transform of "n" points of the "real" and
**      "imag" arrays.
**  mayer_ifft(n,real,imag)
**      Does an inverse fourier transform of "n" points of the "real"
**      and "imag" arrays.
**  mayer_realfft(n,real)
**      Does a real-valued fourier transform of "n" points of the
**      "real" array.  The real part of the transform ends
**      up in the first half of the array and the imaginary part of the
**      transform ends up in the second half of the array.
**  mayer_realifft(n,real)
**      The inverse of the realfft() routine above.
**
**
** NOTE: This routine uses at least 2 patented algorithms, and may be
**       under the restrictions of a bunch of different organizations.
**       Although I wrote it completely myself, it is kind of a derivative
**       of a routine I once authored and released under the GPL, so it
**       may fall under the free software foundation's restrictions;
**       it was worked on as a Stanford Univ project, so they claim
**       some rights to it; it was further optimized at work here, so
**       I think this company claims parts of it.  The patents are
**       held by R. Bracewell (the FHT algorithm) and O. Buneman (the
**       trig generator), both at Stanford Univ.
**       If it were up to me, I'd say go do whatever you want with it;
**       but it would be polite to give credit to the following people
**       if you use this anywhere:
**           Euler     - probable inventor of the fourier transform.
**           Gauss     - probable inventor of the FFT.
**           Hartley   - probable inventor of the hartley transform.
**           Buneman   - for a really cool trig generator
**           Mayer(me) - for authoring this particular version and
**                       including all the optimizations in one package.
**       Thanks,
**       Ron Mayer; mayer@acuson.com
**
*/

/* This is a slightly modified version of Mayer's contribution; write
* msp@ucsd.edu for the original code.  Kudos to Mayer for a fine piece
* of work.  -msp
*/

/* Use below only as needed /
#include <stdio.h>
#include <stdlib.h>
*/

#include <math.h>
#include "mayer_fft.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


float fft_filter::halsec[20]= {
    0,\
    0,\
    .54119610014619698439972320536638942006107206337801,\
    .50979557910415916894193980398784391368261849190893,\
    .50241928618815570551167011928012092247859337193963,\
    .50060299823519630134550410676638239611758632599591,\
    .50015063602065098821477101271097658495974913010340,\
    .50003765191554772296778139077905492847503165398345,\
    .50000941253588775676512870469186533538523133757983,\
    .50000235310628608051401267171204408939326297376426,\
    .50000058827484117879868526730916804925780637276181,\
    .50000014706860214875463798283871198206179118093251,\
    .50000003676714377807315864400643020315103490883972,\
    .50000000919178552207366560348853455333939112569380,\
    .50000000229794635411562887767906868558991922348920,\
    .50000000057448658687873302235147272458812263401372\
};
float fft_filter::costab[20]= {
    .00000000000000000000000000000000000000000000000000,\
    .70710678118654752440084436210484903928483593768847,\
    .92387953251128675612818318939678828682241662586364,\
    .98078528040323044912618223613423903697393373089333,\
    .99518472667219688624483695310947992157547486872985,\
    .99879545620517239271477160475910069444320361470461,\
    .99969881869620422011576564966617219685006108125772,\
    .99992470183914454092164649119638322435060646880221,\
    .99998117528260114265699043772856771617391725094433,\
    .99999529380957617151158012570011989955298763362218,\
    .99999882345170190992902571017152601904826792288976,\
    .99999970586288221916022821773876567711626389934930,\
    .99999992646571785114473148070738785694820115568892,\
    .99999998161642929380834691540290971450507605124278,\
    .99999999540410731289097193313960614895889430318945,\
    .99999999885102682756267330779455410840053741619428\
};
float fft_filter::sintab[20]= {
    1.0000000000000000000000000000000000000000000000000,\
    .70710678118654752440084436210484903928483593768846,\
    .38268343236508977172845998403039886676134456248561,\
    .19509032201612826784828486847702224092769161775195,\
    .09801714032956060199419556388864184586113667316749,\
    .04906767432741801425495497694268265831474536302574,\
    .02454122852291228803173452945928292506546611923944,\
    .01227153828571992607940826195100321214037231959176,\
    .00613588464915447535964023459037258091705788631738,\
    .00306795676296597627014536549091984251894461021344,\
    .00153398018628476561230369715026407907995486457522,\
    .00076699031874270452693856835794857664314091945205,\
    .00038349518757139558907246168118138126339502603495,\
    .00019174759731070330743990956198900093346887403385,\
    .00009587379909597734587051721097647635118706561284,\
    .00004793689960306688454900399049465887274686668768\
};
float fft_filter::coswrk[20]= {
    .00000000000000000000000000000000000000000000000000,\
    .70710678118654752440084436210484903928483593768847,\
    .92387953251128675612818318939678828682241662586364,\
    .98078528040323044912618223613423903697393373089333,\
    .99518472667219688624483695310947992157547486872985,\
    .99879545620517239271477160475910069444320361470461,\
    .99969881869620422011576564966617219685006108125772,\
    .99992470183914454092164649119638322435060646880221,\
    .99998117528260114265699043772856771617391725094433,\
    .99999529380957617151158012570011989955298763362218,\
    .99999882345170190992902571017152601904826792288976,\
    .99999970586288221916022821773876567711626389934930,\
    .99999992646571785114473148070738785694820115568892,\
    .99999998161642929380834691540290971450507605124278,\
    .99999999540410731289097193313960614895889430318945,\
    .99999999885102682756267330779455410840053741619428\
};
float fft_filter::sinwrk[20]= {
    1.0000000000000000000000000000000000000000000000000,\
    .70710678118654752440084436210484903928483593768846,\
    .38268343236508977172845998403039886676134456248561,\
    .19509032201612826784828486847702224092769161775195,\
    .09801714032956060199419556388864184586113667316749,\
    .04906767432741801425495497694268265831474536302574,\
    .02454122852291228803173452945928292506546611923944,\
    .01227153828571992607940826195100321214037231959176,\
    .00613588464915447535964023459037258091705788631738,\
    .00306795676296597627014536549091984251894461021344,\
    .00153398018628476561230369715026407907995486457522,\
    .00076699031874270452693856835794857664314091945205,\
    .00038349518757139558907246168118138126339502603495,\
    .00019174759731070330743990956198900093346887403385,\
    .00009587379909597734587051721097647635118706561284,\
    .00004793689960306688454900399049465887274686668768\
};
/*
fft_filter::fft_filter (float * efxoutl_, float * efxoutr_, float size)
{
  efxoutl = efxoutl_;
  efxoutr = efxoutr_;

}
*/
fft_filter::fft_filter ()
{

}

fft_filter::~fft_filter ()
{

}

void
fft_filter::cleanup ()
{

}

void
fft_filter::out (float * smpsl, float * smpr)
{

}


void fft_filter::mayer_fht(float *fz, int n)
{

    int  k,k1,k2,k3,k4,kx;
    float *fi,*fn,*gi;
    TRIG_VARS;

    for (k1=1,k2=0; k1<n; k1++) {
        float aa;
        for (k=n>>1; (!((k2^=k)&k)); k>>=1);
        if (k1>k2) {
            aa=fz[k1];
            fz[k1]=fz[k2];
            fz[k2]=aa;
        }
    }
    for ( k=0 ; (1<<k)<n ; k++ );
    k  &= 1;
    if (k==0) {
        for (fi=fz,fn=fz+n; fi<fn; fi+=4) {
            float f0,f1,f2,f3;
            f1     = fi[0 ]-fi[1 ];
            f0     = fi[0 ]+fi[1 ];
            f3     = fi[2 ]-fi[3 ];
            f2     = fi[2 ]+fi[3 ];
            fi[2 ] = (f0-f2);
            fi[0 ] = (f0+f2);
            fi[3 ] = (f1-f3);
            fi[1 ] = (f1+f3);
        }
    } else {
        for (fi=fz,fn=fz+n,gi=fi+1; fi<fn; fi+=8,gi+=8) {
            float bs1,bc1,bs2,bc2,bs3,bc3,bs4,bc4,
                  bg0,bf0,bf1,bg1,bf2,bg2,bf3,bg3;
            bc1     = fi[0 ] - gi[0 ];
            bs1     = fi[0 ] + gi[0 ];
            bc2     = fi[2 ] - gi[2 ];
            bs2     = fi[2 ] + gi[2 ];
            bc3     = fi[4 ] - gi[4 ];
            bs3     = fi[4 ] + gi[4 ];
            bc4     = fi[6 ] - gi[6 ];
            bs4     = fi[6 ] + gi[6 ];
            bf1     = (bs1 - bs2);
            bf0     = (bs1 + bs2);
            bg1     = (bc1 - bc2);
            bg0     = (bc1 + bc2);
            bf3     = (bs3 - bs4);
            bf2     = (bs3 + bs4);
            bg3     = M_SQRT22*bc4;
            bg2     = M_SQRT22*bc3;
            fi[4 ] = bf0 - bf2;
            fi[0 ] = bf0 + bf2;
            fi[6 ] = bf1 - bf3;
            fi[2 ] = bf1 + bf3;
            gi[4 ] = bg0 - bg2;
            gi[0 ] = bg0 + bg2;
            gi[6 ] = bg1 - bg3;
            gi[2 ] = bg1 + bg3;
        }
    }
    if (n<16) return;

    do {
        float s1,c1;
        int ii;
        k  += 2;
        k1  = 1  << k;
        k2  = k1 << 1;
        k4  = k2 << 1;
        k3  = k2 + k1;
        kx  = k1 >> 1;
        fi  = fz;
        gi  = fi + kx;
        fn  = fz + n;
        do {
            float g0,f0,f1,g1,f2,g2,f3,g3;
            f1      = fi[0 ] - fi[k1];
            f0      = fi[0 ] + fi[k1];
            f3      = fi[k2] - fi[k3];
            f2      = fi[k2] + fi[k3];
            fi[k2]  = f0         - f2;
            fi[0 ]  = f0         + f2;
            fi[k3]  = f1         - f3;
            fi[k1]  = f1         + f3;
            g1      = gi[0 ] - gi[k1];
            g0      = gi[0 ] + gi[k1];
            g3      = M_SQRT22  * gi[k3];
            g2      = M_SQRT22  * gi[k2];
            gi[k2]  = g0         - g2;
            gi[0 ]  = g0         + g2;
            gi[k3]  = g1         - g3;
            gi[k1]  = g1         + g3;
            gi     += k4;
            fi     += k4;
        } while (fi<fn);
        TRIG_INIT(k,c1,s1);
        for (ii=1; ii<kx; ii++) {
            float c2,s2;
            TRIG_NEXT(k,c1,s1);
            c2 = c1*c1 - s1*s1;
            s2 = 2*(c1*s1);
            fn = fz + n;
            fi = fz +ii;
            gi = fz +k1-ii;
            do {
                float a,b,g0,f0,f1,g1,f2,g2,f3,g3;
                b       = s2*fi[k1] - c2*gi[k1];
                a       = c2*fi[k1] + s2*gi[k1];
                f1      = fi[0 ]    - a;
                f0      = fi[0 ]    + a;
                g1      = gi[0 ]    - b;
                g0      = gi[0 ]    + b;
                b       = s2*fi[k3] - c2*gi[k3];
                a       = c2*fi[k3] + s2*gi[k3];
                f3      = fi[k2]    - a;
                f2      = fi[k2]    + a;
                g3      = gi[k2]    - b;
                g2      = gi[k2]    + b;
                b       = s1*f2     - c1*g3;
                a       = c1*f2     + s1*g3;
                fi[k2]  = f0        - a;
                fi[0 ]  = f0        + a;
                gi[k3]  = g1        - b;
                gi[k1]  = g1        + b;
                b       = c1*g2     - s1*f3;
                a       = s1*g2     + c1*f3;
                gi[k2]  = g0        - a;
                gi[0 ]  = g0        + a;
                fi[k3]  = f1        - b;
                fi[k1]  = f1        + b;
                gi     += k4;
                fi     += k4;
            } while (fi<fn);
        }
        TRIG_RESET(k,c1,s1);
    } while (k4<n);
}

void fft_filter::mayer_fft(int n, float *real, float *imag)
{
    float a,b,c,d;
    float q,r,s,t;
    int i,j,k;
    for (i=1,j=n-1,k=n/2; i<k; i++,j--) {
        a = real[i];
        b = real[j];
        q=a+b;
        r=a-b;
        c = imag[i];
        d = imag[j];
        s=c+d;
        t=c-d;
        real[i] = (q+t)*.5;
        real[j] = (q-t)*.5;
        imag[i] = (s-r)*.5;
        imag[j] = (s+r)*.5;
    }
    mayer_fht(real,n);
    mayer_fht(imag,n);
}

void fft_filter::mayer_ifft(int n, float *real, float *imag)
{
    float a,b,c,d;
    float q,r,s,t;
    int i,j,k;
    mayer_fht(real,n);
    mayer_fht(imag,n);
    for (i=1,j=n-1,k=n/2; i<k; i++,j--) {
        a = real[i];
        b = real[j];
        q=a+b;
        r=a-b;
        c = imag[i];
        d = imag[j];
        s=c+d;
        t=c-d;
        imag[i] = (s+r)*0.5;
        imag[j] = (s-r)*0.5;
        real[i] = (q-t)*0.5;
        real[j] = (q+t)*0.5;
    }
}

void fft_filter::mayer_realfft(int n, float *real)
{
    float a,b;
    int i,j,k;

    mayer_fht(real,n);
    for (i=1,j=n-1,k=n/2; i<k; i++,j--) {
        a = real[i];
        b = real[j];
        real[j] = (a-b)*0.5;
        real[i] = (a+b)*0.5;
    }
}

void fft_filter::mayer_realifft(int n, float *real)
{
    float a,b;
    int i,j,k;

    for (i=1,j=n-1,k=n/2; i<k; i++,j--) {
        a = real[i];
        b = real[j];
        real[j] = (a-b);
        real[i] = (a+b);
    }
    mayer_fht(real,n);
}

void
fft_filter::realifft(int n, float *real)
{
//this way, realpart[0] = real[0]
//and imagpart[0] = real[n/2] for easier indexing
//This function will reverse the order set with reallfft()
    int i,j,k;
    float tmp = 0.0f;

    real[n/2] = 0.0f;
//unwrap this thing into something easier to index
    for (i=1+(n/2),j=n-1,k=(n - n/4); i<k; i++,j--) {
        tmp = real[j];
        real[j] = real[i];
        real[i] = tmp;
    }

    mayer_realifft(n, real);
}


void
fft_filter::realfft(int n, float *real)
{
//this way, realpart[0] = real[0]
//and imagpart[0] = real[n/2] for easier indexing
    int i,j,k;
    float tmp = 0.0f;
    mayer_realfft(n, real);
    real[n/2] = 0.0f;
//unwrap this thing into something easier to index
    for (i=1+(n/2),j=n-1,k=(n - n/4); i<k; i++,j--) {
        tmp = real[j];
        real[j] = real[i];
        real[i] = tmp;

    }

}

void fft_filter::make_window(int n, float *window)
{
    int i;
    float Ts = 1.0f/((float) n);
    float x = 2.0f*M_PI*Ts;
    float tt = 0.0f;
    for(i=0; i<n; i++) {
        window[i] = 0.5f - 0.5f*cosf(tt);
        tt+=x;

    }

}

void fft_filter::resample_impulse(int size, float* ir)
{

    float* fftBuf;
    float ratio = sqrtf(256.0f)/((float) size);
    int interval = size/128;
    int i,j;
    int fftLength = 512;
    while(fftLength<size) fftLength*=2;  //make sure it's a power of 2

    fftBuf = (float*) malloc(fftLength*sizeof(float));
    memset(fftBuf, 0.0f, fftLength*sizeof(float));
    memcpy(fftBuf, ir, size*sizeof(float));

    realfft(fftLength, fftBuf);
    for(i=0,j=fftLength/2; i<fftLength/2; i++, j++) {
        fftBuf[i]*=ratio;
        fftBuf[j]*=ratio;
        fftBuf[i] = fftBuf[i]*fftBuf[i] + fftBuf[j]*fftBuf[j];
        fftBuf[j] = 0.0f;
    }

//now downsample this thing
    float tmp = 0.0f;
    float sign = -1.0f;
    int cnta, cntb, cntc;
    cnta=cntb=0;
    cntc=128;
    for(i=0; i<fftLength/2; i++) {
        tmp+=fftBuf[i];
        fftBuf[i] = 0.0f;
        if(++cntb>interval) {
            if(cnta<128) {
                fftBuf[cnta] =  tmp*sign;
                fftBuf[cntc] =  -tmp*sign;//tmp;
                sign*=-1.0f;
            }
            tmp = 0.0f;
            cntb = 0;
            cnta++;
            cntc++;
        }

//fftBuf[0]  *= 1.14f;
        fftBuf[128]  = 0;
//fftBuf[255] = 0;

    }
    memset(ir, 0.0f, size*sizeof(float));
    realifft(256, fftBuf);
    memcpy(ir, fftBuf, 256*sizeof(float));

    for(i=0; i<100; i++) {
        ir[i]=0.0f;
    }
//Now convert it to something that looks more like a causal IR
//by adding the forward asymmetry
    for(i=100; i<128; i++) {
        ir[i]*=(0.5f-0.5f*cos(M_PI*(float) (i-100)/27.0f));
        ir[i]*=(0.5f-0.5f*cos(M_PI*(float) (i-100)/27.0f)); //square it
    }
    for(i=128; i<256; i++) {
        ir[i]*=(0.5f-0.5f*cos(2.0f*M_PI*(float) i/256.0f));
    }
    for(i=0,j=100; j<256; j++,i++) {
        ir[i] = ir[j];
        ir[j] = 0.0f;
    }

//Just some debugging code to verify the freq response
    /*realfft(256, ir);

    for(i=0; i<256; i++)
    {
    ir[i] = fabs(ir[i]);
    }*/

}

void fft_filter::load_impulse(int size, char* filename)
{


}

void fft_filter::run_filter(float* smps)
{

}


