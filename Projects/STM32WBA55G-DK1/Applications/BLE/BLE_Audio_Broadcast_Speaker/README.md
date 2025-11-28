# **BLE_Audio_Broadcast_Speaker Application Description**

This project implements the **Broadcast Sink** and **Scan Delegator** roles as defined by the Bluetooth® SIG specification.

- As a **Broadcast Sink**, the Broadcast Speaker application scans for and synchronizes with Broadcast Sources. It can switch between multiple Broadcast Sources listed in a predefined table.
- As a **Scan Delegator**, the Broadcast Speaker application advertises and accepts connections from remote **Broadcast Assistants**. Once connected, it processes requests from the remote device to synchronize with Broadcast Sources and handles volume control requests.

This project also serves as a substitute for the BLE Speaker project, which uses a custom PCB with STM32H5 and STM32WBA MCUs, available in this repository.


### **Keywords**

Connectivity, BLE, BLE protocol, BLE pairing, BLE profile, BLE audio, Auracast


### **Hardware and Software Environment**

This example is designed to run on **STM32WBA55G-DK1** boards.

To build the project, you need one of the following IDEs:
  - IAR Embedded Workbench for ARM (EWARM) 9.60.3
  - RealView Microcontroller Development Kit (MDK-ARM) 5.38
  - STM32CubeIDE 1.18.1

Note that non-optimized configuration may lead to instablities due to high CPU load.

### **Setup**

To make the program work, follow these steps:

1. Open your preferred toolchain.
2. Rebuild all files and load the image into the target memory.
3. Run the example.

The following figure illustrates an example setup using the BLE_Audio_PBP_Source_Multilanguage and BLE_Audio_PBP_Source projects (from the STM32CubeWBA firmware) acting as Broadcast Sources, while the BLE_Audio_Broadcast_Speaker acts as the Broadcast Sink:

<div align="left">
	<img src="../../../../../Utilities/Media/BLE_Audio_Broadcast Speaker_Setup.png" alt="Setup Example" width="800" height="auto">
</div>

### **Operate the demo**

#### Source Configuration

The source names to synchronize with are configurable via the `aSourceList[]` table in the *pbp_app.c* file. A source name can be:
- The **Broadcast Name** for a remote PBP Source
- The **Complete Local Name** of a Broadcast Source
- The **Broadcast ID** of a Broadcast Source

#### Startup Behavior

- At startup, the Broadcast Speaker begins BLE scanning and searches for remote Broadcast Source devices associated with the first Broadcast Name in the `aSourceList[]` table. (The `SCAN_INTERVAL` and `SCAN_WINDOW` parameters for scanning are configurable in the *pbp_app.c* file.)
- Once a Broadcast Source is found, the Broadcast Speaker automatically starts the audio synchronization process.

#### Device Menu

On the Broadcast Speaker device, use the **Right** direction of the Joystick to access the menu. The following options are available:

- **Next Source**: Stops synchronization with the current Broadcast Source and starts scanning/synchronizing with the next Broadcast Source in the `aSourceList[]` table.
- **Switch Language**: Switches between available languages when synchronized with a multilingual Broadcast Source.
- **Start Adv.**: Starts advertising and accepts connections from remote **Broadcast Assistants**.
- **Stop Adv.**: Stops advertising.
- **Disconnect**: Initiates disconnection when connected to a Broadcast Assistant.
- **Volume...**: Opens the Volume submenu for volume control.
- **Clear Sec. Db**: Clears the security database.

#### Broadcast Source

- Power on a Broadcast Source device with a **Broadcast Name**, **Complete Local Name**, or **Broadcast ID** matching one of the entries in the `aSourceList[]` table.
- Once the Broadcast Speaker device is synchronized with the Broadcast Source and the audio stream is received, audio output becomes available via the **Jack Connector CN3**. Use headphones to listen to the audio.

#### Broadcast Assistant
- To use the Broadcast Speaker device as a **Scan Delegator** with a remote Broadcast assistant, use the **Start Adv.** menu option and refer to the STM32WBA Broadcast Assistant project's instructions on the [dedicated github repository](https://github.com/stm32-hotspot/STM32WBA-BLE-Audio-Broadcast-Assistant).

### **Documentation**

   - Wiki pages related to the LE Audio solutions developped by STMicroelectronics are available here:
     - <a href="https://wiki.st.com/stm32mcu/wiki/Connectivity:Introduction_to_Bluetooth_LE_Audio"> Introduction to Bluetooth® Low Energy Audio</a>
	 - <a href="https://wiki.st.com/stm32mcu/wiki/Connectivity:Bluetooth_LE_Audio_-_STM32WBA_LC3_Codec"> Bluetooth® Low Energy audio - STM32WBA LC3 codec and audio data path</a>
     - <a href="https://wiki.st.com/stm32mcu/wiki/Connectivity:Bluetooth_LE_Audio_-_STM32WBA_Architecture_and_Integration"> Bluetooth® Low Energy audio - STM32WBA Architecture and Integration</a>
     - <a href="https://wiki.st.com/stm32mcu/wiki/Connectivity:Bluetooth_LE_Audio_-_Content_Control"> Bluetooth® Low Energy audio - Content Control</a>
     - <a href="https://wiki.st.com/stm32mcu/wiki/Connectivity:Bluetooth_LE_Audio_-_STM32WBA_Public_Broadcast_Profile"> Bluetooth® Low Energy audio - Public Broadcast Profile</a>