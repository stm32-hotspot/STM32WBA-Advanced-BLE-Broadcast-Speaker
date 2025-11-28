#ifdef SIMULATION_X86
  #include "nlms/src/c_conf_files/Nlms_mu0p01000000.h"
#else // SIMULATION_X86
  #define NLMS_NUMTAPS  256U
  #define NLMS_MU  0.1f
#endif // SIMULATION_X86
