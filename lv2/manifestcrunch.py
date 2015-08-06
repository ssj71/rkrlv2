#!/usr/bin/env python3

#spencer jackson
# this generates a template for a function that will map the rkr bank data to carla preset data

import os

fxindex = -1
#open master manifest file
f = open("ttl/manifest.ttl","r") 
pindex = 0
fxindex += 1
n = 0 
data = f.read()
if not os.path.exists("ttl//manifests"):
    os.makedirs("ttl/manifests")

# find first plugin
s = data.find("<http://rakarrack",n) 
while s > 0:
    p = data.find("seeAlso <",s)
    n = data.find(".ttl> .",p);
    p += 9
    name = data[p:n]
    print(name, end=", ")

    #next plugin
    s = data.find("<http://rakarrack",n) 

    if not os.path.exists("ttl/manifests/"+name):
        os.makedirs("ttl/manifests/"+name)

    of = open("ttl/manifests/"+name+"/manifest.ttl","w")

    of.write("@prefix lv2:  <http://lv2plug.in/ns/lv2core#> .\n")
    of.write("@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .\n")
    of.write("\n")
    of.write(data[s:n+3])

    of.close()
print("")
f.close()
