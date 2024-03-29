import os
import subprocess
import sys

"""
This routine fills in missing files with empty files for EUMETSAT
"""

final_dir = sys.argv[1] + "/final_prepbufr/"
parent_dir = sys.argv[1] + "/intermediate_prepbufr/"

#make intermediate_bufr if it doesn't exist
if( not os.path.isdir(final_dir)):
    subprocess.call(["mkdir", final_dir])

for subdir, dirs, files in os.walk(parent_dir):

  for filename in files:
    name_list = filename.split('-')  
    dirname = parent_dir + name_list[0]
    if( not os.path.isdir(dirname)):
      subprocess.call(["mkdir", dirname])
    subprocess.call(["cp", parent_dir+filename, dirname])


hr_list = ["00", "06", "12", "18"]

for subdir1, dirs1, files1 in os.walk(parent_dir):
  for dirname1 in dirs1:
    for subdir2, dirs2, files2 in os.walk(parent_dir+dirname1):
      file_hr = []
      for filename2 in files2:
        name_list = filename2.split('-')
        hour_window = name_list[1][0:2]  
        file_hr.append(hour_window)
      diff_set = set(hr_list).difference(file_hr)
      if(len(diff_set) != 0):
        for element in diff_set:
          subprocess.call( ("touch "+parent_dir+dirname1+"/"+dirname1+'-'+element+".bufr"), shell=True )

for subdir, dirs, files in os.walk(parent_dir):
  for dirname in dirs:
    for subdir1,dirs1,files1 in os.walk(parent_dir+dirname):
      for filename1 in files1:
        if(os.path.isfile(sys.argv[1]+'/'+filename1)):
          print("file already exists, call cat")
          subprocess.call(("cat " + parent_dir+dirname+'/'+filename1  + " >> " + sys.argv[1]+'/'+filename1),  shell=True)
        else:
          subprocess.call(["cp", parent_dir+dirname+'/'+filename1, sys.argv[1]+'/'+filename1])

int_prepbufr_abs_path = os.path.abspath(sys.argv[1] + "/intermediate_prepbufr/")
prepbufr_abs_path = os.path.abspath(sys.argv[1] + "/prepbufr/")
subprocess.call(("rm -rf " + int_prepbufr_abs_path), shell=True)
subprocess.call(("rm -rf " + prepbufr_abs_path), shell=True)
