import numpy as np
from scripts.core.integrate_const import IntegrateStr as Const

# =========================================================================
# This function computes the CIC lookup tables for CIC order = 5.
# ==========================================================================
# The provided code is designed to compute lookup tables for a Cascaded Integrator-Comb (CIC) filter of order 5. 
# CIC filters are a class of linear phase finite impulse response (FIR) filters used primarily for decimation and interpolation in digital signal processing. 
# They are particularly useful in applications where the sample rate needs to be changed by a large factor.
#
# CIC Filter Structure:
#
#  + A CIC filter consists of a series of integrator stages followed by a series of comb stages.
#    The integrator stages accumulate the input signal, and the comb stages differentiate the signal.
#    The order of the CIC filter determines the number of integrator and comb stages.
#    Lookup Tables:
#
# Lookup tables (LUTs) are precomputed arrays that store the results of certain operations to speed up processing.
# In this context, the LUTs store the accumulated values for different stages of the CIC filter.







def generate_k_coefficients(lut_size, cic_order):
    # Initialize the K coefficients array
    k_coefficients = np.ones((lut_size, cic_order), dtype=int)

    # For LUT = 1, all Kx = 1
    if lut_size == 1:
        return k_coefficients

    # Generate K coefficients for LUT sizes greater than 1
    for lut in range(2, lut_size + 1):
        for order in range(2, cic_order + 1):
            k_coefficients[lut - 1, order - 1] = k_coefficients[lut - 2, order - 1] + k_coefficients[lut - 1, order - 2]

    return k_coefficients

def print_k_coefficients_md(lut_size, cic_order):
    # Generate the K coefficients
    k_coefficients = generate_k_coefficients(lut_size, cic_order)

    # Print the K coefficients in markdown table format
    header = "| Number of Bits | " + " | ".join([f"CIC Order {i}" for i in range(1, cic_order + 1)]) + " |"
    separator = "|---" * (cic_order + 1) + "|"
    print(header)
    print(separator)

    for lut in range(1, lut_size + 1):
        row = f"| {lut} | " + " | ".join(
            [str(k_coefficients[lut - 1, order - 1]) for order in range(1, cic_order + 1)]) + " |"
        print(row)


def write_k_coefficients_to_c_file(filename, lut_size, cic_order):
    # Generate the K coefficients
    k_coefficients = generate_k_coefficients(lut_size, cic_order)

    # Open the file for writing
    with open(filename, 'w') as c_file:
        c_file.write("#include <stdint.h>\n\n")
        c_file.write(f"#define CIC_LUT_SIZE {lut_size}\n")
        c_file.write(f"#define CIC_ORDER {cic_order}\n\n")

        # Write the K coefficients to the C file
        c_file.write("int32_t kCoefficients[CIC_LUT_SIZE][CIC_ORDER] = {\n")
        for lut in range(1, lut_size + 1):
            c_file.write("    {")
            c_file.write(", ".join([str(k_coefficients[lut - 1, order - 1]) for order in range(1, cic_order + 1)]))
            c_file.write("}")
            if lut < lut_size:
                c_file.write(",\n")
            else:
                c_file.write("\n")
        c_file.write("};\n")

def get_kCoeffs_proto(lut_size, cic_order):
    return "int32_t kCoefficients[CIC_FRAME_SIZE_MAX][CIC_ORDER_MAX]"

def write_h(file_hdle, file_basename, lut_size, cic_order):
    file_hdle.write(f"#include \"{file_basename}_conf.h\"\n")
    file_hdle.write("#include <stdint.h>\n\n")
    file_hdle.write(f"#define CIC_FRAME_SIZE_MAX {lut_size}\n")
    file_hdle.write(f"#define CIC_ORDER_MAX {cic_order}\n\n")
    str_kCoeffs = get_kCoeffs_proto(lut_size, cic_order)
    file_hdle.write(str_kCoeffs + ";\n\n")


def write_conf_h(file_hdle, file_basename, frame_size_list):
    txt = Const.HEADER.replace("audio_chain_key_replace.EXT_REPLACE", "cic_conf header")
    txt= txt.replace("key_replace", "cic_design")
    file_hdle.write(txt)
    header_protect = f"_{file_basename.upper()}_H_"
    txt = Const.H_HEADER.replace("__AUDIO_CHAIN_KEY_REPLACE_H", header_protect)
    txt = txt.replace("#include \"audio_chain.h\"", "")
    file_hdle.write(txt)
    for frame_size in frame_size_list:
        file_hdle.write(f'#define CIC_FRAME_SIZE_{frame_size}_USED\n')
    txt = Const.H_HEADER_CPP_FOOTER.replace("__AUDIO_CHAIN_KEY_REPLACE_H",header_protect)
    file_hdle.write(txt)


def write_lut_proto(file_hdle, cic_order_max, msb_lsb_list, frame_size_list):
    for frame_size in frame_size_list:
        file_hdle.write(f'#ifdef CIC_FRAME_SIZE_{frame_size}_USED\n')
        lut_size = 2 ** frame_size
        for MSBFIRST in msb_lsb_list:
            msb_lsb_str = "Msb" if MSBFIRST == 1 else "Lsb"
            base_name = f'_FrameSize{frame_size}{msb_lsb_str}'
            for order in range(1,cic_order_max+1):
                file_hdle.write(f'  int32_t lookUpAcc{order}{base_name}[{lut_size}];\n' )
        file_hdle.write(f'#endif \n\n')


