import os
import subprocess
import sys

"""
This routine fills in missing files with empty files for JMA
"""

final_dir = sys.argv[1] + "/final_prepbufr/"
parent_dir = sys.argv[1] + "/intermediate_prepbufr/"


#make intermediate_bufr if it doesn't exist
if( not os.path.isdir(final_dir)):
    subprocess.call(["mkdir", final_dir])

for subdir, dirs, files in os.walk(parent_dir):

  for filename in files:
    dirname = parent_dir + filename[0:len(filename)-7]
    if( not os.path.isdir(dirname)):
      subprocess.call(["mkdir", dirname])
    subprocess.call(["cp", parent_dir+filename, dirname])

 
hr_list = ["00", "06", "12", "18"]

for subdir1, dirs1, files1 in os.walk(parent_dir):
  for dirname1 in dirs1:
    for subdir2, dirs2, files2 in os.walk(parent_dir+dirname1):
      file_hr = []
      for filename2 in files2:
        file_hr.append(filename2[len(filename2)-7: len(filename2)-5])
      diff_set = set(hr_list).difference(file_hr)
      if(len(diff_set) != 0):
        for element in diff_set:
          subprocess.call( ("touch "+parent_dir+dirname1+"/"+dirname1+element+".bufr"), shell=True )
    
for subdir, dirs, files in os.walk(parent_dir):
  for dirname in dirs:
    for subdir1,dirs1,files1 in os.walk(parent_dir+dirname):
      for filename1 in files1:
        if(os.path.isfile(sys.argv[1]+'/'+filename1)):
          print("file already exists, call cat")
          subprocess.call(("cat " + parent_dir+dirname+'/'+filename1  + " >> " + sys.argv[1]+'/'+filename1),  shell=True)
        else:
          subprocess.call(["cp", parent_dir+dirname+'/'+filename1, final_dir+filename1])
