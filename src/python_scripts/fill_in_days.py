import os
import sys
import subprocess

parent_dir = sys.argv[1]

hours = ["00", "06", "12", "18"]

year  = parent_dir.split("/")[8][1:] 
month = parent_dir.split("/")[9][1:]
days  = []

thirty_day_month = ["09", "04", "06", "11"]
if month in thirty_day_month:
   actual_days = [str(i).zfill(2) for i in range(1, 31)]
else:
   actual_days = [str(i).zfill(2) for i in range(1, 32)]

#retrieve list of days currently in directory
for subdir, dirs, files in os.walk(parent_dir):
  for filename in files:
      #split file name to retrieve day
      file_parts = filename.split("-")
      days.append(file_parts[1][6:8])
      

for actual_day in actual_days:
    if(actual_day not in days):
        for hour in hours:
            new_filename = "METSAT"+ year + month  + actual_day + "-" + hour + ".bufr"
            subprocess.check_call(['touch', parent_dir + new_filename])
