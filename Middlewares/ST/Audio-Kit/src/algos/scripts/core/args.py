import argparse
import colorama
import scripts.core.utils as utils
from scripts.core.keys import CKeys as CKeys
from scripts.core.integrate_const import IntegrateStr as CIntegrate


class Args:
    """This class provides methods to get all mandatory and optionals option that will appear on the command line"""

    def __init__(self, required, optionals, hidden, description='audio chain'):
        self.required = required
        self.optionals = optionals
        self.hidden = hidden
        self.parser = argparse.ArgumentParser(description=description)
        self.args = None
        self.populate_parser()

    def has_default_value(self, option):
        return 'default' in self.optionals[option]

    def get_help_required(self, option):
        return self.required[option]

    def get_help_optional(self, option):
        return self.optionals[option]["help"]

    def get_default_value(self, option):
        return self.optionals[option]["default"]

    def get_action(self, option):
        return self.optionals[option]["action"]

    def populate_parser(self):
        # required arguments
        req_grp = self.parser.add_argument_group('required arguments')
        for option in self.required:
            req_grp.add_argument(option,
                                 required=True,
                                 help=self.get_help_required(option))

        # optionals arguments (with default values)
        for option in self.optionals:
            if self.has_default_value(option):
                self.parser.add_argument(option,
                                         default=self.get_default_value(option),
                                         help=self.get_help_optional(option))
            else:
                self.parser.add_argument(option,
                                         action=self.get_action(option),
                                         help=self.get_help_optional(option))

        for option in self.hidden:
            self.parser.add_argument(option,
                                     default=self.hidden[option]['default'],
                                     help=argparse.SUPPRESS)
        self.args = self.parser.parse_args()
        # if len(sys.argv) == 1:
        #     print("Please enter --help", flush=True)
        #     quit()

    @staticmethod
    def str_2_list(txt, full_list=None, to_int=False):
        out_list = []
        if txt.lower() == "all":
            out_list = full_list
        else:
            for arg in txt.split(','):
                if to_int:
                    arg = int(arg)
                out_list.append(arg)
        return out_list


class ArgsAfe(Args):
    """This class provides methods to get all mandatory and optionals option that will appear on the command line"""
    # default_waves_folder = os.path.join(r'\\gnbcwd0905.gnb.st.com',
    #                                     'Tests_records',
    #                                     'Picovoice_Rhino_Porcupine_v10_Simulator')
    noise_levels = [0, 45, 55, 65]
    speech_levels = [44, 49, 54, 59, 64]
    noise_names = ["no_noise", "music_noise", "pink_noise", "pub_noise"]
    required = {
        # "--report_path": "output directory with report ",
        "--in_waves": "input directory with waves ",
        "--afe_infos": "input file with all infos about the afe to be tested",
        # "--out_waves": "output directory where to put processed waves ",
    }
    optionals = {
        "--report_path": {"default": "AsrBenchResults", "help": "output directory with report"},
        # "--in_waves": {"default": default_waves_folder, "help": "input directory with waves"},
        "--out_waves": {"default": "afeOutput_WaveFiles", "help": "output directory where to put processed waves"},
        "--no_clear": {"action": "store_true", "help": "do not delete output directories if it exists"},
        "--no_make": {"action": "store_true", "help": "do not generate executable"},
        "--no_make_clean": {"action": "store_true", "help": "do not clean executable"},
        "--no_wave_process": {"action": "store_true", "help": "do not process waves"},
        "--use_sox": {"action": "store_true", "help": "use sox for DC removal, resampling and normalization"},
        "--loop": {"default": "all", "help": "index of the loop to bench or all; default: all"},
        "--noise_levels": {"default": "all", "help": "noises  levels list, comma separated without space (within {}) or all; default: all".format(noise_levels)},
        "--speech_levels": {"default": "all",
                            "help": "speech  levels list, comma separated without space (within {}) or all; default: all".format(speech_levels)},
        "--noise_names": {"default": "all",
                          "help": "noises  names, comma separated without space (within {}) or all; default: all".format(noise_names)},

        # "--verbose": {"action": "store_true", "help": "more traces"}
    }
    hidden = {
        # "--inDir": {"default": "todo", "help": argparse.SUPPRESS},
    }

    def __init__(self):
        Args.__init__(self, ArgsAfe.required, ArgsAfe.optionals, ArgsAfe.hidden, "x86_Afe_simulations")
        utils.get_python_file_name(self.args.afe_infos, "simulation infos.py file")
    # def add_noise_params(self, sns):
    #
    #     self.optionals['--noise'] = {"default": None,
    #                                  "help": "noises list, comma separated without space (within {}) or None; default: None"
    #                                  }
    #
    #     self.optionals['--noise_level'] = {"default": None,
    #                                        "help": "noises  levels list, comma separated without space (within {}) or None; default: None"
    #                                        }
    #     self.optionals['--speech_level'] = {"default": None,
    #                                        "help": "noises  levels list, comma separated without space (within {}) or None; default: None"
    #                                        }


