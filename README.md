# Observation Preprocessor

# Steps to build:
1. CHANGE Source g5_modules from GEOSgcm: `source /discover/nobackup/asewnath/GEOSgcm/@env/g5_modules.sh`. Feel free to use your own GEOSgcm build for this.
2.  `mkdir obs_processing`
3.  `git clone https://github.com/asewnath/observation_preprocessor.git`
4.  `cd observation_preprocessor/build`
5.  You can leave `ESMADIR` in `GNUmakefile` as is or you can change the field to point to your build of GEOSgcm.
6.  `make install`
7.  `cd ../`

Now you have the executable `run_preprocess` to start preprocessing files. 

# Executable Usage
`./run_preprocess <input file directory> <source> <output type> <output file directory>`
where
- `<input file directory>`: directory that contains files to be preprocessed
- `<source>`: input file type
- `<output>`: output file type
- `<output file directory>`: directory where output files will be saved

Possible sources: eumetsat, jma, wisconsin, noaa, kma, avhrr
Possible output types: prepbufr, text

# Example
To process feature-tracked winds from CIMISS into prepbufrs, run the following: 
`./run_preprocess /archive/u/asewnath/reanalysis/GOES/raw_data/data_stage_test/ wisconsin prepbufr ../output_prepbufr`. This will take files from the archive and process them into prepbufrs which will be saved in the indicated output directory one level above the git repository.


