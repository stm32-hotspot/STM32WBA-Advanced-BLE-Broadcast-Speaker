This directory was downloaded from 
https://www.speex.org/downloads/
it is the version 1.2.1 of the SpeexDSP. 

ST modified the following files: 
    fftwrap.c
    filterbank.c
    math_approx.h
    mdf.c
    os_support.h
    preprocess.c
    smallft.c
    smallft.h
    speex_preprocess.h


Modifications :
- move SpeexPreprocessState structure from preprocess.c to speex_preprocess.h (solve a build issue)
- group small mallocs in bigger global malloc (and add macro to check consistency of allocated size)
- avoid access to NULL pointer in case of allocation fail
- use CMSIS for math routines (sqrt, cos, etc...)
