import scripts.core.utils as utils
import colorama



CapabilitiesInfos = {
    "nb": {
        "struct": "audio_capability_chunk_t",
        "prefix": "AUDIO_CAPABILITY_CHUNK_",
        "values": ["none","one","two","three","four","five","multiple"]
    },
    "nbChan": {
        "struct": "audio_capability_channel_t",
        "prefix": "AUDIO_CAPABILITY_",
        "values": ["1ch","2ch","3ch","4ch","5ch","6ch","7ch","8ch","ch_all"]
    },
    "fs": {
        "struct": "audio_capability_fs_t",
        "prefix": "AUDIO_CAPABILITY_FS_",
        "values": ["8000","12000","16000","24000","32000","48000","256000","384000","512000","576000","640000","768000","960000","1024000","1152000","1280000","1536000","1920000","2048000","2304000","2560000","3072000","3840000","4096000","pcm_all", "pcm_all_and_custom","pdm_all","all", "all_and_custom"]
    },
    "interleaving": {
        "struct": "audio_capability_interleaving_t",
        "prefix": "AUDIO_CAPABILITY_INTERLEAVING_",
        "values": ["yes","no","both"]
    },
    "time_freq": {
        "struct": "audio_capability_time_freq_t",
        "prefix": "AUDIO_CAPABILITY_",
        "values": ["time","freq","time_freq"]
    },
    "type": {
        "struct": "audio_capability_type_t",
        "prefix": "AUDIO_CAPABILITY_TYPE_",
        "values": ["all", "pdm_lsb_first","pdm_msb_first","g711_alaw","g711_mulaw","fixed16","fixed32","float","pdm","g711","fixed16_fixed32","fixed16_float","fixed32_float","fixed16_fixed32_float","fixed16_fixed32_float_g711","fixed16_fixed32_float_pdm"]
    },
}

ConsistencyInfos = {
    "struct": "audio_buffer_param_list_t",
    "prefix": "ABUFF_PARAM_",
    "values": ["all","fs","ch","time_freq","interleaving","type","nb_elements","duration","not_fs","not_ch","not_time_freq","not_interleaving","not_type","not_duration","not_nb_elements","not_nb_elements_fs","not_interleaving_ch","not_applicable"]
}

PrioLevelsInfos = {
    "struct": "audio_capability_prio_level_t",
    "prefix": "AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_",
    "values": ["normal","low"]
}

pControlInfos = ["slidershort","default", "checkbox", "static","slider","slidershort", "structarray", "droplist", "percent"]


def get_prefix(param):
    return CapabilitiesInfos[param]["prefix"]

def get_struct(param):
    return CapabilitiesInfos[param]["struct"]

def get_values(param):
    return CapabilitiesInfos[param]["values"]




class Capabilities():
    """This class implements AFE processed as an x86 executable"""
    def __init__(self):
        self.capabilities = {}

    def get_pControl_list(self):
        return pControlInfos

    def build(self, algo_infos):
        self.algo_infos = algo_infos

        def __build_consistency(consistency, type):
            def check(type, param):
                if param not in ConsistencyInfos["values"]:
                    txt = "for {} pins consistency: {} does not belong to possible values\n".format(type, param)
                    txt += "possible values are {}\n".format(str(ConsistencyInfos["values"]))
                    utils.printc(txt, color=colorama.Fore.RED)
                    quit()

            if consistency == None:
                ret = "ABUFF_PARAM_NOT_APPLICABLE"
            elif isinstance(consistency, str) and consistency.lower() == "all":
                ret =  ConsistencyInfos["prefix"]+"ALL"
            else:
                ret = "({})(".format(ConsistencyInfos["struct"])
                for i, param in enumerate(consistency):
                    check(type, param)
                    ret += "(uint32_t){}{}".format(ConsistencyInfos["prefix"],param.upper())
                    if i != len(consistency) - 1:  # check if current index is the last index
                        ret += "|"
                ret += ")"
            return ret

        def __build_param_capability(param, param_values, type, reset=False):
            def check(param, param_value):
                if param_value not in get_values(param):
                    txt = "Error with parameter \"{}\" of attribute \"{}\": \"{}\" does not belong to possible values\n".format(param, type.replace('.',''), param_value)
                    txt += "possible values are {}\n".format(str(get_values(param)))
                    utils.printc(txt, color=colorama.Fore.RED)
                    quit()

            if reset == True:
                if param == "nb":
                    ret = "AUDIO_CAPABILITY_CHUNK_NONE"
                else:
                    ret = "({})AUDIO_ALGO_FORMAT_UNDEF".format(get_struct(param))
            else:
                prefix = get_prefix(param)
                if isinstance(param_values, str):
                    ret = prefix + param_values.upper()
                    check(param, param_values)
                else:
                    ret = "({})(".format(get_struct(param))

                    for i, param_val in enumerate(param_values):
                        check(param, param_val)
                        ret += "(uint32_t){}{}".format(prefix, param_val.upper())
                        if i != len(param_values) - 1:  # check if current index is the last index
                            ret += "|"
                    ret += ")"
            return ret

        def __build(capabilities, pin, type, reset=False):
            def add(capabilities, pin, field_prefix, field):
                capabilities[field_prefix + field] = __build_param_capability(field, pin[field], field_prefix, reset)

            capabilities["chunks_consistency."+type.lower()] = __build_consistency(pin["consistency"], type)
            field_prefix = "ios" + type + "."
            add(capabilities,pin, field_prefix, "nb" )
            add(capabilities,pin, field_prefix, "nbChan" )
            add(capabilities,pin, field_prefix, "fs" )
            add(capabilities,pin, field_prefix, "interleaving" )
            add(capabilities,pin, field_prefix, "time_freq" )
            add(capabilities,pin, field_prefix, "type" )
            if "list" in pin.keys() and pin["list"] is not None:
                pin_name = "AUDIO_ALGO_OPT_STR(\""
                pin_desc = "AUDIO_ALGO_OPT_STR(\""
                for io_name, io_desc in pin["list"].items():
                    pin_name += io_name+"\\0"
                    pin_desc += io_desc+"\\0"
                pin_name += "\")"
                pin_desc += "\")"
                if reset == False:
                    capabilities[field_prefix + "pPinNames"] = pin_name
                    capabilities[field_prefix + "pDescs"] = pin_desc

        input_pins = algo_infos.input_pins
        output_pins = algo_infos.output_pins
        capabilities =  self.capabilities
        capabilities["misc.pAlgoDesc"] = algo_infos.description

        if algo_infos.prio_level.lower() in PrioLevelsInfos["values"]:
            capabilities["prio_level"] = "{}{}".format(PrioLevelsInfos["prefix"],algo_infos.prio_level.upper())
        else:
            txt = "wrong prio_level! possible values are {}\n".format(str(PrioLevelsInfos["values"]))
            utils.printc(txt, color=colorama.Fore.RED)
            quit()
        capabilities["chunks_consistency.in_out"] = __build_consistency(algo_infos.ios_consistency, "ios")
        if input_pins is not None:
            __build(capabilities,input_pins, "In")
        elif output_pins is not None:
            __build(capabilities, output_pins, "In", reset=True)

        if output_pins is not None:
            __build(capabilities,output_pins, "Out")
        elif input_pins is not None:
            __build(capabilities, input_pins, "Out", reset=True)
    def get(self):
        return self.capabilities