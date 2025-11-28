# This template id card is commented to provide a howto way of integrating an algorithm inside Audio-Kit
# the example is around our delay source code.
# one can run the script from the folder 'Middlewares/ST/Audio-Kit/src/algos' with following line:
#   python AcIntegrate.py --outdir myDelay --id_card template/id_card.py --root_dir delay/
# and check the result inside the generated folder = myDelay


# General Information
#  - name
#  - prio_level:
#    + can be "normal" vs "low"
#    + allows to group two types of processing with different priority levels. It is useful with an ASR for instance.
#      In such case, the ASR processing can be tuned to "low" so that audio signal processing is guaranteed all along the
#      data flow.
#  - description to provide detailed blabla about the algorithm
#  - ios_consistency allows to describe if some input pin characteristics should be the same ones as the ones of the output
#    pin. For instance, setting ios_consistency = "nbChan" will force input and output pins to have the same number of channels.

name = "delay"
prio_level = "normal"
description = "Add a Delay"
ios_consistency = "all"

# Api list is optional.
# + if None, the script generates the API structure from the static_params & dynamic_params fields. 
#   It will be located inside the audio_chain_xxx.h where xxx is the algo name.
# + if defined, the listed file should include parameters that match description inside the static_params & dynamic_params fields.
#   These files will be included in generated c files
api_list = ["delay_config.h" ]

# source list is optional. 
# if the algorithm has some header that is not API but should be çincluded in generated c file, it should be listed here. 
# if option --no_file_copy is not used, the files are copied in the output directory
source_list = ["src/delay.c", "src/delay.h"]

# Script will send warning in case of wrong settings providing with the possible values
# Setting an algorithm can be done using this capability. For instance, let's say an algorithm can support mono and/or stereo
# then the "nbChan" field should be:
#     "nbChan"      : ["1ch","2ch"]
# or
#     "nbChan"      : "1ch_2ch"
# In order to get the possible value, just fill in with a wrong data such as
#     "nbChan"      : "xxx"
# the script will output the possible ones.

# Pins description:
#  - "consistency":
#     + is not mandatory but help while debugging.
#     + It allows to check if some parameters of the IO should cope with some restrictions.
#     + For instance, as of today, frequency domain processing only works with floating point and non interleaved data. In
#       such case the setting should be  ==> "consistency" : ['interleaving', 'type']
#  - "list"
#     + is mandatory
#     + provide name & description of all IOs
#  - all other parameters are mandatory

input_pins = {
    "consistency" : None,
    "nb"          : "one",
    "nbChan"      : "ch_all",
    "fs"          : "pcm_all",
    "interleaving": "both",
    "time_freq"   : "time",
    "type"        : ["fixed16","fixed32","float"],
    "list"        : {"pIn" : "input pin" }
}

output_pins = {
    "consistency" : None,
    "nb"          : "one",
    "nbChan"      : "ch_all",
    "fs"          : "pcm_all",
    "interleaving": "both",
    "time_freq"   : "time",
    "type"        : ["fixed16","fixed32","float"],
    "list"        : {"pOut" : "output pin"}
}

# Parameters are either:
#  + static: which have impact on memory allocation
#  + dynamic: that don't have impact on memory and can benefit from on the fly tuning (no deinit then init necessary)
# Note: script will warn you in case of wrong settings, for instance if pControl is left empty or wrong, the script will
# provide with the possible values

static_params = {
  "struct_name" : "delay_static_config_t",
  "params" : {
    "delay" : {"pDefault" : 0.0,
               "pDescription" : "Delay in second ",
               "pControl" : "slidershort",
               "min" : 0.0,
               "max" : 1.0,
               "type": "float"},
  }
}

# dynamic_params = None

# List of callbacks for the algorithm. If NULL, nothing done:
# Algorithm processing can be split into two parts:
#  + init & deinit:
#    - manadatory
#    - for allocation/free and init/deinit.
#
#  + configure:
#    - optional
#    - for dynamic reconfiguration (only dynamic parameters that do not require an allocation).
#
#  + dataInOut:
#    - retrieves input data from the input IOs
#    - provides output data the output IOs.
#    - mandatory routine but can be empty of any audio task. However, in order to trigger the processing callback,
#      AudioAlgo_incReadyForProcess(pAlgo); MUST BE called. In case where no processing callback is needed, calling
#      AudioAlgo_incReadyForProcess is not necessary.
#    - Doesn't need to be called at the same pace as processing callback. For instance, let's assume dataInOut @ 1ms but
#      processing must be called @ 8ms; then developer will have to ensure that dataInOut calls
#      AudioAlgo_incReadyForProcess every 8 calls.
#    - In a low-latency driven system it can host the complete processing, otherwise,
#      it is better to delegate the main part of the processing to the process callback
#    - Automatically triggered when input IOs had at least a frame written
#    - the dataInOut callback of all algorithms in a chain are called within a single task/thread
#
#  + process:
#    - optional routines; everything can be done in dataInOut as explained.
#    - different priority than dataInOut, MUST BE lower
#    - two levels available: "normal" vs "low"
#    - the process callback of all algorithms in a chain are called within a single task/thread which priority can be
#      tuned lower than the thread of the dataInOut
#
#  + checkConsistency:
#    - optional
#    - checks that inputs & outputs are inline with algorithm's specific tuning. Most of consistency checks are generic
#      however some check depending on parameter values can be implemented there. For instance, a resampler which API
#      would offer and conversion ratio should make sure that the in & out IOs frequencies match this ratio.
#
#
callbacks = {
  "init"                       : "delay_init",
  "deinit"                     : "delay_deinit",
  # "configure"                  : None,
  "dataInOut"                  : "delay_dataInOut",
  "process"                    : "delay_process",
  "checkConsistency"           : None,
}
