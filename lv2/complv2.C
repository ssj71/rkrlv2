//Spencer Jackson

#include"rkrlv2.h"

///// comp /////////
LV2_Handle init_complv2(const LV2_Descriptor *descriptor,double sample_freq, const char *bundle_path,const LV2_Feature * const* host_features)
{
    RKRLV2* plug = (RKRLV2*)malloc(sizeof(RKRLV2));

    plug->nparams = 9;
    plug->effectindex = ICOMP;
    plug->prev_bypass = 1;

    plug->comp = new Compressor(0,0, sample_freq);

    return plug;
}

void run_complv2(LV2_Handle handle, uint32_t nframes)
{
    int i;
    int val;

    RKRLV2* plug = (RKRLV2*)handle;

    if(*plug->bypass_p && plug->prev_bypass)
    {
        plug->comp->cleanup();
        //copy dry signal
        memcpy(plug->output_l_p,plug->input_l_p,sizeof(float)*nframes);
        memcpy(plug->output_r_p,plug->input_r_p,sizeof(float)*nframes);
        return;
    }

    //check and set changed parameters
    for(i=0; i<plug->nparams; i++)
    {
        val = (int)*plug->param_p[i];
        if(plug->comp->getpar(i+1) != val)//this effect is 1 indexed
        {
            plug->comp->Compressor_Change(i+1,val);
        }
    }

    //comp does in inline
    memcpy(plug->output_l_p,plug->input_l_p,sizeof(float)*nframes);
    memcpy(plug->output_r_p,plug->input_r_p,sizeof(float)*nframes);

    //now set out ports
    plug->comp->efxoutl = plug->output_l_p;
    plug->comp->efxoutr = plug->output_r_p;

    //now run
    plug->comp->out(plug->output_l_p,plug->output_r_p,nframes);

    xfade_check(plug,nframes);
    return;
}

static void cleanup_rkrlv2(LV2_Handle handle)
{
    RKRLV2* plug = (RKRLV2*)handle;
    switch(plug->effectindex)
    {
    case 0:
        delete plug->comp;
        break;
    }
    free(plug);
}

static const LV2_Descriptor complv2_descriptor=
{
    COMPLV2_URI,
    init_complv2,
    connect_rkrlv2_ports,
    0,//activate
    run_complv2,
    0,//deactivate
    cleanup_rkrlv2,
    0//extension
}; 

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    switch (index)
    {
    case 0:
    case ICOMP:
        return &complv2_descriptor ;
    default:
        return 0;
    }
}
