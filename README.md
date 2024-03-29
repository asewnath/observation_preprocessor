# GMAO Observation Preprocessor

The goal of the observation preprocessor is to provide a framework for reading in various data types and outputting injested information to common formats. 

# Steps to build:
1. Source g5_modules from GEOSgcm: `source /discover/nobackup/asewnath/GEOSgcm/@env/g5_modules.sh`. Feel free to use your own GEOSgcm build for this.
2.  `git clone https://github.com/asewnath/observation_preprocessor.git`
3.  `cd observation_preprocessor/build`
4.  You can leave `ESMADIR` in `GNUmakefile` as is or you can change the field to point to your build of GEOSgcm.
5.  `make install`
6.  `cd ../`

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


