//SPencer Jackson

#include"rkrlv2.h"

///// EQ /////////
LV2_Handle init_eqlv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 12;
    plug->effectindex = IEQ;
    plug->prev_bypass = 1;

    getFeatures(plug,host_features);

    plug->eq = new EQ(0,0,sample_freq, plug->period_max);

    //eq has a bunch of setup stuff. Why isn't this in the EQ initalizer?
    for (int i = 0; i <= 45; i += 5)
    {
        plug->eq->changepar (i + 10, 7);
        plug->eq->changepar (i + 14, 0);
    }

    plug->eq->changepar (11, 31);
    plug->eq->changepar (16, 63);
    plug->eq->changepar (21, 125);
    plug->eq->changepar (26, 250);
    plug->eq->changepar (31, 500);
    plug->eq->changepar (36, 1000);
    plug->eq->changepar (41, 2000);
    plug->eq->changepar (46, 4000);
    plug->eq->changepar (51, 8000);
    plug->eq->changepar (56, 16000);
    return plug;
}

void run_eqlv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        plug->eq->cleanup();
        //copy dry signal
        memcpy(plug->output_l_p,plug->input_l_p,sizeof(float)*nframes);
        memcpy(plug->output_r_p,plug->input_r_p,sizeof(float)*nframes);
        return;
    }

    //check and set changed parameters
    i = 0;
    val = (int)*plug->param_p[0]+64;//gain
    if(plug->eq->getpar(0) != val)
    {
        plug->eq->changepar(0,val);
    }
    val = (int)*plug->param_p[1]+64;//q
    if(plug->eq->getpar(13) != val)
    {
        int j;
        for(j=0; j<10; j++)
        {
            plug->eq->changepar(j*5+13,val);
        }
    }
    for(i=2; i<plug->nparams; i++)
    {
        val = (int)*plug->param_p[i]+64;//various freq. bands
        if(plug->eq->getpar(5*i + 2) != val)
        {
            plug->eq->changepar(5*i+2,val);
        }
    }

    //eq does in inline?
    memcpy(plug->output_l_p,plug->input_l_p,sizeof(float)*nframes);
    memcpy(plug->output_r_p,plug->input_r_p,sizeof(float)*nframes);

    //now set out ports
    plug->eq->efxoutl = plug->output_l_p;
    plug->eq->efxoutr = plug->output_r_p;

    //now run
    plug->eq->out(plug->output_l_p,plug->output_r_p,nframes);

    xfade_check(plug,nframes);

    return;
}

static void cleanup_rkrlv2(LV2_Handle handle)
{
    RKRLV2* plug = (RKRLV2*)handle;
    switch(plug->effectindex)
    {
    case IEQ:
    case IEQP:
        delete plug->eq;//eql, eqp, cabinet
        break;
    }
    free(plug);
}

static const LV2_Descriptor eqlv2_descriptor=
{
    EQLV2_URI,
    init_eqlv2,
    connect_rkrlv2_ports,
    0,//activate
    run_eqlv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    switch (index)
    {
    case IEQ:
        return &eqlv2_descriptor ;
    default:
        return 0;
    }
}
