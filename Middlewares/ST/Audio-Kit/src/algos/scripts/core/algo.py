import os  # for system commands
import scripts.core.utils as utils
import colorama
from scripts.core.keys import CKeys as CKeys
from numpy import save, savetxt, load
import numpy as np

import matplotlib.pyplot as plt
import copy
from scripts.core.audio_chunk import AudioChunk as AudioChunk

def print_weak_warning(log_hdle, func_name):
    log_hdle("%s weak should not be used but overwritten" % func_name, colorama.Fore.RED)


class Algo:
    """This class implements a generic algo"""

    def __init__(self, verbose=False, log_file_handle=None):
        self.log_file_handle = log_file_handle
        self.log = utils.Log(verbose=verbose, log_file_handle=log_file_handle)  # permit to call self.log.log(...) without error

        self.params = dict()
        self.params_list = []
        self.benched_params_list = []

        # init internals
        self.version = "x.y.z"
        self.full_outdir = None
        self.outdir = None
        self.filename = ""
        self.cfilename = ""
        self.hfilename = ""
        self.param_config_define = ""
        self.header_guard = ""
        self.c_file_handler = None
        self.h_file_handler = None
        self.cIncludeFileHandler = None
        self.hIncludeFileHandler = None
        self.provider = "STMicroelectronics"
        self.param_config_str = ""
        self.param_config_str_list = []
        self.build_cmd_option = None
        self.id = 0
        # init for matplot figures
        self.angles = None
        self.fig = None
        self.ax = None

        # audioChain stuff
        self.in_chunk_list = []
        self.out_chunk_list = []
        self.nb_out_chunk = 0
        self.in_samples = {}
        self.out_samples = {}
        self.user_name = os.getenv('USERNAME')

    def set_name(self, name):
        self.add_user_param(p_name=CKeys.AlgoName, p_val=name, short="name")

    def get_name(self):
        return self.get_param_val(CKeys.AlgoName)

    def set_id(self, idx):
        self.id = idx

    def get_id(self):
        return self.id

    def get_name_id(self):
        return self.get_name() + str(self.get_id())

    def is_build_option_set(self):
        return False if self.build_cmd_option is None else True

    @staticmethod
    def sort_internal_params(params):
        to_be_ordered_dict = {}
        unordered_list = []
        for p_name, params_dict in params.items():
            if CKeys.order_id in params_dict.keys():
                to_be_ordered_dict[p_name] = params_dict
            else:
                unordered_list.append(p_name)
        ordered_dict = dict(sorted(to_be_ordered_dict.items(), key=lambda x: x[1][CKeys.order_id], reverse=False))
        ordered_list = list(ordered_dict.keys())
        params_list = ordered_list + unordered_list
        return params_list

    @staticmethod
    def check_internal_params_consistency(internal_params):
        for p_name in internal_params.keys():
            assert (internal_params[p_name][CKeys.value] in internal_params[p_name][CKeys.list]),\
                "parameter '%s':%s not in %s" % (
                    p_name, str(internal_params[p_name][CKeys.value]), str(internal_params[p_name][CKeys.list]))

    def add_user_param(self, p_name, p_val, benched=False, doc="", short=""):
        if p_name in self.params.keys():
            self.log.log("add_user_param: param '%s' is already present -> use update_user_param instead" % p_name,
                         color=colorama.Fore.YELLOW)
            self.update_user_param(p_name, p_val, benched, doc)
        else:
            # param is new
            self.params[p_name] = {
                CKeys.value: p_val,
                CKeys.list: [p_val],
                CKeys.benched: benched,
                CKeys.doc: doc,
                CKeys.short: short
            }
            self.params_list.append(p_name)
            if benched:
                self.benched_params_list.append(p_name)

    def update_user_param(self, p_name, p_val, benched=False, doc="", short=""):
        if p_name in self.params.keys():
            # param is already present
            self.params[p_name][CKeys.value] = p_val
            if CKeys.list in self.params[p_name].keys():
                if p_val not in self.params[p_name][CKeys.list]:
                    self.params[p_name][CKeys.list].append(p_val)
            else:
                self.params[p_name][CKeys.list] = [p_val]
