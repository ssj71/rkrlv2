#!/usr/bin/env python3

#spencer jackson
# this generates a template for a function that will map the rkr bank data to carla preset data

import os

print("#!/usr/bin/env python3")
print()
print("def remap(efxindex, paramindex):")
print("    return{")

#get ttl files (excluding presets)
files = [f for f in os.listdir('../lv2/ttl/') if ( f.find("presets")==-1 and f.find("swp")==-1 \
    and f.find("manifest")==-1 )]

fxindex = -1
#open file
for fn in files:
    
    f = open("../lv2/ttl/"+fn,"r") 
    pindex = 0
    fxindex += 1

    #plugin header data
    data = f.read()
    #uri
    p = data.find("<http://rakarrack") # first one is GUI
    p = data.find("<http://rakarrack",p)
    n = data.find("\n",p);
    uri = data[p+1:n-1]# disclude <>
    #name
    p = data.find("doap:name ",p)
    p = data.find("\"",p)
    p += 1
    n = data.find("\"",p)
    name = data[p:n]
    print("        ",(fxindex,-1),":",(name, uri),",")

    #parameters
    p = data.find("Bypass",p) #skip bypass
    p = data.find("lv2:symbol ",p)
    while p > 0:
        p = data.find("\"",p) #skip ws
        p += 1;
        n = data.find("\"",p) #find end
        psymbol = data[p:n]
        p = data.find("lv2:name ",p)
        p = data.find("\"",p) #skip ws
        p += 1;
        n = data.find("\"",p) #find end
        pname = data[p:n]
        pindex += 1
        p = data.find("lv2:symbol ",p)
        #prints as (carla index, parameter name, parameter URI, offset)
        print("        ", (fxindex,pindex-1),":",(pindex, pname, psymbol, 0),",")
    print("        ", (fxindex,pindex),":",(-1, ),",") # mark end of effect
print("    }[(efxindex,paramindex)]")
