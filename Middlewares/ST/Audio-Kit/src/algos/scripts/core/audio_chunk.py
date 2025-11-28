import scripts.core.utils as utils
from scripts.core.audio_chain_const import AudioChainStr as Const


class AudioChunk:
    """This class implements an AudioChunk initialization C code generation"""

    name = "Name"
    type = "Type"
    duration = "Duration of frame in ms"
    nb_frames = "Number of frames"
    fs = "Sampling frequency"
    nb_channels = "Number of Channels"
    data_format = "fix 16,24,32 or float"
    interleaved = "bool to specify if interleaved"
    domain = "Frequency vs time domain"
    memory_pool = "chosen memory pool"

    def __init__(self, name, type, nb_elements_per_frame, nb_frames, fs, nb_channels, data_format, interleaved, domain, memory_pool,
                 verbose=False, log_file_handle=None):
        self.name = name
        self.type = type
        self.nb_elements_per_frame = nb_elements_per_frame
        self.nb_frames = nb_frames
        self.fs = fs
        self.nb_channels = nb_channels
        self.data_format = data_format
        self.interleaved = interleaved
        self.domain = domain
        self.memory_pool = memory_pool
        self.log_file_handle=log_file_handle
        self.log = utils.Log(verbose=verbose, log_file_handle=log_file_handle)  # permit to call self.log.log(...) without error

        self.c_init_name = "AudioChainInstance_init" + self.name
        self.c_deinit_name = "AudioChainInstance_deinit" + self.name

    def generate_c_init(self):
        interleaved = 1 if self.interleaved else 0
        txt = "static void " + self.c_init_name
        txt += "(void)\n{\n  if (AudioChunk_initAndConfig(&"
        txt += self.name + ",\n"
        nb_space_indent = 22
        txt += " " * nb_space_indent + self.type + ",\n"
        txt += " " * nb_space_indent + str(self.nb_channels) + ",\n"
        txt += " " * nb_space_indent + str(self.fs) + ",\n"
        txt += " " * nb_space_indent + str(self.nb_elements_per_frame) + ",\n"
        txt += " " * nb_space_indent + str(self.nb_frames) + ",\n"
        txt += " " * nb_space_indent + self.domain + ",\n"
        txt += " " * nb_space_indent + self.data_format + ",\n"
        txt += " " * nb_space_indent + str(interleaved) + ",\n"
        txt += " " * nb_space_indent + "\"" + self.name + "\"" + ",\n"
        txt += " " * nb_space_indent + "AudioChain_getUtilsHdle(&AudioChainInstance),\n"
        txt += " " * nb_space_indent + self.memory_pool + ") != AUDIO_ERR_MGNT_NONE)\n"
        txt += " " * 2 + Const.c_error_call
        txt += "\n}\n"
        return txt

    def generate_c_deinit(self):
        txt = "static void " + self.c_deinit_name
        txt += "(void)\n{\n  if (AudioChunk_free(&"
        txt += self.name + ", " + self.memory_pool + ") != AUDIO_ERR_MGNT_NONE)\n"
        txt += " " * 2 + Const.c_error_call
        txt += "\n}\n"
        return txt
