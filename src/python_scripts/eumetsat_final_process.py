#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Nov  8 09:35:30 2019

@author: asewnath
"""

import os
import subprocess
import sys

root_dir = sys.argv[1] + "/"
parent_dir = sys.argv[1] + "/prepbufr/"

for subdir, dirs, files in os.walk(parent_dir):
    for filename in files:
 
      fname_list = filename.split("-")
      str_date = fname_list[8]
      str_hour = fname_list[9][0:2]     

      dirname = parent_dir +  "METSAT" + str_date + "-" + str_hour  
      if( not os.path.isdir(dirname)):
        subprocess.call(["mkdir", dirname])

      subprocess.call(["cp", parent_dir+filename, dirname])
      
intermediate_dirname = root_dir  + "intermediate_prepbufr/"
if( not os.path.isdir(intermediate_dirname)):
  subprocess.call(["mkdir", intermediate_dirname])

#for subdir, dirs, files in os.walk("./eumetsat_bufr/"):
for subdir, dirs, files in os.walk(parent_dir):
    for dirname in dirs:
        bufr_filename = intermediate_dirname + dirname + ".bufr"
        subprocess.call(("cat " + parent_dir + dirname + "/*"+ " > " + bufr_filename),  shell=True)