#            if CKeys.benched in self.params[p_name]:
#                assert (benched == self.params[p_name][CKeys.benched]), \
#                    "param '%s' benched flag has changed: '%s' -> '%s'" % (
#                        p_name, str(self.params[p_name][CKeys.benched]), str(benched))
#            else:
            self.params[p_name][CKeys.benched] = benched
            if benched and (p_name not in self.benched_params_list):
                self.benched_params_list.append(p_name)
            if (CKeys.doc in self.params[p_name]) and (self.params[p_name][CKeys.doc] != ""):
                assert (doc == self.params[p_name][CKeys.doc]), \
                    "param '%s' doc has changed: '%s' -> '%s'" % (
                        p_name, str(self.params[p_name][CKeys.doc]), str(doc))
            else:
                self.params[p_name][CKeys.doc] = doc
 #           if (CKeys.short in self.params[p_name]) and (self.params[p_name][CKeys.short] != ""):
 #               assert (short == self.params[p_name][CKeys.short]), \
 #                   "param '%s' short has changed: '%s' -> '%s'" % (
 #                       p_name, str(self.params[p_name][CKeys.short]), str(short))
 #           else:
            self.params[p_name][CKeys.short] = short
        else:
            # param is new
            self.log.log("update_user_param: param '%s' is not present -> use add_user_param instead" % p_name,
                         color=colorama.Fore.YELLOW)
            self.add_user_param(p_name, p_val, benched, doc, short)

    def load_user_params(self, params):
        for p_name, p_val in params.items():
            self.add_user_param(p_name, p_val)

    def load_params(self, params):
        if params is not None:
            self.load_user_params(params)
        else:
            self.update_benched_params_list(self.params)

    def update_benched_params_list(self, params_dict):
        params_list = Algo.sort_internal_params(params_dict)
        for p_name in params_list:
            if params_dict[p_name][CKeys.benched] and (p_name not in self.benched_params_list):
                self.benched_params_list.append(p_name)

    @staticmethod
    def reset_internal_param_dict(params_dict):
        if CKeys.benched not in params_dict.keys():
            params_dict[CKeys.benched] = False
        if CKeys.value not in params_dict.keys():
            params_dict[CKeys.value] = None
        # set doc in params_dict (empty if absent in params_dict)
        if CKeys.doc not in params_dict.keys():
            params_dict[CKeys.doc] = ""
        if CKeys.short not in params_dict.keys():
            params_dict[CKeys.short] = ""

    def set_internal_param_list(self, p_name, params_dict):
        if CKeys.list in params_dict.keys():
            self.params[p_name][CKeys.list] = params_dict[CKeys.list]

    def add_internal_param(self, p_name, params_dict):
        self.reset_internal_param_dict(params_dict)
        self.add_user_param(p_name,
                            params_dict[CKeys.value],
                            params_dict[CKeys.benched],
                            params_dict[CKeys.doc],
                            params_dict[CKeys.short])
        self.set_internal_param_list(p_name, params_dict)

    def update_internal_param(self, p_name, params_dict):
        self.reset_internal_param_dict(params_dict)
        self.update_user_param(p_name,
                               params_dict[CKeys.value],
                               params_dict[CKeys.benched],
                               params_dict[CKeys.doc],
                               params_dict[CKeys.short])

    def load_internal_params(self, params):
        params_list = self.sort_internal_params(params)
        for p_name in params_list:
            if p_name in self.params.keys():
                self.update_internal_param(p_name, params[p_name])
            else:
                self.add_internal_param(p_name, params[p_name])

    def remove_param(self, p_name):
        self.params_list.remove(p_name)
        if (CKeys.benched in self.params[p_name].keys()) and self.params[p_name][CKeys.benched]:
            self.benched_params_list.remove(p_name)
        del self.params[p_name]

    def load_params_from_algo(self, algo):
        self.params = copy.deepcopy(algo.params)
        self.params_list = copy.deepcopy(algo.params_list)
        self.benched_params_list = copy.deepcopy(algo.benched_params_list)

    def save_params_val(self):
        params_save = {}
        for p_name in self.benched_params_list:
            params_save[p_name] = self.params[p_name][CKeys.value]
        return params_save

    def restore_params_val(self, params_save):
        for p_name in params_save.keys():
            self.params[p_name][CKeys.value] = params_save[p_name]

    def compute_algo(self):
        print_weak_warning(self.log.log, self.compute_algo.__name__)

    def write_c_file(self):
        print_weak_warning(self.log.log, self.write_c_file.__name__)

    def write_h_file(self):
        print_weak_warning(self.log.log, self.write_h_file.__name__)

    def get_version(self):
        return self.version

    def set_param_val(self, p_name, value):
        self.params[p_name][CKeys.value] = value

    def get_param_val(self, p_name, to_hashable=False, to_str=False, remove_quotes=True):
        if (p_name in self.params.keys()) and (CKeys.value in self.params[p_name].keys()):
            val = self.params[p_name][CKeys.value]
            if type(val) is str:
                if remove_quotes:
                    while (val[0] == '"') and (val[-1] == '"'):
                        val = val[1: -1]
            elif to_str:
                val = str(val)
            elif to_hashable and not val.__hash__:
                val = str(val)
            return val
        else:
            return None

    def get_param_short(self, p_name, upper_case=False, lower_case=False):
        if (p_name in self.params.keys()) and (CKeys.short in self.params[p_name].keys()):
            ret = self.params[p_name][CKeys.short]
            if upper_case:
                ret = ret.upper()
            if lower_case:
                ret = ret.lower()
            return ret
        return None

    def get_param_doc(self, p_name):
        if (p_name in self.params.keys()) and (CKeys.doc in self.params[p_name].keys()):
            return self.params[p_name][CKeys.doc]
        else:
            return ""

    def is_param_benched(self, p_name):
        if (p_name in self.params.keys()) and (CKeys.benched in self.params[p_name].keys()):
            return self.params[p_name][CKeys.benched]
        else:
            return False

    def set_param_val_list(self, p_name, val_list):
        self.params[p_name][CKeys.list] = val_list

    def get_param_val_list(self, p_name):
        if p_name in self.params:
            if CKeys.list in self.params[p_name].keys():
                return self.params[p_name][CKeys.list]
            else:
                return [self.params[p_name][CKeys.value]]
        else:
            return []

    def set_outdir(self, folder, clear=False, mkdir=False):
        self.full_outdir = os.path.normpath(folder)
        self.outdir = os.path.basename(self.full_outdir)
        if clear:
            self.clear_outdir()
        if mkdir:
            self.make_outdir()

    def clear_outdir(self):
        if utils.is_dir(self.full_outdir):
            utils.rm_tree(self.full_outdir)

    def make_outdir(self):
        if not utils.is_dir(self.full_outdir):
            utils.mkdir(self.full_outdir)

    def convert_vector_to_txt(self, vector, idx=None):
        elements_nb = vector.size
        txt = ""
        algo_name = self.get_name()
        coef_name = '%s' % algo_name
        coef_name += '%d' % idx if idx is not None else None
        txt += 'static const float %s[2 * %d] =\n{\n' % (coef_name, elements_nb)
        txt += utils.table_complex_2_txt(vector)
        txt += '\n};\n\n'
        txt += '%s_Coeff_t const %sCoeff' % (algo_name.upper(), algo_name.lower())
        txt += '%d' % idx if idx is not None else None
        txt += ' =\n{\n'
        txt += '  .pCoeff = %s,\n' % coef_name
        txt += '  .nbElements = %d\n' % elements_nb
        txt += '};\n\n'

        return txt

    def compute_filename_define(self):
        txt = self.get_name()
        for param_name in self.benched_params_list:
            param_dict = self.params[param_name]
            param_value = param_dict[CKeys.value]
            param_name_short = param_dict[CKeys.short]
            if isinstance(param_value, list):
                for p_val in param_value:
                    txt += '_' + param_name_short + utils.param_value_to_txt(p_val).replace('-', '_')
            # todo: case of dict if needed one day
            # elif isinstance(self.params[p_name], dict):
            #     txt += params
            else:
                txt += '_' + param_name_short + utils.param_value_to_txt(param_value).replace('-', '_')
        self.filename = txt.replace('.', 'p')
        self.param_config_str = self.filename.upper()
        self.param_config_define = 'USE_%s' % self.param_config_str
        if self.param_config_str not in self.param_config_str_list:
            self.param_config_str_list.append(self.param_config_str)

    def get_conf_id(self):
        return self.param_config_str_list.index(self.param_config_str)

    def sfc(self):
        return self.build_cmd_option

    def set_build_cmd_option(self, build_cmd_option):
        self.build_cmd_option = build_cmd_option

    def compute_c_file_names(self):
        self.compute_filename_define()
        self.cfilename = '%s.c' % self.filename
        self.hfilename = '%s.h' % self.filename
        self.header_guard = '__%s_H' % self.param_config_str

    def compute_c_file_header(self):
        txt = ""
        txt += '#ifdef %s\n\n' % self.param_config_define
        for p_name in self.params_list:
            param_dict = self.params[p_name]
            param_value = param_dict[CKeys.value]
            if isinstance(param_value, list):
                for p_val in param_value:
                    txt += '// - ' + param_dict[CKeys.short] + ' : ' + utils.param_value_to_txt(p_val)
            # todo: case of dict if needed one day
            # elif isinstance(self.params[p_name], dict):
            #     txt += params
            else:
                txt += '// - ' + param_dict[CKeys.short] + ' : ' + utils.param_value_to_txt(param_value)
            txt += '\n'
        return txt

    def write_c_file_header(self):
        # if self.cfilename == "":
        self.compute_c_file_names()

        cfilename = os.path.join(self.full_outdir, self.cfilename)
        self.log.log("%s writing file %s" % (self.get_name(), cfilename), colorama.Fore.YELLOW)
        self.c_file_handler = utils.open_file(cfilename)
        self.c_file_handler.write(self.compute_c_file_header())

    def write_c_file_footer(self):
        self.c_file_handler.write('#endif /* %s */\n' % self.param_config_define)
        self.c_file_handler.close()

    def write_c_file_coeff(self, idx=None, coeff=None):
        self.c_file_handler.write(self.convert_vector_to_txt(coeff, idx))

    def write_h_file_header(self):
        # if self.hfilename == "":
        self.compute_c_file_names()
        hfilename = os.path.join(self.full_outdir, self.hfilename)
        self.h_file_handler = utils.open_file(hfilename)
        self.h_file_handler.write('#ifndef %s\n' % self.header_guard)
        self.h_file_handler.write('#define %s\n\n' % self.header_guard)
        self.h_file_handler.write('#ifdef %s\n' % self.param_config_define)

    def write_h_file_coeff(self, idx=None):
        self.h_file_handler.write(
            'extern %s_Coeff_t const %sCoeff' % (self.get_name().upper(), self.get_name().lower()))
        self.h_file_handler.write('%d;' % idx) if idx is not None else None
        self.h_file_handler.write('\n')

    def write_h_file_footer(self):
        self.h_file_handler.write('#endif /* %s */\n' % self.param_config_define)
        self.h_file_handler.write('\n#endif  /* %s */\n' % self.header_guard)
        self.h_file_handler.close()

    def open_txt_files(self):
        self.cIncludeFileHandler = utils.open_file(os.path.join(self.full_outdir, self.get_name() + '_cInclude.c'))
        self.hIncludeFileHandler = utils.open_file(os.path.join(self.full_outdir, self.get_name() + '_hInclude.h'))

    def append_txt_files(self):
        self.cIncludeFileHandler.write('#include "%s/%s"\n' % (self.outdir, self.cfilename))
        self.hIncludeFileHandler.write('#include "%s/%s"\n' % (self.outdir, self.hfilename))

    def close_txt_files(self):
        self.cIncludeFileHandler.close()
        self.hIncludeFileHandler.close()

    def write_output_file(self):
        self.compute_algo()
        self.write_c_file()
        self.write_h_file()
        self.append_txt_files()

    def get_cur_config(self):
        txt = ""
        for p_name in self.benched_params_list:
            txt += " %s:%s " % (self.params[p_name][CKeys.short], str(self.params[p_name][CKeys.value]))
        return txt

    def print_cur_config(self):
        txt = self.get_cur_config()
        self.log.log(txt)

    @staticmethod
    def get_prototype(benched=True,
                      parse_algos_cb=None,
                      parse_algos_cb_params=None,
                      parse_params_cb=None,
                      parse_params_cb_name="",
                      algo_to_algos_list_cb_hook=None):
        proto = {
            CKeys.benched: benched,
            "parse_algos_cb": parse_algos_cb,
            "parse_algos_cb_params": parse_algos_cb_params,
            "parse_params_cb": parse_params_cb,
            "parse_params_cb_name": parse_params_cb_name,
            "algo_to_algos_list_cb_hook": algo_to_algos_list_cb_hook,
        }
        return proto

    def parse_params(self, cb_params=None, p_name_id=0):
        # if p_name_id == 0:
        #     self.log.log("parse_params start " + self.get_name(), colorama.Fore.LIGHTRED_EX)
        if cb_params is None:
            self.print_cur_config()
        else:
            if cb_params[CKeys.benched]:
                params_nb = len(self.benched_params_list)
                params_list = self.benched_params_list
            else:
                params_nb = len(self.params_list)
                params_list = self.params_list
            if p_name_id == params_nb:
                # self.log.log("parse_params end " + self.get_name(), colorama.Fore.LIGHTRED_EX)
                # all parameters have been browsed => call parse_params_cb and algo_to_algos_list_cb_hook
                parse_params_cb = cb_params["parse_params_cb"]
                if parse_params_cb:
                    parse_params_cb()
                algo_to_algos_list_cb_hook = cb_params["algo_to_algos_list_cb_hook"]
                if algo_to_algos_list_cb_hook is not None:
                    algo_to_algos_list_cb_hook(cb_params, self)
            else:
                # get parameter name
                p_name = params_list[p_name_id]
                # browse parameter values
                for p_val in self.get_param_val_list(p_name):
                    self.set_param_val(p_name, p_val)
                    # browse next parameter
                    # ################ DEBUG ###################
                    # txt = "parse_params (%d):" % (p_name_id + 1)
                    # for idx in range(p_name_id + 1):
                    #     p_name = params_list[idx]
                    #     txt += " %s:%s" % (self.params[p_name][CKeys.short], str(self.params[p_name][CKeys.value]))
                    # self.log.log(txt, colorama.Fore.LIGHTBLUE_EX)
                    # ################ DEBUG ###################
                    self.parse_params(cb_params, p_name_id + 1)

    @staticmethod
    def load(filename):
        return load(filename)

    def dump_npy_data(self, data):
        # save the test_results data structures
        file_without_extension = os.path.join(self.full_outdir, self.filename + "_results")
        # self.log.log(file_without_extension, colorama.Fore.YELLOW)
        save(file_without_extension+".npy", data)
        savetxt(file_without_extension+".txt", data)

        csv_file = file_without_extension+".csv"
        data_str = str(data).replace('[ ', '').replace(' ]', '').replace('[', '').replace(']', '')
        while '  ' in data_str:
            data_str = data_str.replace('  ', ' ')

        with open(csv_file, 'w') as f:
            f.write(data_str)

    def dump(self, data=None, data_name="params"):
        if data is None:
            data = self.params
        if self.outdir is not None:
            # save the test_results data structures
            file_without_extension = self.outdir + "/" + data_name
            utils.dump(data, file_without_extension)

    def polar_plot(self, data, offscreen=False, newFigure=True):
        self.angles = np.linspace(0, 2 * np.pi, 360)
        if offscreen:
            plt.ioff()  # don't show the QT window
        if newFigure or self.fig is None:
            self.fig, self.ax = plt.subplots(subplot_kw={'projection': 'polar'})

        self.ax.plot(self.angles, data)
        self.ax.set_rmax(np.max(data))
        # ax.set_rticks([0.5, 1, 1.5, 2])  # Less radial ticks
        self.ax.set_rlabel_position(-22.5)  # Move radial labels away from plotted line
        self.ax.grid(True)
        self.ax.set_title("Beam pattern", va='bottom')

    def polar_plot_all(self):
        assert False, "WARNING: empty function polar_plot_all, this method must be overwritten by algo itself!"

    def polar_plot_dynamic(self, data):
        assert False, "WARNING: empty function polar_plot_dynamic, this method must be overwritten by algo itself!"

    def generate_c_code(self):
        utils.printc("WARNING: empty function generate_c_code," +
                     " this method should be overwritten by algo itself if needed!")

    def plot_show(self):
        plt.show()

    def params_val_to_txt(self):
        txt = ""
        for p_name in self.benched_params_list:
            val = self.get_param_val(p_name, to_str=True)
            txt += p_name + "= " + val + ", "
        return txt

    def print_params_val(self):
        self.log.log(self.params_val_to_txt(), colorama.Fore.LIGHTCYAN_EX)

    def print_conf(self):
        txt = "algo %d: %s, " % (self.get_id(), self.get_name())
        txt += "conf %d: %s" % (self.get_conf_id(), self.param_config_define)
        self.log.log(txt, colorama.Fore.CYAN)
        return txt

    # AudioChain generic stuffs
    def generate_c_config(self):
        txt = " " * 2 + "AudioAlgo_config(&%s\n" % self.get_name_id()
        nb_space_indent = 19
        txt += " " * nb_space_indent + "%d,\n" % self.get_id()
        txt += " " * nb_space_indent + "1U,\n"
        txt += " " * nb_space_indent + "\"%s\",\n" % self.get_name()
        txt += " " * nb_space_indent + "&%s,\n" % (self.get_name_id() + "Conf")
        txt += " " * nb_space_indent + "NULL,\n"
        txt += " " * nb_space_indent + "&AudioChainWrp_%sCbs,\n" % self.get_name()
        txt += " " * nb_space_indent + "%s,\n" % "DISPLAY_CPU_LOAD_TIMEOUT_MS"
        txt += " " * nb_space_indent + "%s);\n" % "DISPLAY_CPU_LOAD_CB"
        return txt

    def add_input_chunk(self, chunk):
        if chunk not in self.in_chunk_list:
            self.in_chunk_list.append(chunk)

    def add_output_chunk(self, chunk):
        if chunk not in self.out_chunk_list:
            self.out_chunk_list.append(chunk)

    def get_out_chunks(self, nb_elements_per_frame, nb_frames, fs, nb_ch):
        if self.nb_out_chunk > 0:
            for i in range(self.nb_out_chunk):
                chunk_id = str(i) if self.nb_out_chunk > 1 else ""
                name = "OutChunk" + chunk_id + self.get_name_id()
                self.out_chunk_list.append(self.init_out_chunk(name, nb_elements_per_frame, nb_frames, fs, nb_ch))
        return self.out_chunk_list

    def init_out_chunk(self, name, nb_elements_per_frame, nb_frames, fs, nb_ch):
        return AudioChunk(name=name,
                          type="AUDIO_CHUNK_TYPE_USER_INOUT",
                          nb_elements_per_frame=nb_elements_per_frame,
                          nb_frames=nb_frames,
                          fs=fs,
                          nb_channels=nb_ch,
                          data_format=self.out_samples["data_format"],
                          interleaved=self.out_samples["interleaved"],
                          domain=self.out_samples["domain"],
                          memory_pool="AUDIO_MEM_RAMINT",
                          log_file_handle=self.log_file_handle)
