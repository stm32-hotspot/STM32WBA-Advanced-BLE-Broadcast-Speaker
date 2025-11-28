# X-cube_audio_kit_running_on_BLE_Speaker

This example shows the usage of Audio-kit on the BLE Audio board, integrating a STM32H573 for extending audio processing capabilities. 
It is based on the Livetune application for STM32H573I-DK provided in x-cube-audio-kit-1.3 package and is provided for EWARM 9.40 and STM32CubeIDE.

This firmware relies on a external coprocessor running the main application and sending I2C commands for changing states. 

The internal flash is used for storing audio graph and keywords. USB communication is used only for the CDC class (VCP) class.  

Audio IN/OUT has been adapted to the board schematics:
* SAI2 block B as audio IN from the STM32WBA
* SAI1 block A as audio IN from the MEMS microphones
* SAI2 block A as audio OUT to the STM32WBA
* SPI1 as audio OUT to the codec (DAC)

The tuner mode as been removed from the project, only Designer and Release mode are supported.

## Main_changes
* Update UART
    * Mapping of GPIO (PC10 and PC11)
    * Move USE_UART define to project level (Livetune only), allowing removal of trace manager
* Use internal flash as non-volatile memory
    * Create 4 reserved 16k regions of flash starting at address 0x081F0000 : 1 region for registry and 3 regions for JSON
    * Disable cache when verifying the flash 
* Remove calls to BSP, button, screen, LEDs...
* Remove USB audio class
* Update SYSIN/SYSOUT: remove Ethernet and USB, add SAI input / output to WBA, Wavefile Input and update PDM microphone input and SPI to codec (DAC) output.
* add files:
    * **voice_announce.c**: sample of announcement (keywords) in 16kHz, 16bits/s mono: *"advertising"*, *"connected"*, *"disconnected"*, *"standby"*, *"low battery"*, *"synchronized"*, *"scanning"* or the demonstration sentence. 
    * **speaker_bsp_audio.c**: bsp for initializing audio input and output: 
    * **speaker_bsp_com.c**: BSP for communication by I2C with the STM32WBA and audio master clock generation 
    * **wba_link.c**: used for handling the local application and commands coming from STM32WBA
    * **plugin_com_usb.c**: used for USB initialization
    * **tusb_cdc_vpc0.c**: USB CDC VCP class

Several defines are introduced at project level:
* USE_BLE_SPEAKER: define the hardware used
* ST_NO_FLASHEXT: remove calls/drivers linked to external flash
* USE_UART: move to project level, allowing to enable trace module (designer mode only)
* USE_UART_OVER_USB: use the USB CDC class for UART communication instead of CN6 UART (designer mode only)
* LOWPWR: introduce low power with sleep mode when audio is on and standby otherwise (release mode only)
* configUSE_IDLE_HOOK set to 1 for allowing to access the background task and add sleep mode (release mode only)
>* USE_PDM2PCM_LIB must be added if PDM2PCM library is required, additionally, the library has to be added to the project as explained in the [top level README](../../../../README.md#some-middleware-libraries-are-unavailable-in-this-repository)

Note that both TUD_AUDIO_OUT_USED and TUD_AUDIO_IN_USED define has been removed since USB audio is not used.

The firmware can be reset using an I2C command or a toggle on PC13 pin.

### Development mode (Default configuration)

This is the mode used for tuning the current audio flow with Livetune. The generated binary does not contain any graph, they must be loaded from the Livetune Interface or directly to the memory at the [reserved addresses](../../../../README.md#audio_graph_management).

### Release mode 

This mode is used for measuring power consumption of the product;
* Sleep mode is used if a stream is running (PLL ON)
* Otherwise the STM32H5 is entering deep sleep, waiting a wakeup signal on PC13

Livetune is removed, only AudioChain is running and audio flows are discribed in audio_chain_generated_code.c