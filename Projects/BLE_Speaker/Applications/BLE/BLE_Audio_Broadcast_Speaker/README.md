# __BLE_Audio_Broadcast_Speaker Application Description__

This project implements the **Broadcast Sink** and **Scan Delegator** roles as defined by the Bluetooth® SIG specification on the BLE_Speaker product. A variation allows using the Telephony and Media Audio Profile (TMAP) in **Unicast Server role** (Unicast Media Receiver)

- As a **Broadcast Sink**, the Broadcast Speaker application scans for and synchronizes with Broadcast Sources. It can switch between multiple Broadcast Sources listed in a predefined table.
- As a **Scan Delegator**, the Broadcast Speaker application advertises and accepts connections from remote **Broadcast Assistants**. Once connected, it processes requests from the remote device to synchronize with Broadcast Sources and handles volume control requests.
- Optionally, as **Unicast Server role**, it advertises and accept connections from remote devices, accepts Isochronous Connections from a remote TMAP Central (Unicast Media Sender) and supports Volume Control and Media Control features. This feature is shown in the [release mode](#release-mode). 

See also **BLE_Audio_Broadcast_Speaker** project for development-kit in this repository that can substitute this project. 

### **Keywords**

Connectivity, BLE, BLE protocol, BLE pairing, BLE profile, BLE audio, Auracast

### **Hardware and Software Environment**

This example runs on STM32WBA55CE device placed on the BLE Speaker board (revision B).

To build the project, you need one of the following IDEs:
  - IAR Embedded Workbench for ARM (EWARM) 9.60.3
  - STM32CubeIDE 1.18.1

### **Setup**

Please refer to the top-level [README.md](../../../../../README.md) for BLE Speaker setup. 

### **Operate the demo**

At startup, the application starts the broadcast sink role by searching for the first source of the list defined in tmap_app.c:

```c
const char SourcesToSync[NUM_SOURCES][20] = {"Public_TV_1", "Music_Player_1", "Music_Player_2"};
```

Pushing a button starts the advertising, allowing selection of the source with a broadcast assistant (scan delegator role).


### Main_changes
BLE_Audio_Broadcast_Speaker projects are based on the BLE_Audio_TMAP_Peripheral project of the STM32CubeWBA release v1.7.0. 
Compared to the STM32WBA55G-DK1 version, here is a list of modification:

* Remove BSP sources, components and functions calls related to the STM32WBA55G-DK1 discovery board. 
* Remove menu and calls to the menu
* Add files :
    * **board_mngr.c** in charge of managing the high-level application and board states.
    * **BLE_Speaker_WBA5_bsp_audio.c** : BLE Speaker board BSP file, derived from the stm32wba55g_discovery_audio.c file
    * **H5com.c** : component driver for communication with STM32H5 flashed with the x-cube-audio-kit modified firmware
    * **ltc3556.c** : basic component driver for communication with the LTC3556 power manager chip
    * **max9867.c** : basic component driver for communication with the MAX9867 audio codec (DAC)
    * **stc3115_Driver.c** : component driver for communication with STC3115 fuel gauge (not included in this repository)
* Add peripheral initialization not managed by the audio BSP for the I2C communication (I2C3), audio master clock generation (TIM1) and GPIOs 
* Remove downclocking of the chip during sleep mode since a timer is used for audio clock generation

Several defines are introduced at the **app_conf.h** file:
* EW25DEMO: when set to 1, the device starts to scan and synchronize a broadcast source. Additionally, audio front end is always ON, limiting pops effect on the speaker but increasing overall power consumption. When set to 0, the device starts advertising, pending a connection from a broadcast assistant or a unicast central. 
* BAT_MNGMT: when set to 1, the battery level is monitored and system may be switched off in case of low battery. Note that monitoring the battery level requires a battery to be in place and keep for a while the time to the fuel gauge to provide reliable information. 
* USB_LIVETUNE: using Livetune with USB connection requires this define to be set to 1, allowing to increase VDD voltage to 3.3 required by the USB. Keep this define to 0 for the release mode for decreasing power consumption.
* RELEASE: set this define to 1 for the product to reset in case of error and autonomously enter in shutdown mode if it is no longer active.

#### Development mode (Default configuration)
This is the mode used for tuning the audio flow with Livetune. 

EW25DEMO must be set to 1, so the application starts by scanning for a broadcast source. USB_LIVETUNE must also be set to 1 according to the default configuration of the STM32H5 firmware.

#### Release mode 
This is the mode used for measuring power consumption of the product.

EW25DEMO can be set to 0, so the application starts advertising instead of scanning. USB_LIVETUNE must be set to 0 for saving power consumption.

### **Documentation**

   - Wiki pages related to the LE Audio solutions developed by STMicroelectronics are available here:
     - <a href="https://wiki.st.com/stm32mcu/wiki/Connectivity:Introduction_to_Bluetooth_LE_Audio"> Introduction to Bluetooth® Low Energy Audio</a>
	 - <a href="https://wiki.st.com/stm32mcu/wiki/Connectivity:Bluetooth_LE_Audio_-_STM32WBA_LC3_Codec"> Bluetooth® Low Energy audio - STM32WBA LC3 codec and audio data path</a>
     - <a href="https://wiki.st.com/stm32mcu/wiki/Connectivity:Bluetooth_LE_Audio_-_STM32WBA_Architecture_and_Integration"> Bluetooth® Low Energy audio - STM32WBA Architecture and Integration</a>
     - <a href="https://wiki.st.com/stm32mcu/wiki/Connectivity:Bluetooth_LE_Audio_-_Content_Control"> Bluetooth® Low Energy audio - Content Control</a>
     - <a href="https://wiki.st.com/stm32mcu/wiki/Connectivity:Bluetooth_LE_Audio_-_STM32WBA_Public_Broadcast_Profile"> Bluetooth® Low Energy audio - Public Broadcast Profile</a>
     - <a href="https://wiki.st.com/stm32mcu/wiki/Connectivity:Bluetooth_LE_Audio_-_STM32WBA_Telephony_%26_Media_Audio_Profile"> Bluetooth® Low Energy audio - Telephony & Media Audio Profile</a>

