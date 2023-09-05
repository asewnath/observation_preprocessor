//input_output.h

#ifndef INPUT_OUTPUT_H_
#define INPUT_OUTPUT_H_

  enum class FileSource{
    fromNRL,
    fromWisconsin,
    fromGSI,
    fromEUMETSAT,
    fromJMA,
    fromNOAA,
    fromKMA,
    fromAVHRR,
    Combined
  };

  enum class OutputFormat{
    toTextFile,
    toPrepbufr,
    toHdf
  };

#endif //INPUT_OUTPUT_H_