class ArgsTerminal(Args):
    """This class provides methods to get all mandatory and optional option that will appear on the command line"""
    baud_rates = [9600, 14400, 19200, 38400, 57600, 115200, 230400, 460800, 921600]
    baud_rate_default = 115200
    required = {}
    optionals = {
        "--com_port": {"default": None,
                       "help": "COM port number (if None, script will try to find it automatically); default: None"},
        "--baud_rate": {"default": baud_rate_default,
                        "help": "COM port baud rate (within %s); default: %d" % (utils.format_list(
                            baud_rates), baud_rate_default)},
        "--out_dir": {"default": "logs",
                      "help": "output directory where logs are stored"}

    }
    hidden = {}

    def __init__(self):
        Args.__init__(self, ArgsTerminal.required, ArgsTerminal.optionals, ArgsTerminal.hidden, "terminal")
        if self.args.baud_rate not in ArgsTerminal.baud_rates:
            print('Error: --baud_rate must be among :' + str(ArgsTerminal.baud_rates))


class ArgsAcValidateRT(Args):
    """This class provides methods to get all mandatory and optional option that will appear on the command line"""
    baud_rates = [9600, 14400, 19200, 38400, 57600, 115200, 230400, 460800, 921600]
    baud_rate_default = 921600
    required = {
        # "--report_path": "output directory with report ",
        "--in_file": "input livetune file",
    }
    optionals = {
        "--com_port": {"default": None,
                       "help": "COM port number (if None, script will try to find it automatically); default: None"},
        "--baud_rate": {"default": baud_rate_default,
                        "help": "COM port baud rate (within %s); default: %d" % (utils.format_list(
                            baud_rates), baud_rate_default)},
        "--out_dir": {"default": "validation_rt", "help": "Name directory that will host all validation data "},
        "--log_filename": {"default": "validation.log", "help": "Name of the file with complete logs"},
        "--report_filename": {"default": "report.txt", "help": "text file report with summarized information"},
        "--verbose": {"action": "store_true", "help": "more traces"}

    }
    hidden = {}

    def __init__(self):
        Args.__init__(self, ArgsAcValidateRT.required, ArgsAcValidateRT.optionals, ArgsAcValidateRT.hidden, "AcValidateRT")
        if self.args.baud_rate not in ArgsAcValidateRT.baud_rates:
            print('Error: --baud_rate must be among :' + str(ArgsAcValidateRT.baud_rates))


class ArgsValidate(Args):
    """This class provides methods to get all mandatory and optionals option that will appear on the command line"""
    algo_list = ["acousticbf", "cic", "delay", "fBeamforming", "filter", "fir_equalizer", "fjoin", "fnlms",
                 "gain", "hpf", "iir_equalizer", "interleave", "linear_mix", "mix", "mono2stereo", "nlms",
                 "normalization", "nr", "pdm2pcm", "resample", "rms", "route", "sam", "sfc", "signal_generator",
                 "speex_nr", "speex_aec", "st_aec", "stereo_widener", "stereo2mono", "switch", "wola"]
    required = {
        # "--report_path": "output directory with report ",
        # "--in_waves": "input directory with waves ",
        # "--out_waves": "output directory where to put processed waves ",
    }
    optionals = {
        # "--out_waves": {"default": "afeOutput_WaveFiles", "help": "output directory where to put processed waves"},
        "--outdir": {"default": CKeys.AlgosDir + "/" + "session", "help": "Name directory that will host all algos validation data "},
        "--algo_name": {"default": "all", "help": "Name of the algorithms to validate: " + str(algo_list)},
        "--no_clear": {"action": "store_true", "help": "do not delete output directories if it exists"},
        "--no_make": {"action": "store_true", "help": "do not generate executable"},
        "--no_make_clean": {"action": "store_true", "help": "do not clean executable"},
        "--no_wave_process": {"action": "store_true", "help": "do not process waves"},
        "--makefile_dir": {"default": CKeys.MakefileDir, "help": "Directory where is the makefile"},
        "--log_filename": {"default": "validation.log", "help": "Name of the file with complete logs"},
        "--report_filename": {"default": "report.txt", "help": "text file report with summarized information"},
        "--verbose": {"action": "store_true", "help": "more traces"}
    }
    hidden = {
        "--algo_list": {"default": "todo", "help": argparse.SUPPRESS},
    }

    def __init__(self):
        Args.__init__(self, ArgsValidate.required, ArgsValidate.optionals, ArgsValidate.hidden, "AcValidate arguments details")
        if self.args.algo_name == "all":
            self.args.algo_list = ArgsValidate.algo_list
        else:
            self.args.algo_list = self.args.algo_name.split(',')

        for algo_name in self.args.algo_list:
            if algo_name not in ArgsValidate.algo_list:
                utils.printc('Error: --algo_name must be among: ' + str(ArgsValidate.algo_list), color=colorama.Fore.RED)
                quit()


