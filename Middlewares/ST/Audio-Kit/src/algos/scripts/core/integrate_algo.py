import os
import colorama
import subprocess
import scripts.core.utils as utils
import numpy as np
from scripts.core.args import ArgsIntegrate as Args
from scripts.core.integrate_const import IntegrateStr as Const
from scripts.core.capabilities import Capabilities as Capabilities
import re
# import audioop
from scripts.core.keys import CKeys as CKeys


def nb2TableSize(nb):
    return "[{}]".format(str(nb)) if nb > 1 else ""


def error_infos_file(conf, attribute):
    utils.printc("ERROR in %s infos files; %s must be defined " %(conf[CKeys.algo_name], attribute), color=colorama.Fore.RED)
    quit()


def upper_first(s):
    if s:  # Check if the string is not empty
        return s[0].capitalize() + s[1:]
    else:
        return ""

PREFIX = "audio_chain"

class Integrate(Args):
    """This class implements AFE processed as an x86 executable"""
    PATH_TO_REPO_ROOT = "../../../"
    default_txt_folder = os.path.join(CKeys.MakefileDir, "..", "logs")
    CALLBACKS = ["init", "deinit", "configure", "process", "dataInOut", "checkConsistency"]
    PARAM_FIELDS = ["pDefault", "pDescription", "pControl", "min" , "max" , "type"]
    TXT_PARAM_FIELDS = [ "pDescription", "type"]
    CONTROL_FIELDS = ["pDescription", "type"]
    MANDATORY_ATTRIBUTES = ["name", "prio_level", "description", "ios_consistency" , "callbacks" ]
    OPTIONAL_ATTRIBUTES = ["api_list", "source_list", "input_pins", "output_pins" , "static_params", "dynamic_params", "control_params" ]


    def __init__(self):
        super().__init__()
        self.nb_input = 0
        self.nb_output = 0
        self.verbose = self.args.verbose
        self.outdir = self.args.outdir
        self.root_dir = None
        if self.args.root_dir is not None:
            self.root_dir = os.path.normpath(self.args.root_dir)
        outdir = os.path.join(os.getcwd(), self.outdir)
        if self.args.no_clear is False:
            utils.rm_tree(outdir)
        if not utils.is_dir(outdir):
            utils.mkdir(outdir)

        self.log_file = os.path.normpath(os.path.join(self.outdir, self.args.log_filename))
        self.report_file = os.path.normpath(os.path.join(self.outdir, self.args.report_filename))
        self.report_file_handle = open(self.report_file, "w")
        self.log_file_handle = open(self.log_file, "w")
        self.id_card =  self.args.id_card

        self.hdle_plug_c = None
        self.hdle_wrp_c = None
        self.hdle_wrp_h = None
        self.hdle_factory = None

        self.capabilities = Capabilities()
        self.__load_parameters()
        self.__check_consistency()


    def __create_folder(self):
        outdir = os.path.join(os.getcwd(), self.outdir)
        outdir_algo = os.path.join(outdir, self.name)
        if not utils.is_dir(outdir_algo):
            utils.mkdir(outdir_algo)
        outdir_src = os.path.join(outdir, self.name, "src")
        if not utils.is_dir(outdir_src):
            utils.mkdir(outdir_src)
        outdir_wrp = os.path.join(outdir, self.name, "src", "wrapper")
        if not utils.is_dir(outdir_wrp):
            utils.mkdir(outdir_wrp)
        self.outdir_algo = outdir_algo
        self.outdir_src = outdir_src
        self.outdir_wrp = outdir_wrp

    def __check_consistency(self):
        def check_params_consistency(param_name, params_dict):
            error = False
            err_txt = ""
            for field in params_dict.keys():
                if field not in Integrate.PARAM_FIELDS:
                    err_txt = "ID_CARD CONSISTENCY ERROR: parameter \"{}\" has unknown field  \"{}\", please correct the ID_CARD\n".format(param_name, field)
                    err_txt += "The parameter should have the following fields {}".format(Integrate.PARAM_FIELDS)
                    utils.printc(err_txt, color=colorama.Fore.RED)
                    quit()
            if "type" not in params_dict.keys():
                error = True
                err_txt = "ID_CARD CONSISTENCY ERROR: parameter \"{}\" has no field  \"{}\", please complete the ID_CARD\n".format(param_name,"type")
            else:
                type = params_dict["type"]
                field_list = Integrate.TXT_PARAM_FIELDS if type.lower() == "txt" else Integrate.PARAM_FIELDS
                for field in field_list:
                    if field not in params_dict.keys():
                        error = True
                        err_txt = "ID_CARD CONSISTENCY ERROR: parameter \"{}\" has no field  \"{}\", please complete the ID_CARD\n".format(param_name,field)
            if error:
                utils.printc(err_txt,color=colorama.Fore.RED)
                quit()

        def check_control_consistency(param_name, params_dict):
            for field in params_dict.keys():
                if field not in Integrate.CONTROL_FIELDS:
                    txt = "ID_CARD CONSISTENCY ERROR: control parameter \"{}\" has unknown field  \"{}\", please correct the ID_CARD\n".format(param_name, field)
                    txt += "The parameter should have the following fields {}".format(Integrate.CONTROL_FIELDS)
                    utils.printc(txt, color=colorama.Fore.RED)
                    quit()

            for field in Integrate.CONTROL_FIELDS:
                if field not in params_dict.keys():
                    txt = "ID_CARD CONSISTENCY ERROR: control parameter \"{}\" has no field  \"{}\", please complete the ID_CARD\n".format(param_name,field)
                    utils.printc(txt,color=colorama.Fore.RED)
                    quit()

            # for field in list(set(Integrate.PARAM_FIELDS)-set(Integrate.CONTROL_FIELDS)):
            #     if field in params_dict.keys():
            #         txt = "ID_CARD CONSISTENCY ERROR: control parameter \"{}\" has a useless field  \"{}\", please remove from the ID_CARD\n".format(param_name,field)
            #         utils.printc(txt,color=colorama.Fore.YELLOW)


        if self.algo_infos.callbacks["configure"]  is not None and not self.has_dynamic_param:
            utils.printc("ID_CARD CONSISTENCY ERROR: if callback \"configure\" is defined, the field \"dynamic_params\" MUST be described! ", color=colorama.Fore.RED)
            quit()

        if self.algo_infos.callbacks["configure"] is None  and self.has_dynamic_param:
            utils.printc("ID_CARD CONSISTENCY ERROR: if callback \"configure\" is None, the field \"dynamic_params\" MUSTN'T be described! ", color=colorama.Fore.RED)
            quit()

        if self.algo_infos.callbacks["process"] is None and self.algo_infos.callbacks["dataInOut"] is None:
            txt = """ID_CARD CONSISTENCY ERROR: both callbacks "process" & "dataInOut" CAN'T be None."""
            txt += "\n\n Usage is:\n" + Const.HELP_DATAINOUT_PROCESS
            utils.printc(txt, color=colorama.Fore.RED)
            quit()

        if self.has_dynamic_param:
            for param_name, param_info in self.algo_infos.dynamic_params["params"].items():
                check_params_consistency(param_name, param_info)

        if self.has_static_param:
            for param_name, param_info in self.algo_infos.static_params["params"].items():
                check_params_consistency(param_name, param_info)

        if self.has_control_param:
            for param_name, param_info in self.algo_infos.control_params["params"].items():
                check_control_consistency(param_name, param_info)

    def __create_files(self):
        # create source file names
        prefix = PREFIX +"_" + self.name
        self.wrp_c = prefix + ".c"
        self.wrp_h = prefix + ".h"
        self.factory = prefix + "_factory.c"
        self.plug_c = "plugin_processing_" + self.name + ".c"

        self.hdle_wrp_c = open(os.path.join(self.outdir_wrp, self.wrp_c), "w")
        self.hdle_wrp_h = open(os.path.join(self.outdir_algo, self.wrp_h), "w")
        self.hdle_factory = open(os.path.join(self.outdir_algo, self.factory), "w")
        if self.has_control_param:
            self.hdle_plug_c = open(os.path.join(self.outdir_algo, self.plug_c), "w")

    def __copy_files(self):
        def __parse_and_copy(file_list):
            for file in file_list:
                search_files_list = utils.find([self.root_dir], file)
                for search_file in search_files_list:
                    if utils.is_file(search_file):
                        utils.copy_file(search_file, self.outdir_src)
        if self.args.no_file_copy is False:
            if self.algo_infos.api_list is not None and self.root_dir is not None:
                __parse_and_copy(self.algo_infos.api_list)
            if self.algo_infos.source_list is not None and self.root_dir is not None:
                __parse_and_copy(self.algo_infos.source_list)



    def __load_parameters(self):
        self.python_file_name = utils.get_python_file_name(self.id_card, "ID_CARD")
        self.algo_infos = utils.module_from_name(self.python_file_name)
        user_defined_attributes = [attr for attr in dir(self.algo_infos) if not attr.startswith('__')]

        # Consistency check around user defined fields
        for attr in user_defined_attributes:
            all_fields =  Integrate.MANDATORY_ATTRIBUTES + Integrate.OPTIONAL_ATTRIBUTES
            if attr not in all_fields:
                txt = "ID_CARD CONSISTENCY ERROR: field  \"{}\" is wrong , please correct the ID_CARD\n".format(attr)
                txt += "Did you mean: {}?\n\n".format(utils.find_closest_string(attr, all_fields))
                txt += "\t - the list of mandatory fields is: {}\n".format(Integrate.MANDATORY_ATTRIBUTES)
                txt += "\t - the list of optional fields is: {}\n".format(Integrate.OPTIONAL_ATTRIBUTES)
                utils.printc(txt, color=colorama.Fore.RED)
                quit()

        # Consistency check around mandatory fields
        for attr in Integrate.MANDATORY_ATTRIBUTES:
            if not hasattr(self.algo_infos, attr):
                txt = "ID_CARD CONSISTENCY ERROR: field  \"{}\" is missing, please correct the ID_CARD\n".format(attr)
                utils.printc(txt, color=colorama.Fore.RED)
                quit()

        # to avoid interpretation error and make further code simpler set all optional attributes/fields to None
        for attr in Integrate.OPTIONAL_ATTRIBUTES:
            if not hasattr(self.algo_infos, attr):
                setattr(self.algo_infos, attr, None)

        # do the same for callback elements
        for cb in Integrate.CALLBACKS:
            if cb not in self.algo_infos.callbacks.keys():
                self.algo_infos.callbacks[cb] = None

        # set some internal flags for simpler code later on
        self.has_input =  True if (hasattr(self.algo_infos, 'input_pins') and self.algo_infos.input_pins is not None)  else False
        self.has_output =  True if (hasattr(self.algo_infos, 'output_pins') and self.algo_infos.output_pins is not None) else False
        self.has_static_param = True if (hasattr(self.algo_infos, 'static_params') and self.algo_infos.static_params is not None) else False
        self.has_dynamic_param = True if (hasattr(self.algo_infos, 'dynamic_params') and self.algo_infos.dynamic_params is not None) else False
        self.has_control_param = True if (hasattr(self.algo_infos, 'control_params') and self.algo_infos.control_params is not None) else False

        if self.has_input:
            self.nb_input = len(self.algo_infos.input_pins["list"].keys())

        if self.has_output:
            self.nb_output = len(self.algo_infos.output_pins["list"].keys())
        self.name = self.algo_infos.name
        self.capabilities.build(self.algo_infos)
        self.__create_folder()
        self.__create_files()
        self.__copy_files()


    def __create_files_plug_c(self):
        def __print_typedef():
            txt ="typedef struct st_plugin_element_ext\n{\n  livetune_db_instance_cnx *pConnectedMsg;\n"
            self.hdle_plug_c.write(txt)
            for param_name, param_info in self.algo_infos.control_params["params"].items():
                type = param_info["type"].ljust(5)
                comment = param_info["pDescription"]
                self.hdle_plug_c.write("  {} {}; /*{}*/\n".format(type, param_name, comment))

            txt ="} st_plugin_element_ext;\n\n"
            self.hdle_plug_c.write(txt)

        if self.has_control_param:
            txt = Const.HEADER.replace("audio_chain_key_replace.EXT_REPLACE",self.plug_c)
            txt = txt.replace("audio_chain.c", "LiveTune").replace("key_replace",self.name)
            self.hdle_plug_c.write(txt)
            txt = Const.C_PLUG_HEADER.replace("key_replace",self.name)
            self.hdle_plug_c.write(txt)
            __print_typedef()
            self.hdle_plug_c.write(Const.C_PLUG_GLOBALS)
            self.hdle_plug_c.write(Const.C_PLUG_FUNC_BEG.replace("key_replace", self.name))
            self.hdle_plug_c.write(Const.C_PLUG_FUNC_MID)
            first_param = None
            for param_name, param_info in self.algo_infos.control_params["params"].items():
                if first_param is None:
                    first_param = param_name
                txt = "        ST_VERIFY(acAlgoGetControl(hAlgo, \"{}\", &pClsExt->{}) == 0);".format(param_name,param_name)
                self.hdle_plug_c.write(txt)

            self.hdle_plug_c.write(Const.C_PLUG_FUNC_END.replace("key_replace", first_param))
            self.hdle_plug_c.write(Const.C_PLUG_FOOTER.replace("key_replace", self.name))

    def __create_files_wrp_h(self):
        def generate_param_struct(param_dict, what):
            txt = "typedef struct\n{\n"
            if "struct_name" not in param_dict.keys():
                param_dict["struct_name"] = None

            if param_dict["struct_name"] is not None:
                struct_name = param_dict["struct_name"]
            else:
                struct_name = self.name + "_{}_t".format(what)
                param_dict["struct_name"] = struct_name

            self.hdle_wrp_h.write(txt)
            for param_name, param_info in param_dict["params"].items():
                type = param_info["type"].ljust(5)
                comment = param_info["pDescription"]
                self.hdle_wrp_h.write("  {} {}; /*{}*/\n".format(type, param_name, comment))
            self.hdle_wrp_h.write("}" + struct_name + ";\n\n")

        self.hdle_wrp_h.write(Const.HEADER.replace("key_replace",self.name).replace("EXT_REPLACE","h"))
        self.hdle_wrp_h.write(Const.H_HEADER.replace("KEY_REPLACE",self.name.upper()))

        if self.algo_infos.api_list is not None:
            for file in self.algo_infos.api_list:
                line = "#include \"{}/{}\"\n".format(self.name, file)
                self.hdle_wrp_h.write(line)
        else:
            # no api means we need to create structure from parameter description
            if self.has_static_param:
                generate_param_struct(self.algo_infos.static_params,"static_config")
            if self.has_dynamic_param:
                generate_param_struct(self.algo_infos.dynamic_params,"dynamic_config")
            if self.has_control_param:
                generate_param_struct(self.algo_infos.control_params,"control")

        line = Const.H_HEADER_FOOTER.replace("key_replace",self.name).replace("KEY_REPLACE",self.name.upper())
        self.hdle_wrp_h.write(line)
        line = Const.H_HEADER_CPP_FOOTER.replace("key_replace",self.name).replace("KEY_REPLACE",self.name.upper())
        self.hdle_wrp_h.write(line)


    def __create_files_wrp_c(self):
        self.hdle_wrp_c.write(Const.HEADER.replace("key_replace",self.name).replace("EXT_REPLACE","c"))

        self.__generate_includes()
        self.__generate_typdef()

        self.hdle_wrp_c.write(Const.C_DEFINES_COMMENT)
        self.hdle_wrp_c.write(Const.C_MEM_POOL.replace("KEY_REPLACE", self.name.upper()))
        self.hdle_wrp_c.write(Const.C_MACROS_COMMENT)
        self.hdle_wrp_c.write(Const.C_VAR_COMMENT)
        self.hdle_wrp_c.write(Const.C_FUNC_COMMENT)
        if self.args.data_management is not None:
            self.hdle_wrp_c.write(Const.C_CTXT_USAGE_COMMENT)


        self.__generate_prototypes()
        self.hdle_wrp_c.write(Const.C_GLOBALS_COMMENT)
        self.__generate_capabilities()
        self.__register_cbs()
        self.__generate_functions()


    def __generate_typdef(self):
        self.hdle_wrp_c.write(Const.C_TYPEDEF_COMMENT)
        if self.args.data_management is not None and (self.has_input or self.has_output):
            self.hdle_wrp_c.write(Const.C_TYPEDEF_START)
            self.hdle_wrp_c.write("  void *pReserved; /* dummy field, if specific field needed please store them here !*/\n")
            self.hdle_wrp_c.write("}"+Const.C_TYPEDEF_END.format(self.name+"_user"))

            self.hdle_wrp_c.write(Const.C_STRUCT_CTXT_BUFF.replace("key_replace",self.name) +"\n\n")

            self.hdle_wrp_c.write(Const.C_TYPEDEF_START)

            if self.args.var_request is not None:
                txt = ""
                for var_name, var_type in Const.C_CTXT_VAR_LIST.items():
                    if var_name in self.args.var_request:
                        txt += "  {}{};\n".format(var_type.ljust(Const.C_JUSTIFY_VAR_NAME-3), var_name)
                self.hdle_wrp_c.write(txt)
            justify_val = Const.C_JUSTIFY_VAR_TYPE-len(self.name)
            if self.algo_infos.input_pins is not None:
                for chunk in self.algo_infos.input_pins["list"].keys():
                    self.hdle_wrp_c.write("  {}_buffer_context_t {}{};\n".format(self.name, "".ljust(justify_val), chunk.lower()))

            if self.algo_infos.output_pins is not None:
                for chunk in self.algo_infos.output_pins["list"].keys():
                    self.hdle_wrp_c.write("  {}_buffer_context_t {}{};\n".format(self.name, "".ljust(justify_val), chunk.lower()))

            self.hdle_wrp_c.write("  {}_user_context_t {}{};\n".format(self.name, "".ljust(justify_val+2), "user"))

            self.hdle_wrp_c.write("}"+Const.C_TYPEDEF_END.format(self.name))

    def __generate_includes(self):
        def print_include(file):
            txt = ""
            extension = os.path.splitext(file)[1]
            if extension == ".h":
                if self.args.no_file_copy:
                    search_files_list = utils.find([self.root_dir], os.path.basename(file), absolute=True)
                    if len(search_files_list) == 0 :
                        txt += "#include \"{}\"\n".format(file)
                    for search_file in search_files_list:
                        if utils.is_file(search_file):
                            # searching CKeys.PythonRoot is a trick to know if it is our algo because the root include
                            # path in project is the same as the CKeys.PythonRoot
                            if os.path.normpath(CKeys.PythonRoot) in search_file:
                                search_file = search_file.split(os.path.normpath(CKeys.PythonRoot))[1]
                                name = os.path.dirname(search_file)[1:].replace('\\','/') + "/"
                                txt += "#include \"{}{}\"\n".format(name, os.path.basename(file))
                            else:
                                # in case CKeys.PythonRoot  is not found in file name, we copy the information
                                # from ID_card without managing it !
                                txt += "#include \"{}\"\n".format(file)


                else:
                    name = self.name + "/"
                    txt += "#include \"{}{}\"\n".format(name, file)
            return txt

        line =""
        if self.algo_infos.api_list is not None:
            for file in self.algo_infos.api_list:
                line += print_include(file)
        if self.algo_infos.source_list is not None:
            for file in self.algo_infos.source_list:
                line += print_include(file)

        line = Const.C_INCLUDES.replace("key_replace",self.name).replace("key_extra_include",line)
        self.hdle_wrp_c.write(line)


    def __generate_prototypes(self):
        for key, value in self.algo_infos.callbacks.items():
            if value is not None:
                line = self.__generate_proto(key) +  ";\n"
                self.hdle_wrp_c.write(line)

    def __generate_capabilities(self):
        txt = "const audio_algo_common_t AudioChainWrp_{}_common =\n".format(self.name)
        self.hdle_wrp_c.write(txt + "{\n")
        self.hdle_wrp_c.write("  .pName".ljust(Const.C_JUSTIFY_VAR_CAPA+3)+"= \"{}\",\n".format(self.name))

        built_capabilities = self.capabilities.get()
        for key, value in built_capabilities.items():
            if "pAlgoDesc" in key:
                value = "\""+value + "\""
            # utils.printc("++++++++++++++++++++++++++ " + key + "  " + value, color=colorama.Fore.RED)
            line = "  ."+ key.ljust(Const.C_JUSTIFY_VAR_CAPA) + "= " + value + ",\n"
            self.hdle_wrp_c.write(line)
        if self.has_control_param is True:
            line = "  .misc.flags                = AUDIO_ALGO_FLAGS_MISC_DISABLE_AUTO_MOUNT\n"
            self.hdle_wrp_c.write(line)

        self.hdle_wrp_c.write("};\n\n")

    def __register_cbs(self):
        txt = "audio_algo_cbs_t AudioChainWrp_{}_cbs =\n".format(self.name)
        self.hdle_wrp_c.write(txt+ "{\n")
        for key, value in self.algo_infos.callbacks.items():
            line = "  ." + key.ljust(27)
            if value is not None:
                line += "= s_" + value + ",\n"
            else:
                line +=  "= NULL,\n"
            self.hdle_wrp_c.write(line)

        self.hdle_wrp_c.write("  .control                    = NULL, /* Do not register any function in here. It is done through user API. */\n")
        self.hdle_wrp_c.write("  .isDisabled                 = NULL, /* Allows to define a behavior when algorithm is bypassed */ \n")
        self.hdle_wrp_c.write("  .isDisabledCheckConsistency = NULL\n")
        self.hdle_wrp_c.write("};\n\n")

    def __generate_algo_call(self, cb, add_comment=True, error_check=Const.C_ERROR_OK_CHECK):
        self.hdle_wrp_c.write(error_check)
        # self.hdle_wrp_c.write(Const.C_CTXT_GET)
        if add_comment:
            comment = ", /* ADD YOUR PARAM IF ANY */"
        else:
            comment = ""
        line = "    error = {}(pAlgo{});\n".format(cb, comment)
        self.hdle_wrp_c.write(line)
        self.hdle_wrp_c.write("  }\n")

    def __generate_init_function(self, cb_name):
        def print_local_var(in_out):
            txt = ""
            if self.args.var_request is not None:
                for var_name, var_type in Const.C_CTXT_VAR_LIST.items():
                    if var_name in self.args.var_request:
                        txt += "  {} const {} = AudioBuffer_get{}(pBuff{});\n".format(var_type.ljust(Const.C_JUSTIFY_VAR_NAME+1), var_name.ljust(Const.C_JUSTIFY_VAR_VAL+5), upper_first(var_name), in_out)
            return txt

        if self.has_static_param:
            self.hdle_wrp_c.write(Const.C_STATIC_CONF_CHECK.replace("key_replace", self.algo_infos.static_params["struct_name"]))
        if self.args.data_management is not None:
            just_var_name = Const.C_JUSTIFY_VAR_NAME-20
            just_var_val = Const.C_JUSTIFY_VAR_VAL - 1
            if self.has_input:
                self.hdle_wrp_c.write(Const.C_CHUNK_DECLARE.format("".ljust(just_var_name),"In".ljust(just_var_val), "In"))
                self.hdle_wrp_c.write(Const.C_BUFF_DECLARE.format("".ljust(just_var_name),"In".ljust(just_var_val+1), "In"))
            if self.has_output:
                self.hdle_wrp_c.write(Const.C_CHUNK_DECLARE.format("".ljust(just_var_name),"Out".ljust(just_var_val), "Out"))
                self.hdle_wrp_c.write(Const.C_BUFF_DECLARE.format("".ljust(just_var_name),"Out".ljust(just_var_val+1), "Out"))

            if self.has_input:
                txt = print_local_var("In")
                self.hdle_wrp_c.write(txt)
            else:
                txt = print_local_var("Out")
                self.hdle_wrp_c.write(txt)

            if self.has_input or self.has_output:
                self.hdle_wrp_c.write(
                    Const.C_CTXT_ALLOCATE.replace("key_replace", self.name).replace("KEY_REPLACE", self.name.upper()))
                self.hdle_wrp_c.write(Const.C_ERROR_OK_CHECK)

                if self.args.var_request is not None:
                    txt = ""
                    for var_name, var_type in Const.C_CTXT_VAR_LIST.items():
                        if var_name in self.args.var_request:
                            txt += "    pContext->{} = {};\n".format(var_name.ljust(15), var_name)
                    self.hdle_wrp_c.write(txt)

                if self.algo_infos.input_pins is not None:
                    i = 0
                    for chunk in self.algo_infos.input_pins["list"].keys():
                        pChunk = self.__get_pChunk(chunk)
                        self.hdle_wrp_c.write("    {}      = AudioAlgo_getChunkPtrIn(pAlgo, {}U);\n".format(pChunk, i))
                        i += 1
                if self.algo_infos.output_pins is not None:
                    i = 0
                    for chunk in self.algo_infos.output_pins["list"].keys():
                        pChunk = self.__get_pChunk(chunk)
                        self.hdle_wrp_c.write("    {}     = AudioAlgo_getChunkPtrOut(pAlgo, {}U);\n".format(pChunk, i))
                        i += 1
                self.hdle_wrp_c.write("  }\n")
        self.__generate_algo_call(cb_name)

        if self.has_dynamic_param:
            self.hdle_wrp_c.write(Const.C_ERROR_OK_CHECK)
            line = "    error = s_{}_configure(pAlgo);\n".format(self.name)
            self.hdle_wrp_c.write(line)
            self.hdle_wrp_c.write("  }\n")

        if self.has_control_param:
            txt = Const.C_CONTROL_MALLOC.replace("KEY_REPLACE", self.name.upper())
            txt = txt.replace("struct_replace", self.algo_infos.control_params["struct_name"])
            self.hdle_wrp_c.write(txt)

        self.__generate_algo_call("s_{}_deinit".format(self.name), add_comment=False, error_check=Const.C_ERROR_NOK_CHECK)

    def __generate_deinit_function(self, cb_name):
        self.hdle_wrp_c.write(self.__generate_contextDeclare())

        if self.has_control_param:
            self.hdle_wrp_c.write(self.__generate_controlDeclare())
            txt = Const.C_CONTROL_FREE.replace("KEY_REPLACE", self.name.upper())
            self.hdle_wrp_c.write(txt)

        if self.args.data_management is not None:
            self.hdle_wrp_c.write(
                Const.C_CTXT_FREE.replace("key_replace", self.name).replace("KEY_REPLACE", self.name.upper()))
        self.__generate_algo_call(cb_name)

    def __get_pChunk(self, chunk_name):
        return "pContext->{}.pChunk".format(chunk_name.lower())

    def __get_buff(self, chunk_name):
        return "pContext->{}.buff".format(chunk_name.lower())

    def __generate_dataPtrDeclare(self, chunk_name, in_out):
        readWrite = "Read"
        rdWr = "Rd"
        const = "const"
        if in_out.lower() == "out":
            readWrite = "Write"
            rdWr = "Wr"
            const = ""
        txt = ""
        if self.args.data_management == "untyped" or self.args.data_management == "all":
            pChunk = self.__get_pChunk(chunk_name)
            txt += "\n  /* Following line only needed if untyped data pointers are used to manage samples */\n"
            txt += "  uint8_t".ljust(22 -len(const)) + const + " *const " + chunk_name + "".ljust(17-len(chunk_name))
            txt += "  = (uint8_t {}*)AudioChunk_get{}Ptr({}, 0UL, 0UL);\n".format(const, readWrite, pChunk)
        if self.args.data_management == "audio_buffer" or self.args.data_management == "all":
            buff = self.__get_buff(chunk_name)
            txt += "\n  /* Following line only needed if audio_buffer_t is used to manage samples */\n"
            txt += "  error = AudioChunk_setAudioBufPtrFrom{}Ptr({},&{});\n".format(rdWr,pChunk, buff)
        return txt

    def __generate_contextDeclare(self):
        txt = ""
        if self.args.data_management is not None:
            txt = "  {}_context_t".format(self.name)
            txt += "".ljust(Const.C_JUSTIFY_VAR_NAME-9-len(self.name))
            txt += "*const pContext".ljust(Const.C_JUSTIFY_VAR_VAL+13)
            txt += "= ({}_context_t *)AudioAlgo_getWrapperContext(pAlgo);\n".format(self.name, self.name)
        return txt

    def __generate_controlDeclare(self, justified=True):
        txt = ""
        if justified is True:
            just_val1= Const.C_JUSTIFY_VAR_NAME-10-len(self.name)
            just_val2= Const.C_JUSTIFY_VAR_VAL+13
        else:
            just_val1 = 1
            just_val2 = 1

        if self.has_control_param:
            struct_name = self.algo_infos.control_params["struct_name"]
            txt = "  {} ".format(struct_name)
            txt += "".ljust(just_val1)
            txt += "*const pCtrl".ljust(just_val2)
            txt += "= ({} *)AudioAlgo_getCtrlData(pAlgo);\n".format(struct_name)
        return txt


    def __generate_dataInOut_function(self, cb_name):
        if self.args.data_management is not None:
            self.hdle_wrp_c.write(self.__generate_contextDeclare())
            if self.args.var_request is not None:
                for var_name, var_type in Const.C_CTXT_VAR_LIST.items():
                    if var_name in self.args.var_request:
                        txt = "  {}{} = pContext->{};\n".format(var_type.ljust(Const.C_JUSTIFY_VAR_NAME+8), var_name.ljust(Const.C_JUSTIFY_VAR_VAL+5), var_name)
                        self.hdle_wrp_c.write(txt)
            if self.algo_infos.input_pins is not None:
                for chunk in self.algo_infos.input_pins["list"].keys():
                    txt =  self.__generate_dataPtrDeclare(chunk, "in")
                    self.hdle_wrp_c.write(txt)
            if self.algo_infos.output_pins is not None:
                for chunk in self.algo_infos.output_pins["list"].keys():
                    txt =  self.__generate_dataPtrDeclare(chunk, "out")
                    self.hdle_wrp_c.write(txt)
            self.hdle_wrp_c.write("\n")
        self.__generate_algo_call(cb_name)
        self.hdle_wrp_c.write(Const.C_ERROR_OK_CHECK)
        txt = "    AudioAlgo_incReadyForProcess(pAlgo);\n /* Needs to be called when enough data to run process callback; do not call otherwise */\n"
        self.hdle_wrp_c.write(txt)
        self.hdle_wrp_c.write("  }\n")
        if self.algo_infos.callbacks["process"] is None:
            self.__generate_control_increment()

    def __generate_control_increment(self):
        if self.has_control_param:
            self.hdle_wrp_c.write(Const.C_ERROR_OK_CHECK)
            self.hdle_wrp_c.write("  " + self.__generate_controlDeclare(justified=False))
            for param_name, param_info in self.algo_infos.control_params["params"].items():
                self.hdle_wrp_c.write("    pCtrl->{} = /* update control value */;\n".format(param_name))
            self.hdle_wrp_c.write("    AudioAlgo_incReadyForControl(pAlgo); /* Can be filtered to avoid two many control cb calls */\n")
            self.hdle_wrp_c.write("  }\n")

    def __generate_functions(self):
        def add_proto(cb_name):
            self.hdle_wrp_c.write("\n\n")
            line = self.__generate_proto(cb_name) + "\n"
            self.hdle_wrp_c.write(line + "{\n")
            self.hdle_wrp_c.write(Const.C_ERROR_DECLARE.format("".ljust(Const.C_JUSTIFY_VAR_NAME), "".ljust(Const.C_JUSTIFY_VAR_VAL)))
        def end_function():
            line = "  return error;\n"
            self.hdle_wrp_c.write(line)
            self.hdle_wrp_c.write("}\n")

        self.hdle_wrp_c.write(Const.C_FUNC_DECLARE_COMMENT)

        for key, value in self.algo_infos.callbacks.items():
            if "init" == key.lower():
                add_proto(key)
                self.__generate_init_function(value)
                end_function()

            elif "deinit" == key.lower():
                add_proto(key)
                self.__generate_deinit_function(value)
                end_function()

            elif "configure" == key.lower() and self.has_dynamic_param:
                add_proto(key)
                self.hdle_wrp_c.write(Const.C_DYNAMIC_CONF_CHECK.replace("key_replace", self.algo_infos.dynamic_params["struct_name"]))
                self.hdle_wrp_c.write(self.__generate_contextDeclare())
                self.__generate_algo_call(value)
                end_function()

            elif "datainout" == key.lower():
                if value is None:
                    txt = self.__generate_proto(key) + "\n"
                    self.hdle_wrp_c.write(txt + "{\n")
                    self.hdle_wrp_c.write(Const.C_ERROR_DECLARE.format("".ljust(Const.C_JUSTIFY_VAR_NAME), "".ljust(Const.C_JUSTIFY_VAR_VAL)))
                    txt = "  AudioAlgo_incReadyForProcess(pAlgo);\n"
                    self.hdle_wrp_c.write(txt)
                    txt = "  return error;\n"
                    self.hdle_wrp_c.write(txt)
                    self.hdle_wrp_c.write("}\n")
                else:
                    add_proto(key)
                    self.__generate_dataInOut_function(value)
                    end_function()

            elif "process" == key.lower():
                if self.algo_infos.callbacks["dataInOut"] is None:
                    add_proto(key)
                    self.__generate_dataInOut_function(value)
                elif value is not None:
                    add_proto(key)
                    self.__generate_algo_call(value)
                    self.__generate_control_increment()
                    end_function()

            elif value is not None:
                add_proto(key)
                self.__generate_algo_call(value)
                end_function()

    def __generate_proto(self, cb):
        return "static int32_t s_{}_{}(audio_algo_t *const pAlgo)".format(self.name, cb)


    def __create_files_factory(self):
        def convert_type(type):
            if type == "float":
                return "F"
            elif  type == "int32_t":
                return "S32"
            elif  type == "int":
                return "S32"
            elif  type == "int16_t":
                return "S16"
            elif  type == "int8_t":
                return "S8"
            elif  type == "uint32_t":
                return "U32"
            elif  type == "uint16_t":
                return "U16"
            elif  type == "uint8_t":
                return "U8"
        def convert_param_val(type, val):
            val = str(val)
            if type == "float":
                return val + "f"
            elif  type == "int32_t":
                return val
            elif  type == "int":
                return val
            elif  type == "int16_t":
                return val
            elif  type == "int8_t":
                return val
            elif  type == "uint32_t":
                return val+"UL"
            elif  type == "uint16_t":
                return val+"U"
            elif  type == "uint8_t":
                return val+"U"

        def print_param(struct_name,param_list, isControl=False):
            for i, (pname, pinfo) in enumerate(param_list.items()):
                justify_val = 13
                self.hdle_factory.write("  {\n")
                line = "    {} = {},\n".format(".pName".ljust(justify_val), '"{}"'.format(pname))
                self.hdle_factory.write(line)
                for info_name, info_val in pinfo.items():
                    if info_name == "pDescription" or info_name == "pControl":
                        if info_name == "pControl" and info_val not in self.capabilities.get_pControl_list():
                            txt = "Error with parameter \"{}\" wrong pControl: \"{}\" does not belong to possible values\n".format(
                                pname,  info_val)
                            txt += "possible values are {}\n".format(str(self.capabilities.get_pControl_list()))
                            utils.printc(txt, color=colorama.Fore.RED)
                        line = "    {} = AUDIO_ALGO_OPT_STR({}),\n".format('.{}'.format(info_name).ljust(justify_val),'"{}"'.format(info_val))
                    elif info_name != "min" and info_name != "max" and info_name != "type":
                        line = "    {} = {},\n".format('.{}'.format(info_name).ljust(justify_val), '"{}"'.format(info_val))
                    else:
                        continue
                    self.hdle_factory.write(line)
                if isControl is True:
                    line = "    AUDIO_DESC_CONTROL_{}({}, {})".format(convert_type(pinfo["type"]),struct_name, pname)
                else:
                    line = "    AUDIO_DESC_PARAM_{}({}, {}, {}, {})".format(convert_type(pinfo["type"]),struct_name, pname, convert_param_val(pinfo["type"],pinfo["min"]),  convert_param_val(pinfo["type"],pinfo["max"]))

                self.hdle_factory.write(line)
                self.hdle_factory.write("\n  }")
                if i == len(param_list) - 1:
                    self.hdle_factory.write("\n")
                else:
                    self.hdle_factory.write(",\n")


        self.hdle_factory.write(Const.HEADER.replace("key_replace",self.name+"_factory").replace("EXT_REPLACE","c"))
        line =""
        if self.algo_infos.api_list is not None:
            for file in self.algo_infos.api_list:
                extension = os.path.splitext(file)[1]
                if extension == ".h":
                    line += "#include \"{}/{}\"\n".format(self.name, file)

        line = Const.C_INCLUDES.replace("key_replace",self.name).replace("key_extra_include","")
        self.hdle_factory.write(line)
        self.hdle_factory.write(Const.C_TYPEDEF_COMMENT)
        self.hdle_factory.write(Const.C_DEFINES_COMMENT)
        self.hdle_factory.write(Const.C_MACROS_COMMENT)
        self.hdle_factory.write(Const.C_VAR_COMMENT)
        self.hdle_factory.write(Const.C_FUNC_COMMENT)

        self.hdle_factory.write(Const.C_FACTORY_HEADER)

        if self.has_static_param:
            line = "static const audio_descriptor_param_t s_{}_staticParamsDesc[] =\n".format(self.name)

            struct_name = self.algo_infos.static_params["struct_name"]
            param_list = self.algo_infos.static_params["params"]
            self.hdle_factory.write(line + "{\n")
            print_param(struct_name, param_list)
            self.hdle_factory.write("};\n")

        if self.has_dynamic_param:
            line = "static const audio_descriptor_param_t s_{}_dynamicParamsDesc[] =\n".format(self.name)

            struct_name = self.algo_infos.dynamic_params["struct_name"]
            param_list = self.algo_infos.dynamic_params["params"]
            self.hdle_factory.write(line + "{\n")
            print_param(struct_name, param_list)
            self.hdle_factory.write("};\n")

        if self.has_control_param:
            line = "static const audio_algo_control_param_t s_{}_controlParamsDesc[] =\n".format(self.name)

            struct_name = self.algo_infos.control_params["struct_name"]
            param_list = self.algo_infos.control_params["params"]
            self.hdle_factory.write(line + "{\n")
            print_param(struct_name, param_list,isControl=True)
            self.hdle_factory.write("};\n")

        line = "END_IGNORE_DIV0_WARNING\n/*cstat +MISRAC2012-Rule-1.4_a*/\n"
        self.hdle_factory.write(line)

        if self.has_static_param:
            struct_name = self.algo_infos.static_params["struct_name"]
            line = Const.C_FACTORY_PARAM_DECLARE.replace("key_replace", self.name).replace("type_replace", "static").replace("struct_replace", struct_name)
            self.hdle_factory.write(line)

        if self.has_dynamic_param:
            struct_name = self.algo_infos.dynamic_params["struct_name"]
            line = Const.C_FACTORY_PARAM_DECLARE.replace("key_replace", self.name).replace("type_replace", "dynamic").replace("struct_replace", struct_name)
            self.hdle_factory.write(line)

        if self.has_control_param:
            line = Const.C_FACTORY_CONTOL_DECLARE.replace("key_replace", self.name)
            self.hdle_factory.write(line)

        line = Const.C_FACTORY_FOOTER.replace("key_replace", self.name)
        if self.has_static_param:
            line = line.replace("pointer_static_replace","AUDIO_ALGO_OPT_TUNING(&s_{}_staticParamTemplate)".format(self.name))
        else:
            line = line.replace("pointer_static_replace","AUDIO_ALGO_OPT_TUNING(NULL)")

        if self.has_dynamic_param:
            line = line.replace("pointer_dynamic_replace", "AUDIO_ALGO_OPT_TUNING(&s_{}_dynamicParamTemplate)".format(self.name))
        else:
            line = line.replace("pointer_dynamic_replace", "AUDIO_ALGO_OPT_TUNING(NULL)")

        if self.has_control_param:
            line = line.replace("pointer_control_replace", "AUDIO_ALGO_OPT_TUNING(&s_{}_controlParamTemplate)".format(self.name))
        else:
            line = line.replace("pointer_control_replace", "AUDIO_ALGO_OPT_TUNING(NULL)")


        self.hdle_factory.write(line)

    def quit(self):
        txt = "Find generated files inside {}\{}\n".format(os.getcwd(), self.outdir)
        txt += "Find script logs inside the file {}\{}\n" .format(os.getcwd(),  self.log_file_handle.name)
        utils.printc(txt=txt, color=colorama.Back.WHITE + colorama.Fore.BLUE, verbose=True)

    def run(self ):

        self.__create_files_wrp_h()
        self.__create_files_wrp_c()
        self.__create_files_factory()
        self.__create_files_plug_c()

        self.report_file_handle.close()
        self.log_file_handle.close()
        self.hdle_wrp_c.close()
        self.hdle_wrp_h.close()
        if self.hdle_plug_c is not None:
            self.hdle_plug_c.close()
        self.hdle_factory.close()

        self.quit()