export PATH=$PATH:"/c/Program Files/CMake/bin":"C:\MinGW\bin":"C:\ST\STM32CubeIDE_1.14.1\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.11.3.rel1.win32_1.1.100.202309141235\tools\bin"
python build_CMSIS_DSP_library.py
#cd "C:\data\repo.AC.designer\Firmware\Drivers\CMSIS\bin"
#mingw32-make.exe -j 8 

#XX=arm_biquad_cascade_df1_q15
# cd "C:\data\repo.AC.designer\Firmware\Drivers\CMSIS\bin\bin_dsp\FilteringFunctions"
# "C:\ST\STM32CubeIDE_1.14.1\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.11.3.rel1.win32_1.1.100.202309141235\tools\bin\arm-none-eabi-gcc.exe" \
# -DARM_MATH_HELIUM -DARM_MATH_LOOPUNROLL -DARMv81MML_DSP_DP_MVE_FP -DCORTEXM -D__ARM_MVE_HYBRID_INTRINSICS \
# -I"C:\data\repo.AC.designer\Firmware\Drivers\CMSIS\Core\Include" \
# -I"C:\data\repo.AC.designer\Firmware\Drivers\CMSIS\DSP\PrivateInclude" -I"C:\data\repo.AC.designer\Firmware\Drivers\CMSIS\DSP\Include" \
# -flax-vector-conversions -ffunction-sections -fdata-sections -march=armv8.1-m.main+mve.fp+fp.dp -mfloat-abi=hard -mlittle-endian -mthumb  -mfpu=fpv5-d16 -MD -MT \
# "bin_dsp/FilteringFunctions/CMakeFiles/CMSISDSPFiltering.dir/$XX.c.o" -MF "CMakeFiles\CMSISDSPFiltering.dir/$XX.c.o.d" -o "CMakeFiles\CMSISDSPFiltering.dir/$XX.c.o" \
# -c "C:\data\repo.AC.designer\Firmware\Drivers\CMSIS\DSP\Source\FilteringFunctions/$XX.c"

echo "done"
read