import os
import subprocess
import sys

"""
The purpose of this script is to take all of the prepbufr files
we created, and concatenate them together based on the hour window
that they occur. These new prepbufrs will be saved in the 
'intermediate_prepbufr' directory for the next step!
"""

root_dir = sys.argv[1]
parent_dir = root_dir+"/prepbufr/"

#Looping through all prepbufr files we generated in the prepbufr dir 
for _, _, files in os.walk(parent_dir):
  for filename in files:
    fname_parts = filename.split('-')

    #Make a new directory with the date and hour window
    #(skip if one already exists)
    dirname = parent_dir + "GK_2A" + '-' + fname_parts[1] + fname_parts[2][0:2]
    if( not os.path.isdir(dirname)):
      subprocess.call(["mkdir", dirname])
        
    #Save file into it's dir with correct date and hour window
    subprocess.call(["cp", parent_dir+filename, dirname])

#make intermediate_bufr if it doesn't exist
intermediate_dir = parent_dir + "../intermediate_prepbufr/"    
if( not os.path.isdir(intermediate_dir)):
    subprocess.call(["mkdir", intermediate_dir])


#Looping through all the date and hour window directories we created in the /prepbufr dir
for _, dirs, _ in os.walk(parent_dir):
  for dirname in dirs:
    bufr_filename = intermediate_dir + dirname + ".bufr"
    #concatenate all the prepbufr files in the dir and save new prepbufr
    #to the intermediate_prepbufr folder
    subprocess.call(("cat " + parent_dir  + dirname + "/*" + " > " + bufr_filename),  shell=True)
