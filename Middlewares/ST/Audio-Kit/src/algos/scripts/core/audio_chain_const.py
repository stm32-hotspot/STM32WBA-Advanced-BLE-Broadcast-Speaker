class AudioChainStr:
    """ This class contains all const strings needed to generate release notes """

    # C header  --------------------------------------------------
    header = r"""
/*
* This software component is licensed by ST under Ultimate Liberty license
* SLA0044, the "License"; You may not use this file except in compliance with
* the License. You may obtain a copy of the License at:
*                             www.st.com/SLA0044
*
*/


#include "audio_chain_tasks.h"
#include "audio_chain_instance.h"
#include "audio_chain_instance_system_conf.h"
"""

    use_display = """
#ifdef DISPLAY_CPU_LOAD
  #define DISPLAY_CPU_LOAD_CB         cycleMeasure_displayCpuLoadOnUart
  #define DISPLAY_CPU_LOAD_TIMEOUT_MS 1000UL
#else
  #define DISPLAY_CPU_LOAD_CB         NULL
  #define DISPLAY_CPU_LOAD_TIMEOUT_MS 0UL
#endif
    
"""

    c_error_call = """{
     AudioChainInstance_error();
  }"""