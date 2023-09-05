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
    
      #Look at filename and get time and date and satellite
      name_list = filename.split('-')
      noaa_satnum = name_list[0].split("_")[0]
      str_date = name_list[1]
      hour_window = name_list[2][0:2]

      #Construct bufr filename
      dirname = parent_dir + noaa_satnum  + "-" + str_date + "-" + hour_window 
      #If 6 hour window bufr was not created, create bufr file
      if( not os.path.isdir(dirname)):
        subprocess.call(["mkdir", dirname])
      subprocess.call(["cp", parent_dir+filename, dirname])
   
#for subdir, dirs, files in os.walk("./prepbufr/"):
#make final prepbufr directory if it doesn't exist
intermediate_dirname = root_dir + "intermediate_prepbufr/"
if( not os.path.isdir(intermediate_dirname)):
  subprocess.call(["mkdir", intermediate_dirname])

for subdir, dirs, files in os.walk(parent_dir):
    for dirname in dirs:
        bufr_filename = intermediate_dirname + dirname + ".bufr"
        subprocess.call(("cat " + parent_dir + dirname + "/*"+ " > " + bufr_filename),  shell=True)
