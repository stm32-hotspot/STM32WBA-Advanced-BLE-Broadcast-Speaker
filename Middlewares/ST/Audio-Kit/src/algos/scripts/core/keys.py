class CKeys:
    """This class implements keys for algo parameters access"""
    PythonRoot = "Middlewares/ST/Audio-Kit/src/algos"
    AfeProvider = "Audio processing algorithm provider"
    AsrProvider = "Speech recognition algorithm provider"
    AfeVersion = "Audio processing algorithm version"
    AsrVersion = "Speech recognition algorithm version"
    Filename = "Filename"
    AlgoName = "algo name"
    AlgoNameList = "List of algorithm names"
    WaveAlgoName = "Waves"
    WavesList = "List of waves matching current parameters"
    sourceFile = "source file"
    afeSysOutIdx = "AFE system out index"
    TestsVersion = "Tests version"
    LoopId = "Loop index"
    QuestionList = "List of questions"
    QuestionId = "question id"
    Question = "question"
    NbQuestions = "nb questions"
    SpeechLoc = "speech LOC"
    NoiseLoc = "noise LOC"
    SpeechLevel = "speech level"
    SpeechMode = "speech mode"
    NoiseLevel = "noise level"
    NoiseName = "noise name"
    DifficultyLevel = "Description of the test difficulty"
    NbWakeWordOk = "nb wake word ok"
    NbWakeWordFail = "nb wake word fail"
    NbCmdFail = "nb command fail"
    NbCmdOk = "nb command ok"
    # computed results
    FRRpercent = "FRR percent"
    RARpercent = "RAR percent"
    FRRstatus = "FRR status"
    RARstatus = "RAR status"
    no_noise_name = "no_noise"
    intent = "intent"
    ww = "wake word"
    metadata = "metadata"  # "All data that are describing the end results, i.e noise and speech levels, names, etc... "
    data = "data"
    general_infos = "General information"
    doc = "Documentation"
    value = "Value"
    is_ready = "tag to check if param has been set while calling recursive algo"
    str_param_in_filename = "String used in file name"
    name = "Name of algorithm"
    short = "Short description"
    benched = "bool to specify if parameter is benched"
    benched_params = "Sorted list of parameters in session data; i.e dictionary keys"
    order_id = "Index of parameter in the end filename"
    list = "list of values to be tested"
    CommandFailList = "command fail list"
    CommandOkList = "command ok list"
    average = "average"  # for results and targets
    per_noise = "per noise"
    requirements = "requirements"
    AlgosDir = "../.."
    MakefileDirFromAlgosDir = "validation/src"
    MakefileDir = AlgosDir + "/" + MakefileDirFromAlgosDir
    MakefileSimu = "SIMULATION_X86"

    algo_name             = "algo_name"
    audio_chain_instance  = "audio_chain_instance"
    sys_in_ch_nb_list     = "sys_in_ch_nb_list"
    sys_out_ch_nb_list    = "sys_out_ch_nb_list"
    audio_in_frequency    = "audio_in_frequency"
    audio_out_frequency   = "audio_out_frequency"
    max_rms_error_dB      = "max_rms_error_dB"
    build_mode            = "build_mode"
    waves_in_dict         = "waves_in_dict"
    waves_out_dict        = "waves_out_dict"
    waves_ref_dict        = "waves_ref_dict"
    waves_ref_dict        = "waves_ref_dict"
    build_options         = "build_options"
    txt_out_dict          = "txt_out_dict"
    txt_ref_dict          = "txt_ref_dict"
    waves_indir           = "waves_indir"
    waves_refdir          = "waves_refdir"
    txt_refdir            = "txt_refdir"
    isOutputingWav        = "isOutputingWav"
    isOutputingTxt        = "isOutputingTxt"

    # WARNING: NbSysInMax/NbSysOutMax must comply with ac_sys_in_id_t/ac_sys_out_id_t enums values AC_NB_MAX_SYS_IN/AC_NB_MAX_SYS_OUT respectively (see audio_chain_sysIOs_conf.h file)
    NbSysInMax = 4
    NbSysOutMax = 3

    # WARNING: c_defines_for_sysin_channels_nb/c_define_for_sysout_channels_nb order must comply with C defines
    c_defines_for_sysin_channels_nb = ["AC_SYSIN_MIC_CHANNELS_NB",
                                       "AC_SYSIN_USB_CHANNELS_NB",
                                       "AC_SYSIN_ETH_CHANNELS_NB",
                                       "AC_SYSIN_PDM_CHANNELS_NB"]
    c_defines_for_sysout_channels_nb = ["AC_SYSOUT_SPK_CHANNELS_NB",
                                        "AC_SYSOUT_USB_CHANNELS_NB",
                                        "AC_SYSOUT_ETH_CHANNELS_NB"]

    # WARNING: all keys MUST be included in this list for update_keys() operation (see below)
    keys_list = [
        AfeProvider,
        AsrProvider,
        AfeVersion,
        AsrVersion,
        AlgoName,
        AlgoNameList,
        WaveAlgoName,
        WavesList,
        Filename,
        sourceFile,
        afeSysOutIdx,
        TestsVersion,
        LoopId,
        QuestionList,
        QuestionId,
        Question,
        NbQuestions,
        SpeechLoc,
        NoiseLoc,
        SpeechLevel,
        SpeechMode,
        NoiseLevel,
        NoiseName,
        DifficultyLevel,
        NbWakeWordOk,
        NbWakeWordFail,
        NbCmdFail,
        NbCmdOk,
        FRRpercent,
        RARpercent,
        FRRstatus,
        RARstatus,
        no_noise_name,
        intent,
        ww,
        metadata,
        data,
        general_infos,
        doc,
        value,
        is_ready,
        str_param_in_filename,
        name,
        short,
        benched,
        benched_params,
        order_id,
        list,
        CommandFailList,
        CommandOkList,
        average,
        per_noise,
        requirements,
    ]

    # check that all keys are strings, that no key is duplicated and that 2 different keys have different strings
    for idx1 in range(len(keys_list)):
        key1 = keys_list[idx1]
        assert (type(key1) is str), "key %d: '%s' is not a string" % (idx1, str(key1))
        for idx2 in range(len(keys_list)):
            if idx2 != idx1:
                key2 = keys_list[idx2]
                assert(key1 != key2), "key %d is equal to key %d: '%s'" % (idx1, idx2, key1)

    @staticmethod
    def update_keys(data):
        # convert keys from python file (strings) into keys from keys.py
        # so that checks such as 'if key in dict.keys()' or 'if key is CKeys.xxx' work fine
        if type(data) is list:
            data2 = []
            for val in data:
                data2.append(CKeys.update_keys(val))
        elif type(data) is dict:
            data2 = {}
            for key, val in data.items():
                assert (key in CKeys.keys_list), "key '%s' not in keys_list" % str(key)
                idx = CKeys.keys_list.index(key)
                key2 = CKeys.keys_list[idx]
                data2[key2] = CKeys.update_keys(val)
        else:
            data2 = data
        return data2
