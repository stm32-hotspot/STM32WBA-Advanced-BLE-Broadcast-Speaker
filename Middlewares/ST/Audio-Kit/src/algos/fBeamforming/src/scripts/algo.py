import numpy as np
from numpy import linalg
import os
import colorama
from scripts.core.keys import CKeys as CKeys
from scripts.core.algo import Algo as Algo
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider
import scripts.core.utils as utils

root_dir = os.path.normpath(os.path.join(os.path.dirname(__file__), '..'))


class fBeamforming(Algo):
    """This class implements frequency beamformer algo config methods"""

    gamma = 1.4                                 # indice adiabatique air: https://fr.wikipedia.org/wiki/Indice_adiabatique
    r = 287.058                                 # constante gaz parfaits(air): https://fr.wikipedia.org/wiki/Constante_universelle_des_gaz_parfaits
    temp = 20.0                                 # température °C
    c = np.sqrt(gamma * r * (273.15 + temp))    # https://fr.wikipedia.org/wiki/Vitesse_du_son

    type = "Name of beamforming method"
    antenna_id = "index of the antenna; works only with two"
    mic_nb = "Number of microphones"
    mic_dist = "Microphones distance"
    bands_nb = "Number of bands"
    sample_freq = "Sampling frequency"
    noise_std = "Standard deviation of noise sources"
    speech_sources = "Angles of the source to point at"
    interference_sources = "Angles of the interferences to be avoided"
    output_folder = "coeff_files"
    unitDb = "unit decibels or linear"

    """This class implements a frequency fBeamforming ready to be exported in c file format  """
    version = 1.0
    parameters = {
                  type: {CKeys.short: "type", CKeys.order_id: 0, CKeys.benched: False},
                  mic_nb: {CKeys.short: "micsNb", CKeys.order_id: 2, CKeys.benched: False},
                  mic_dist: {CKeys.short: "micDist", CKeys.order_id: 3, CKeys.benched: False},
                  bands_nb: {CKeys.short: "bandsNb", CKeys.order_id: 4, CKeys.benched: False},
                  sample_freq: {CKeys.short: "freqHz", CKeys.order_id: 1, CKeys.benched: False},
                  noise_std: {CKeys.short: "noiseStdDev", CKeys.order_id: 7, CKeys.benched: False},
                  speech_sources: {CKeys.short: "source", CKeys.order_id: 5, CKeys.benched: False},
                  interference_sources: {CKeys.short: "noise", CKeys.order_id: 6, CKeys.benched: False},
                  unitDb: {CKeys.short: "unit", CKeys.order_id: 9, CKeys.benched: False},
                  antenna_id: {CKeys.short: "antenna_id", CKeys.order_id: 8, CKeys.benched: False},
    }

    # set default values to allow creation from no params
    parameters[type][CKeys.value] = "SIR"
    parameters[unitDb][CKeys.value] = True
    parameters[sample_freq][CKeys.value] = 16000
    # value from the spec =
    # parameters[mic_nb][CKeys.value] = 4
    # parameters[bands_nb][CKeys.value] = 129
    # parameters[mic_dist][CKeys.value] = c / parameters[sample_freq][CKeys.value] * 1000
    # parameters[noise_std][CKeys.value] = 0.01
    # parameters[speech_sources][CKeys.value] = [25]
    # parameters[interference_sources][CKeys.value] = [0,-45]

    parameters[mic_nb][CKeys.value] = 2
    parameters[bands_nb][CKeys.value] = 129
    parameters[mic_dist][CKeys.value] = 21
    parameters[noise_std][CKeys.value] = 0.0001
    parameters[speech_sources][CKeys.value] = [0]
    parameters[interference_sources][CKeys.value] = [45]

    parameters[antenna_id][CKeys.value] = 0

    parameters[sample_freq][CKeys.list] = [16000]
    parameters[mic_nb][CKeys.list] = [2]
    parameters[bands_nb][CKeys.list] = [129]

    # parameters[mic_dist][CKeys.list] = [36]
    # parameters[noise_std][CKeys.list] = [0.000001]
    # parameters[speech_sources][CKeys.list] = [[0]]
    # parameters[interference_sources][CKeys.list] = [[20]]

    # parameters[mic_dist][CKeys.list] = [72]
    # # parameters[type][CKeys.list] = ["SIR", "MVDR"]
    # parameters[noise_std][CKeys.list] = [0.001]
    # parameters[speech_sources][CKeys.list] = [[0]]
    # parameters[interference_sources][CKeys.list] = [[45, 70]]  # [30], [40], [50], [60], [70], [80], [90]]

    # old values found that would beat competition
    parameters[mic_dist][CKeys.list] = [63] #[21, 42, 63]
    # parameters[type][CKeys.list] = ["SIR", "MVDR"]
    parameters[noise_std][CKeys.list] = [0.00000000001]
    parameters[speech_sources][CKeys.list] = [[0]]
    parameters[interference_sources][CKeys.list] = [ [45]]#, [20]]  # [30], [40], [50], [60], [70], [80], [90]]
    parameters[antenna_id][CKeys.list] = [0, 1]

    parameters[mic_dist][CKeys.benched] = True
    parameters[bands_nb][CKeys.benched] = True
    parameters[noise_std][CKeys.benched] = True
    parameters[speech_sources][CKeys.benched] = True
    parameters[interference_sources][CKeys.benched] = True
    parameters[antenna_id][CKeys.benched] = False

    def __init__(self, name, params=None, verbose=False, log_file_handle=None):
        super().__init__(verbose=verbose, log_file_handle=log_file_handle)
        self.set_name(name)
        self.load_internal_params(fBeamforming.parameters)
        self.load_params(params)
        # computed parameters
        self.set_outdir(os.path.join(root_dir, fBeamforming.output_folder), clear=True)
        # init for plots of beam pattern
        self.s_factor = None
        self.version = fBeamforming.version
        # other inits

    def init_weights(self):
        mic_nb = self.get_param_val(fBeamforming.mic_nb)
        bands_nb = self.get_param_val(fBeamforming.bands_nb)
        sample_freq = self.get_param_val(fBeamforming.sample_freq)
        fft_len = (bands_nb - 1)*2
        overlap_buff_nb = 2
        self.freq_bands = np.linspace(0.0, sample_freq / 2.0, bands_nb)
        self.weights = np.zeros((bands_nb, mic_nb), dtype=np.complex128)
        duration = fft_len / (sample_freq/1000) / overlap_buff_nb
        self.build_cmd_option = "AC_SYSIN_MIC_MS=%d" % duration
        if self.get_param_val(fBeamforming.type) == "DMA":
            self.__remove_element(fBeamforming.noise_std)
            self.__remove_element(fBeamforming.speech_sources)
            self.__remove_element(fBeamforming.interference_sources)
            self.__remove_element(fBeamforming.antenna_id)
            # self.benched_params_list.remove(fBeamforming.speech_sources)
            # self.benched_params_list.remove(fBeamforming.interference_sources)
            # self.benched_params_list.remove(fBeamforming.antenna_id)
            # del self.params[fBeamforming.noise_std]
            # del self.params[fBeamforming.speech_sources]
            # del self.params[fBeamforming.interference_sources]
            # del self.params[fBeamforming.antenna_id]

    def get_theta_sources(self):
        return self.get_param_val(fBeamforming.speech_sources) + self.get_param_val(fBeamforming.interference_sources)

    def set_param_val(self, p_name, value):
        self.params[p_name][CKeys.value] = value

    def __remove_element(self, element_name):
        if element_name in self.benched_params_list:
            self.benched_params_list.remove(element_name)

        if element_name in self.params_list:
            self.params_list.remove(element_name)
        if element_name in self.params.keys():
            del self.params[element_name]

    def __dma_compute_weights_per_band(self):
        mic_nb = self.get_param_val(fBeamforming.mic_nb)
        mic_dist = self.get_param_val(fBeamforming.mic_dist)

        if self.get_param_val(fBeamforming.sample_freq) != 16000 and mic_nb != 2:
            self.log.log("ERROR DMA BF is implemented for dual microphones @ 16kHz only", colorama.Fore.RED)

        Ta = (mic_dist / 2 /1000) / fBeamforming.c  # Acoustic delay between sensors
        T = Ta                        # Delay to be applied(Cardioid: Ta = T)
        # T = Ta/3                      # Delay to be applied(T/TA =1/3 for hyper cardoid)
        # T = 0                      # Delay to be applied(T = 0 for dipole)

        omega = self.freq_bands * 2 * np.pi + 1e-10 # epsilon to avoid warning div0
        omegaC = (np.pi / (Ta + T))

        # antifilter: saturated at low frequencies for avoiding adding infinite
        # (or very high) gain
        Weq = (1 / np.sin(np.pi / 2 * omega / omegaC))
        idx = np.where(omega > omegaC)

        if len(idx[0]):
            Weq[idx[0][0]:] = 1
        Weq[0:4] = Weq[4]

        # 0.25 due to (the average + the filter is applied twice)
        Weq_compensated = Weq/4

        # Half the delay applied to each mic (first minus is for subtraction, -T is for negative delay)
        FilterM2 = -np.exp( 1j * 2 * np.pi * self.freq_bands * -T) * Weq_compensated    # minus due to subtraction + Delay + equalization + 0.25 due to (the average + the filter is applied twice)
        FilterM1 =  np.exp( 1j * 2 * np.pi * self.freq_bands *  T) * Weq_compensated    # Equalization +
        # in order to match matlab script ; need to conjugate
        # FilterM1 = FilterM1.conjugate()
        # FilterM2 = FilterM2.conjugate()
        self.weights[:, 0] = FilterM1
        self.weights[:, 1] = FilterM2

    def __sir_compute_weights_per_band(self, band_id):
        n_sources = len(self.get_theta_sources())
        fs = self.freq_bands[band_id]
        mic_nb = self.get_param_val(fBeamforming.mic_nb)
        mic_dist = self.get_param_val(fBeamforming.mic_dist)
        noise_std = self.get_param_val(fBeamforming.noise_std)

        # mic_nb = 4
        # n_sources = 3
        # theta_sources = [25, 0, -45]
        # c = 343
        # fs = 16000
        # mic_dist = c / fs / 2
        # noise_std = 0.01

        a = np.zeros((mic_nb, n_sources), dtype=np.complex128)
        for mic_id in range(mic_nb):
            theta_sources_id = 0
            for theta_source in self.get_theta_sources():
                t = mic_dist / 1000.0 / fBeamforming.c * mic_id * np.sin(np.radians(theta_source))
                a[mic_id, theta_sources_id] = np.exp(1j * 2.0 * np.pi * fs * t)
                theta_sources_id += 1
        # reshape -1 automatically detects the size, the 1: is to force the shape to be N by 1
        # (doesn't work without it as it creates single element somehow; todo: toBeInvestigated)
        a0 = a[:, 0].reshape(-1, 1)
        a0_t = a0.conj().transpose()
        rdd = a0 @ a0_t
        a12 = a[:, 1:n_sources]
        a12_t = a12.conj().transpose()
        rnn = np.identity(mic_nb, dtype=np.complex128) * noise_std
        rii = a12 @ a12_t + rnn

        invrii = linalg.inv(rii)
        invriirdd = invrii @ rdd
        # Other technique using the left divider
        # invriirdd = linalg.lstsq(rii, rdd)[0]
        e, v = linalg.eig(invriirdd)
        k = np.nonzero(e == e.max(0))  # np.find(e == np.max(e))
        w = v[:, k].reshape(-1, 1)
        # w = np.flipud(w)
        check_w_normalized = abs(sum(w * w.conj()))
        if abs(check_w_normalized - 1.0) > 1e-9:
            self.log.log("error: bf weights not normalized; take a closer look !!!", colorama.Fore.RED)
        self.weights[band_id, :] = w.T

    def compute_algo(self):
        self.init_weights()
        if self.get_param_val(fBeamforming.type) == "SIR":
            for band_id in range(self.get_param_val(fBeamforming.bands_nb)):
                self.__sir_compute_weights_per_band(band_id)
        elif self.get_param_val(fBeamforming.type) == "DMA":
            self.__dma_compute_weights_per_band()
        else:
            self.log.log("ERROR ONLY SIR & DMA BF are implemented", colorama.Fore.RED)
            quit()

    def get_polar_weights(self, freq, id=None):
        nb_bands = self.get_param_val(fBeamforming.bands_nb)
        freq_per_band = self.get_param_val(fBeamforming.sample_freq) / 2.0 / nb_bands
        freq_idx = 0
        if id is not None:
            freq_idx = id
        else:
            freq_idx = int(np.floor(freq / freq_per_band)) - 1
        weights_direction = self.weights[freq_idx, :]
        n_angles = 360
        k = 2.0 * np.pi * freq / fBeamforming.c
        coeffs_polar_plot = np.zeros((n_angles, 1), dtype=np.complex128)
        mic_dist = self.get_param_val(fBeamforming.mic_dist)/1000
        mic_nb = self.get_param_val(fBeamforming.mic_nb)

        # dephasage = np.radians(90)
        for a in range(n_angles):
            phy = k * mic_dist * np.sin(np.radians(a)) # + dephasage;
            for id_mic in range(mic_nb):
                coeffs_polar_plot[a] = coeffs_polar_plot[a] + weights_direction[id_mic] * np.exp(1j * phy * (id_mic - 1) );
        coeffs_polar_plot = abs(coeffs_polar_plot)
        if self.get_param_val(fBeamforming.unitDb):
            coeffs_polar_plot = 20.0 * np.log10(coeffs_polar_plot)
        return coeffs_polar_plot

    def write_output_file(self):
        super().write_output_file()
        self.dump_npy_data(self.weights)

    def write_c_file(self):
        self.write_c_file_header()
        for mic_id in range(self.get_param_val(fBeamforming.mic_nb)):
            self.write_c_file_coeff(mic_id, self.weights[:, mic_id].reshape(-1, 1))
        self.write_c_file_footer()


    def write_json(self,json):
        for mic_id in range(self.get_param_val(fBeamforming.mic_nb)):
            vector =  self.weights[:, mic_id].reshape(-1, 1)
            nb_rows = vector.size
            json[mic_id] = {}
            for row in range(nb_rows):
                json[mic_id][row] = []
                json[mic_id][row].append(vector[row].real[0])
                json[mic_id][row].append(vector[row].imag[0])


    def write_h_file(self):
        self.write_h_file_header()
        for param in self.params.keys():
            if param != fBeamforming.antenna_id:
                continue
            param_val = self.get_param_val(param, to_str=True)
            self.h_file_handler.write('  #define %s_%s %s\n' % (self.get_name().upper(), self.get_param_short(param, upper_case=True), param_val))
        self.write_h_file_footer()
        # self.write_h_file_header()
        # for mic_id in range(self.get_param_val(fBeamforming.mic_nb)):
        #     self.write_h_file_coeff(mic_id)
        # self.write_h_file_footer()

    def open_txt_files(self):
        self.hIncludeFileHandler = utils.open_file(os.path.join(self.full_outdir, self.get_name() + '_hInclude.h'))
        self.hIncludeFileHandler.write('  #include "fBeamforming/fBeamforming_coef.h"\n')

        self.hIncludeFileHandler.write('extern BEAMFORMER_Coeff_t const beamformerCoeff0;\n')
        self.hIncludeFileHandler.write('extern BEAMFORMER_Coeff_t const beamformerCoeff1;\n')
        self.hIncludeFileHandler.write('#ifdef %s\n' % CKeys.MakefileSimu)

        self.cIncludeFileHandler = utils.open_file(os.path.join(self.full_outdir, self.get_name() + '_cInclude.c'))

    def append_txt_files(self):
        self.hIncludeFileHandler.write('  #include "fBeamforming/src/%s/%s"\n' % (self.outdir, self.hfilename))
        self.cIncludeFileHandler.write('  #include "fBeamforming/src/%s/%s"\n' % (self.outdir, self.cfilename))

    def close_txt_files(self):
        self.hIncludeFileHandler.write('#else // %s\n' % CKeys.MakefileSimu)
        self.hIncludeFileHandler.write('  #define BEAMFORMER_ANTENNA_ID 0U\n')
        self.hIncludeFileHandler.write('#endif // %s\n' % CKeys.MakefileSimu)
        self.hIncludeFileHandler.close()
        self.cIncludeFileHandler.close()

    def generate_c_code(self):
        self.param_config_str_list = []
        self.make_outdir()
        self.open_txt_files()
        proto = self.get_prototype(parse_params_cb=self.write_output_file)
        self.parse_params(proto)
        self.close_txt_files()

    def polar_plot_all(self,offScreen = False):
        self.polar_plot(self.get_polar_weights(0),offScreen)
        freq_per_band = self.get_param_val(fBeamforming.sample_freq) / 2.0 / self.get_param_val(fBeamforming.bands_nb)
        for i in range(1, self.get_param_val(fBeamforming.bands_nb) - 1):
            freq = i * freq_per_band
            data = self.get_polar_weights(freq)
            self.ax.plot(self.angles, data)


    def polar_plot_dynamic(self, data, newFigure=True):
        self.polar_plot(data, newFigure=newFigure)
        self.add_slider(self.get_param_val(fBeamforming.sample_freq), self.get_param_val(fBeamforming.bands_nb))

    def polar_plot_val(self,current_freq,offScreen=False, newFigure=True):
        self.polar_plot(self.get_polar_weights(0), offScreen, newFigure=newFigure)
        try:
            data = self.get_polar_weights(current_freq)
            self.ax.clear()
            self.ax.plot(self.angles, data)
        except:
            print("An exception occurred during the polarplot")


    def polar_plot_update(self, val):
        current_freq = self.s_factor.val
        data = self.get_polar_weights(current_freq)
        self.ax.clear()
        self.ax.plot(self.angles, data)

    def add_slider(self, sampling_freq, bands_nb):
        plt.subplots_adjust(bottom=0.25)
        ax_slide = plt.axes([0.25, 0.1, 0.65, 0.03])
        self.s_factor = Slider(ax=ax_slide,
                               label="Frequency Hz",
                               valmin=0,
                               valmax=sampling_freq/2,
                               valinit=1000,
                               valstep=sampling_freq/2/bands_nb)

    def plot_show(self):
        if self.s_factor is not None:
            self.s_factor.on_changed(self.polar_plot_update)
        plt.show()