def write_k_coefficients_to_c_file(file_hdle, file_basename, frame_size, cic_order):
    # Generate the K coefficients
    k_coefficients = generate_k_coefficients(frame_size, cic_order)

    file_hdle.write(f"#include \"{file_basename}.h\"\n\n")

    # Write the K coefficients to the C file
    file_hdle.write("{} = \n".format( get_kCoeffs_proto(frame_size, cic_order)))
    file_hdle.write("{ \n")
    for lut in range(1, frame_size + 1):
        file_hdle.write("    {")
        file_hdle.write(", ".join([str(k_coefficients[lut - 1, order - 1]) for order in range(1, cic_order + 1)]))
        file_hdle.write("}")
        if lut < frame_size:
            file_hdle.write(",\n")
        else:
            file_hdle.write("\n")
    file_hdle.write("};\n\n")

def write_lookup_table(c_file, table_name, table_data, LutSize):
    c_file.write(f'int32_t {table_name}[{LutSize}] = \n{{\n')
    for id_lut in range(LutSize - 1):
        c_file.write(f'{table_data[id_lut]:6d},  ')
        if (id_lut + 1) % 8 == 0:
            c_file.write('\n')
    c_file.write(f'{table_data[LutSize - 1]:6d}\n')
    c_file.write('};\n\n')



def write_lut_to_c_file(file_hdle):
    for frame_size in frame_size_list:
        file_hdle.write(f'#ifdef CIC_FRAME_SIZE_{frame_size}_USED\n')

        for MSBFIRST in msb_first_list:
            #  LutSize is calculated as (2^frame_size), which determines the number of entries in each LUT.
            LutSize = 2 ** frame_size

            # Five arrays (Acc1Tab, Acc2Tab, Acc3Tab, Acc4Tab, Acc5Tab) are initialized to store the accumulated values for each stage of the CIC filter.
            Acc1Tab = np.zeros(LutSize, dtype=int)
            Acc2Tab = np.zeros(LutSize, dtype=int)
            Acc3Tab = np.zeros(LutSize, dtype=int)
            Acc4Tab = np.zeros(LutSize, dtype=int)
            Acc5Tab = np.zeros(LutSize, dtype=int)

            for id_lut in range(LutSize):
                Acc1 = 0
                Acc2 = 0
                Acc3 = 0
                Acc4 = 0
                Acc5 = 0
                num = id_lut
                if MSBFIRST == 1:
                    mask = LutSize // 2
                else:
                    mask = 1

                # The following inner loop processes each possible input value (from 0 to LutSize - 1).
                # For each input value, the script performs bitwise operations to determine the sample value (either -1 or 1).
                # The sample value is accumulated through five stages, and the results are stored in the corresponding LUTs.
                for id_sample in range(frame_size):
                    sample = num & mask
                    if sample == 0:
                        sample = -1
                    else:
                        sample = 1
                    Acc1 += sample
                    Acc2 += Acc1
                    Acc3 += Acc2
                    Acc4 += Acc3
                    Acc5 += Acc4
                    if MSBFIRST == 1:
                        mask //= 2
                    else:
                        mask *= 2
                Acc1Tab[id_lut] = Acc1
                Acc2Tab[id_lut] = Acc2
                Acc3Tab[id_lut] = Acc3
                Acc4Tab[id_lut] = Acc4
                Acc5Tab[id_lut] = Acc5

            # The write_lookup_table function is used to write the computed LUTs to a C file.
            # The function formats the LUTs in a way that can be easily used in C programs.

            msb_lsb_str = "Msb" if MSBFIRST == 1 else "Lsb"
            base_name = f'_FrameSize{frame_size}{msb_lsb_str}'
            write_lookup_table(file_hdle, f'lookUpAcc1{base_name}', Acc1Tab, LutSize)
            write_lookup_table(file_hdle, f'lookUpAcc2{base_name}', Acc2Tab, LutSize)
            write_lookup_table(file_hdle, f'lookUpAcc3{base_name}', Acc3Tab, LutSize)
            write_lookup_table(file_hdle, f'lookUpAcc4{base_name}', Acc4Tab, LutSize)
            write_lookup_table(file_hdle, f'lookUpAcc5{base_name}', Acc5Tab, LutSize)

            print(f'All done for frame_size = {frame_size}, MSBFIRST = {msb_lsb_str}')
        file_hdle.write('#endif\n\n')

# Define the LUT size and CIC order
frame_size_list = [8]
msb_first_list = [0 ,1]
frame_size_max = max(frame_size_list)
cic_order_max = 5
lut_size_max = 2 ** frame_size_max

file_basename = "cic_design"

with open(file_basename+"_conf.h", 'w') as h_conf_file:
    write_conf_h(h_conf_file, file_basename+"_conf", frame_size_list)
    h_conf_file.close()

with open(file_basename+".h", 'w') as h_file:
    txt = Const.HEADER.replace("audio_chain_key_replace.EXT_REPLACE", "cic_conf header")
    txt= txt.replace("key_replace", "cic_design")
    h_file.write(txt)
    header_protect = f"_{file_basename.upper()}_H_"
    txt = Const.H_HEADER.replace("__AUDIO_CHAIN_KEY_REPLACE_H", header_protect)
    txt = txt.replace("#include \"audio_chain.h\"", "")
    h_file.write(txt)


    write_h(h_file, file_basename, lut_size_max, cic_order_max)
    write_lut_proto(h_file, cic_order_max, msb_first_list, frame_size_list)
    txt = Const.H_HEADER_CPP_FOOTER.replace("__AUDIO_CHAIN_KEY_REPLACE_H",header_protect)
    h_file.write(txt)

    h_file.close()

with open(file_basename+".c", 'w') as c_file:
    write_k_coefficients_to_c_file(c_file, file_basename, frame_size_max, cic_order_max)
    write_lut_to_c_file(c_file)

    c_file.close()

# print_k_coefficients_md(lut_size,cic_order)