class ArgsIntegrate(Args):
    """This class provides methods to get all mandatory and optionals option that will appear on the command line"""
    data_gnt = [None, "all", "untyped", "audio_buffer"]
    required = {
        "--id_card": "files with description of algorithm to integrate inside audioChain ",
        # "--root_dir": "root directory with all files to integrate ",
    }
    full_var_list = list(CIntegrate.C_CTXT_VAR_LIST.keys())

    optionals = {
        "--root_dir": {"default": None, "help": "root directory with all files to integrate"},
        "--no_clear": {"action": "store_true", "help": "do not delete output directories if it exists"},
        "--no_file_copy": {"action": "store_true", "help": "do not copy source file inside outdir"},
        "--data_management": {"default": None, "help": "Generation of code for data management. By default = None, means that the developer as to implement it; \"untyped\" will generate code to manage standard data pointer; \"audio_buffer\" will generate code using the audio_buffer_t structure available in the package. \"all\" will generate both codes"},
        "--var_request": {"default": None, "help": "List of variables to create in context and initialize : can be None, all or a list from following values: " + str(full_var_list)},
        "--outdir": {"default": "integration", "help": "Name directory that will host all algos wrapper sources "},
        "--log_filename": {"default": "validation.log", "help": "Name of the file with complete logs"},
        "--report_filename": {"default": "report.txt", "help": "text file report with summarized information"},
        "--verbose": {"action": "store_true", "help": "more traces"}
    }
    hidden = {
        "--var_list": {"default": "todo", "help": argparse.SUPPRESS},
    }

    def __init__(self):
        Args.__init__(self, ArgsIntegrate.required, ArgsIntegrate.optionals, ArgsIntegrate.hidden, "AcIntegrate arguments details")
        if self.args.var_request == "all":
            self.args.var_list = ArgsIntegrate.full_var_list
        elif self.args.var_request is not None:
            self.args.var_list = self.args.var_request.split(',')
            for var in self.args.var_list:
                if var not in ArgsIntegrate.full_var_list:
                    utils.printc('Error --var_request:  must be among: ' + str(ArgsIntegrate.data_gnt), color=colorama.Fore.RED)
                    quit()
        if self.args.root_dir is not None and not utils.is_dir(self.args.root_dir):
            utils.printc('Error: wrong root_dir option; Please verify the path', color=colorama.Fore.RED)
            quit()

        if self.args.data_management not in ArgsIntegrate.data_gnt:
            utils.printc('Error: --data_management must be among: ' + str(ArgsIntegrate.data_gnt), color=colorama.Fore.RED)
            quit()


class ArgsConvert(Args):
    """This class provides methods to get all mandatory and optionals option that will appear on the command line"""
    required = {
        "--livetune": "input livetune file to be converted into audio chain instance"
    }
    full_var_list = list(CIntegrate.C_CTXT_VAR_LIST.keys())

    optionals = {
        "--no_clear": {"action": "store_true", "help": "do not delete output directories if it exists"},
        "--outdir": {"default": "converted_livetune", "help": "Name directory that will host the generated audio_chain instance  "},
        "--log_filename": {"default": "validation.log", "help": "Name of the file with complete logs"},
        "--report_filename": {"default": "report.txt", "help": "text file report with summarized information"},
        "--verbose": {"action": "store_true", "help": "more traces"}
    }
    hidden = {
    }

    def __init__(self):
        Args.__init__(self, ArgsConvert.required, ArgsConvert.optionals, ArgsConvert.hidden, "AcConvert arguments details")
