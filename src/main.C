/*
  rakarrack - a guitar efects software

  main.C  -  Main file of the organ
  Copyright (C) 2008-2010 Josep Andreu
  Author: Josep Andreu & Douglas McClendon

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2) for more details.

  You should have received a copy of the GNU General Public License
(version2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/


#include <getopt.h>
#include <sched.h>
#include <sys/mman.h>
#include "global.h"
#include "rakarrack.h"
#include "jack.h"

void
show_help ()
{
    fprintf (stderr, "Usage: rakarrack [OPTION]\n\n");
    fprintf (stderr,
             "  -h ,     --help \t\t\t display command-line help and exit\n");
    fprintf (stderr, "  -n ,     --no-gui \t\t\t disable GUI\n");
    fprintf (stderr, "  -l File, --load=File \t\t\t loads preset\n");
    fprintf (stderr, "  -b File, --bank=File \t\t\t loads bank\n");
    fprintf (stderr, "  -p #,    --preset=# \t\t\t set preset\n");
    fprintf (stderr, "  -x, --dump-preset-names \t\t prints bank of preset names and IDs\n\n");
    fprintf (stderr, "FLTK options are:\n\n");
    fprintf (stderr, "  -bg2 color\n");
    fprintf (stderr, "  -bg color\n");
    fprintf (stderr, "  -di[splay] host:n.n\n");
    fprintf (stderr, "  -dn[d]\n");
    fprintf (stderr, "  -fg color\n");
    fprintf (stderr, "  -g[eometry] WxH+X+Y\n");
    fprintf (stderr, "  -i[conic]\n");
    fprintf (stderr, "  -k[bd]\n");
    fprintf (stderr, "  -na[me] classname\n");
    fprintf (stderr, "  -nod[nd]\n");
    fprintf (stderr, "  -nok[bd]\n");
    fprintf (stderr, "  -not[ooltips]\n");
    fprintf (stderr, "  -s[cheme] scheme (plastic,none,gtk+)\n");
    fprintf (stderr, "  -ti[tle] windowtitle\n");
    fprintf (stderr, "  -to[oltips]\n");
    fprintf (stderr, "\n");

}

int
main (int argc, char *argv[])
{


// Read command Line

    fprintf (stderr,
             "\n%s %s - Copyright (c) Josep Andreu - Ryan Billing - Douglas McClendon - Arnout Engelen\n",
             PACKAGE, VERSION);

    if (argc == 1)
        fprintf (stderr, "Try 'rakarrack --help' for command-line options.\n");


    struct option opts[] = {
        {"load", 1, NULL, 'l'},
        {"bank", 1, NULL, 'b'},
        {"preset",1,NULL, 'p'},
        {"no-gui", 0, NULL, 'n'},
        {"dump-preset-names", 0, NULL, 'x'},
        {"help", 0, NULL, 'h'},
        {"session",1,NULL,'u'},
        {0, 0, 0, 0}
    };


    int needtoloadfile=0;
    int needtodump=0;

    needtoloadbank=0;
    needtoloadstate=0;
    Pexitprogram = 0;
    preset=1000;
    commandline = 0;
    gui = 1;
    opterr = 0;
    int option_index = 0, opt;
    exitwithhelp = 0;

    option_index=0;

    while (1) {
        opt = getopt_long (argc, argv, "l:b:p:u:nxh", opts, &option_index);
        char *optarguments = optarg;

        if (opt == -1)
            break;

        switch (opt) {
        case 'h':
            exitwithhelp = 1;
            break;
        case 'n':
            gui = 0;
            break;
        case 'l':
            if (optarguments != NULL) {
                commandline = 1;
                needtoloadfile=1;
                filetoload=strdup(optarguments);
                break;
            }
        case 'b':
            if (optarguments != NULL) {
                needtoloadbank=1;
                banktoload=strdup(optarguments);
                break;
            }
        case 'p':
            if(optarguments != NULL) {
                preset=atoi(optarguments);
                break;
            }
        case 'x':
            needtodump=1;
            break;

        case 'u':
            if(optarguments != NULL) {
                commandline = 1;
                needtoloadstate=1;
                s_uuid=strdup(optarguments);
                statefile=strdup(argv[optind]);
                break;

            }


        }

    }


    if (exitwithhelp != 0) {
        show_help ();
        return (0);
    };


    RKR rkr;

    if (nojack) {
        show_help ();
        if (gui)
            rkr.Message (1,"rakarrack error",
                         "Cannot make a jack client, is jackd running?");
        return (0);
    }



    if(needtodump) {
        rkr.dump_preset_names ();
        exit(1);
    }


    JACKstart (&rkr, rkr.jackclient);
    rkr.InitMIDI ();
    rkr.ConnectMIDI ();


    if (needtoloadstate) rkr.loadfile(statefile);
    if (needtoloadfile) rkr.loadfile(filetoload);
    if (needtoloadbank) rkr.loadbank(banktoload);


    // Launch GUI

    RKRGUI *rgui=NULL;

    if (gui) rgui = new RKRGUI (argc, argv, &rkr);


    if (gui == 0) {
        rkr.Bypass = 1;
        rkr.calculavol (1);
        rkr.calculavol (2);
        rkr.booster = 1.0f;

    }

    mlockall (MCL_CURRENT | MCL_FUTURE);

    //Main Loop

    while (Pexitprogram == 0) {
        // Refresh GUI
        if (gui) {
            Fl::wait ();
        } else {
            usleep (1500);
            if (preset != 1000) {
                if( (preset>0) && (preset<61)) rkr.Bank_to_Preset (preset);
                preset = 1000;
            }


        }



        if((!rkr.jdis) && (rkr.jshut))

        {

            rkr.jdis=1;
            rkr.Message (1,rkr.jackcliname,
                         "Jack Shut Down, try to save your work");
        }


        rkr.miramidi ();

    }

// free memory etc.

    if(needtoloadstate) rgui->save_stat(0);
    JACKfinish ();
    return (0);


};

