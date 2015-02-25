#ifndef MAYER_H
#define MAYER_H

//You can tweak how fast this routine executes here with a little sacrifice
#define GOOD_TRIG

#ifdef GOOD_TRIG
#else
#define FAST_TRIG
#endif

#if defined(GOOD_TRIG)
#define FHT_SWAP(a,b,t) {(t)=(a);(a)=(b);(b)=(t);}
#define TRIG_VARS                                                \
      int t_lam=0;
#define TRIG_INIT(k,c,s)                                         \
     {                                                           \
      int i;                                                     \
      for (i=2 ; i<=k ; i++)                                     \
          {coswrk[i]=costab[i];sinwrk[i]=sintab[i];}             \
      t_lam = 0;                                                 \
      c = 1;                                                     \
      s = 0;                                                     \
     }
#define TRIG_NEXT(k,c,s)                                         \
     {                                                           \
         int i,j;                                                \
         (t_lam)++;                                              \
         for (i=0 ; !((1<<i)&t_lam) ; i++);                      \
         i = k-i;                                                \
         s = sinwrk[i];                                          \
         c = coswrk[i];                                          \
         if (i>1)                                                \
            {                                                    \
             for (j=k-i+2 ; (1<<j)&t_lam ; j++);                 \
             j         = k - j;                                  \
             sinwrk[i] = halsec[i] * (sinwrk[i-1] + sinwrk[j]);  \
             coswrk[i] = halsec[i] * (coswrk[i-1] + coswrk[j]);  \
            }                                                    \
     }
#define TRIG_RESET(k,c,s)
#endif

#if defined(FAST_TRIG)
#define TRIG_VARS                                        \
      float t_c,t_s;
#define TRIG_INIT(k,c,s)                                 \
    {                                                    \
     t_c  = costab[k];                                   \
     t_s  = sintab[k];                                   \
     c    = 1;                                           \
     s    = 0;                                           \
    }
#define TRIG_NEXT(k,c,s)                                 \
    {                                                    \
     float t = c;                                         \
     c   = t*t_c - s*t_s;                                \
     s   = t*t_s + s*t_c;                                \
    }
#define TRIG_RESET(k,c,s)
#endif

#define M_SQRT2_2   0.70710678118654752440084436210484
#define M_SQRT22   2.0*0.70710678118654752440084436210484

class fft_filter
{
public:
    //fft_filter (float * efxoutl_, float * efxoutr_, float size);
    fft_filter();
    ~fft_filter ();
    void out (float * smpsl, float * smpr);
    void make_window(int n, float* window);
    void load_impulse(int size, char* filename);
    void run_filter(float* smps);
    void resample_impulse(int size, float* ir);
    void cleanup ();
    float outvolume;

    float *efxoutl;
    float *efxoutr;

    //makes linear indexing easier
    void realifft(int n, float *real);
    void realfft(int n, float *real);

//  void interleaved_real_fft(int n, float *real);
//  void interleaved_real_ifft(int n, float *real);

private:
    void mayer_fft(int n, float *real, float *imag);
    void mayer_ifft(int n, float *real, float *imag);
    void mayer_fht(float *fz, int n);
    void mayer_realifft(int n, float *real);
    void mayer_realfft(int n, float *real);

    static float halsec[20];
    static float costab[20];
    static float sintab[20];
    static float coswrk[20];
    static float sinwrk[20];

};

#endif
