import numpy as np

def generate_lookup_tables(frame_size, cic_order, msb_first):
    lut_size = 2 ** frame_size
    lookup_tables = {f'Acc{i}': np.zeros(lut_size, dtype=int) for i in range(1, cic_order + 1)}

    for i in range(lut_size):
        acc = [0] * cic_order
        num = i
        if msb_first == 1:
            mask = lut_size // 2
        else:
            mask = 1

        for j in range(frame_size):
            sample = num & mask
            sample = -1 if sample == 0 else 1
            acc[0] += sample
            for k in range(1, cic_order):
                acc[k] += acc[k - 1]
            if msb_first == 1:
                mask //= 2
            else:
                mask *= 2

        for k in range(cic_order):
            lookup_tables[f'Acc{k + 1}'][i] = acc[k]

    return lookup_tables

def write_lookup_table(c_file, table_data, LutSize, order):
    for id_lut in range(LutSize - 1):
        c_file.write(f'{table_data[f"Acc{order}"][id_lut]:6d},  ')
        if (id_lut + 1) % 8 == 0:
            c_file.write('\n')
    c_file.write(f'{table_data[f"Acc{order}"][LutSize - 1]:6d}\n')
    c_file.write('};\n\n')

def write_lookup_tables_to_c_file(c_file, frame_size, cic_order, lookup_tables, msb_first):

    msb_lsb_str = "Msb" if msb_first else "Lsb"
    for i in range(1, cic_order + 1):
        table_name = f'lookUpAcc{i}_FrameSize{frame_size}{msb_lsb_str}'
        c_file.write(f'int32_t {table_name}[{2 ** frame_size}] = \n{{\n')

        write_lookup_table(c_file, lookup_tables, 2 ** frame_size,i)

# Define the frame size and CIC order
frame_size = 8
cic_order = 5
filename = "cic_design2.c"
with open(filename, 'w') as c_file:
    c_file.write("#include \"cic_design.h\"\n")
    c_file.write("#include <stdint.h>\n\n")

    # Generate the lookup tables for LSB-first
    lookup_tables_lsb = generate_lookup_tables(frame_size, cic_order, msb_first=False)
    write_lookup_tables_to_c_file(c_file, frame_size, cic_order, lookup_tables_lsb, msb_first=False)

    # Generate the lookup tables for MSB-first
    lookup_tables_msb = generate_lookup_tables(frame_size, cic_order, msb_first=True)
    write_lookup_tables_to_c_file(c_file, frame_size, cic_order, lookup_tables_msb, msb_first=True)
    c_file.close()