
name = "disto"
prio_level = "normal"
description = "Guitar distortion"
ios_consistency = "all"

api_list = None

source_list = None

input_pins = {
    "consistency" : None,
    "nb"          : "one",
    "nbChan"      : "1ch",
    "fs"          : "48000",
    "interleaving": "yes",
    "time_freq"   : "time",
    "type"        : ["float"],
    "list"        : {"pIn" : "input pin" }
}

output_pins = {
    "consistency" : None,
    "nb"          : "one",
    "nbChan"      : "1ch",
    "fs"          : "48000",
    "interleaving": "yes",
    "time_freq"   : "time",
    "type"        : ["float"],
    "list"        : {"pOut" : "output pin"}
}

# Parameters are either:
#  + static: which have impact on memory allocation
#  + dynamic: that don't have impact on memory and can benefit from on the fly tuning (no deinit then init necessary)
# Note: script will warn you in case of wrong settings, for instance if pControl is left empty or wrong, the script will
# provide with the possible values

static_params = {
  "struct_name" : "disto_static_config_t",
  "params" : {
      "drive" : {"pDefault" : 0.0,
                 "pDescription" : "Drive ",
                 "pControl" : "slidershort",
                 "min" : 1.0,
                 "max" : 10.0,
                 "type": "float"},

      # "range": {"pDefault": 0.0,
      #             "pDescription": "multiplication factor for the drive to make distortion stronger ",
      #             "pControl": "slidershort",
      #             "min": 1.0,
      #             "max": 10.0,
      #             "type": "float"},
      #
      "off_axis": {"pDefault": 0.0,
                "pDescription": "Add DC to tune symmetry/asymmetry to enhance even harmonics",
                "pControl": "slidershort",
                "min": 0.0,
                "max": 10.0,
                "type": "float"},

      "dry_wet": {"pDefault": 0.0,
                "pDescription": "Dry wet mix ",
                "pControl": "slidershort",
                "min": 0.0,
                "max": 1.0,
                "type": "float"},

      "post_gain": {"pDefault": 0.0,
                  "pDescription": "Final gain dB",
                  "pControl": "slidershort",
                  "min": -100.0,
                  "max": 60.0,
                  "type": "float"},
  }
}

callbacks = {
  "init"                       : "disto_init",
  "deinit"                     : "disto_deinit",
  "dataInOut"                  : "disto_dataInOut",
  "process"                    : "disto_process",
  "checkConsistency"           : None,
}
