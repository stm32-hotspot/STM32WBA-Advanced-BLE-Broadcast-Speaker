import numpy as np
import os        # for system commands
import colorama  # for colored characters display
import pickle    # for python data structures save & load (compact but not readable)
import json      # for python data structures save & load (less compact but more readable, all data is in one line)
import shutil
import timeit    # for time measurement (more accurate than time library on Windows platform)
import datetime  # for date & time display
import importlib
import decimal
from scripts.core.keys import CKeys as CKeys
import Levenshtein
import re

VERBOSE = False


def check_python_path():
    pythonpath = os.path.normpath(os.environ.get('PYTHONPATH'))
    pythonpath_list = pythonpath.split(";")
    pypath_is_ok = False
    for pypath in pythonpath_list:
        if pypath.endswith(os.path.normpath(CKeys.PythonRoot)):
            pypath_is_ok = True
            break

    if pypath_is_ok is False:
        print("$PYTHONPATH is wrongly set! It should end with {} but $PYTHONPATH={}\n".format(CKeys.PythonRoot, pythonpath))
        print("Please run the set_pythonpath.sh script or set your path manually in your environment\n")
        quit()


def get_cur_python_path():
    pythonpath = os.path.normpath(os.environ.get('PYTHONPATH'))
    pythonpath_list = pythonpath.split(";")
    for pypath in pythonpath_list:
        if pypath.endswith(os.path.normpath(CKeys.PythonRoot)):
            return pypath
    return None


def module_from_name(module_name):
    # load the module, will raise ImportError if module cannot be loaded
    m = importlib.import_module(module_name)
    return m


def class_from_name(module_name, class_name):
    m = module_from_name(module_name)
    # get the class, will raise AttributeError if class cannot be found
    c = getattr(m, class_name)
    return c


def set_attribute(obj, attribute, dictionary):
    if attribute in dictionary.keys():
        setattr(obj, attribute, dictionary[attribute])
    else:
        setattr(obj, attribute, None)


def get_attribute(infos, attribute, out_dict):
    if hasattr(infos, attribute):
        out_dict[attribute] = getattr(infos, attribute)
    else:
        out_dict[attribute] = None


def is_attribute_in_dict(dictionary, attribute):
    return attribute in dictionary.keys() and dictionary[attribute] is not None


def find(paths, name, absolute=False):
    names = []
    for path in paths:
        for dirpath, dirnames, filenames in os.walk(path):
            if absolute is True:
                dirpath = os.path.abspath(dirpath)
            for filename in [f for f in filenames if os.path.basename(f.lower()) == os.path.basename(name.lower())]:
                             # (f.lower().endswith(name.lower())) or (f.lower().startswith(name.lower()))]:
                names.append(os.path.join(dirpath, filename))
    return names


def search_path(path_list, file_list):
    path_found = False
    path = None
    for path in path_list:
        path_found = True
        for file in file_list:
            if not os.path.exists(os.path.join(path, file)):
                path_found = False
                break
        if path_found:
            break
    return path if path_found else None


def search_bin_path(bin_path_end, exe_file_list):
    home_drive = os.environ['HOMEDRIVE'] + os.path.sep
    path_env_list = os.environ['PATH']
    program_files = os.environ['PROGRAMFILES']
    program_files_x86 = os.environ['PROGRAMFILES(X86)']
    local_appdata_programs = os.path.join(os.environ['LOCALAPPDATA'], 'Programs')
    bin_path_list = [os.path.join(home_drive, bin_path_end),
                     os.path.join(program_files, bin_path_end),
                     os.path.join(program_files_x86, bin_path_end),
                     os.path.join(local_appdata_programs, bin_path_end)]
    for path_env in path_env_list.split(';'):
        bin_path_list.append(os.path.join(path_env, bin_path_end))
    return search_path(bin_path_list, exe_file_list)


def add_path(path):
    if path not in os.environ['PATH']:
        path_separator = ":" if (os.path.sep == "/") else ";"  # ":" if bash, ";" if Windows
        os.environ['PATH'] += path_separator + path


def print_env_variables():
    for key in os.environ.keys():
        print("%s: %s" % (key, os.getenv(key)))


def open_file(filename):
    return open(file=filename, mode="wt", encoding="utf-8")


def rm_tree(folder):
    if os.path.isdir(folder) is True:
        printc("Erasing folder: " + folder, verbose=VERBOSE)
        shutil.rmtree(folder)


def rm_file(file):
    if os.path.isfile(file):
        printc("Erasing file: " + file, verbose=VERBOSE)
        os.remove(file)


def is_dir(folder):
    if not os.path.isdir(folder):
        printc("Folder not found " + folder, colorama.Fore.MAGENTA, verbose=VERBOSE)
        return False
    return True


def is_file(file):
    if not os.path.isfile(file):
        printc("File not found " + file, colorama.Fore.MAGENTA, verbose=VERBOSE)
        return False
    return True


def is_filename_with_path(file):
    # Check if the string is an absolute path
    if os.path.isabs(file):
        return True

    # Split the path into the directory and the filename
    directory, filename = os.path.split(file)

    # If the directory part is not empty, it includes a path
    return bool(directory)


def mkdir(folder):
    if not os.path.isdir(folder):
        printc("Create folder: " + folder, verbose=VERBOSE)
        os.mkdir(folder)  # , mode=0o777)


def copy_file(file, dest):
    printc("Copy file : " + file + " to : " + dest)
    shutil.copy(file, dest)


def simplify_path(src_path, dest_path):
    src_path = os.path.normpath(src_path).replace("\\", "/")
    dest_path = os.path.normpath(dest_path).replace("\\", "/")
    i = 0
    for i in range(min(len(src_path), len(dest_path))):
        if src_path[i] != dest_path[i]:
            break
    src_path = src_path[i:]
    dest_path_simplified = dest_path[i:]
    if len(src_path) > 0:
        dest_path_simplified = "../" + dest_path_simplified
        for i in range(len(src_path)):
            if src_path[i] == "/":
                dest_path_simplified = "../" + dest_path_simplified
    return dest_path_simplified


def find_closest_string(input_str, str_list):
    # Initialize the minimum distance and the closest string
    min_distance = float('inf')
    closest_str = None

    # Iterate over the list to find the closest string
    for s in str_list:
        # Compute the Levenshtein distance between the input string and the current string
        distance = Levenshtein.distance(input_str, s)
        # Update the minimum distance and closest string if the current distance is smaller
        if distance < min_distance:
            min_distance = distance
            closest_str = s

    return closest_str


def table_complex_2_txt(vector):
    nb_rows = vector.size
    txt = ""
    for row in range(nb_rows):
        if np.mod(row + 1, 4) == 1:
            txt += '  '
        txt += '%+1.10ff, %+1.10ff' % (vector[row].real, vector[row].imag)
        if row + 1 < nb_rows:
            if np.mod(row + 1, 4) == 0:
                txt += ',\n'
            else:
                txt += ', '
    return txt


def print_float(param):
    # Determine the number of decimal places based on the input float number
    decimal_places = abs(decimal.Decimal(str(param)).as_tuple().exponent)
    txt = f"{param:.{decimal_places}f}"
    return txt


def param_value_to_txt(param):
    txt = ''
    if isinstance(param, str):
        txt = param
    elif isinstance(param, int):
        txt = str(param)
    elif isinstance(param, float):
        # txt = str('%.8f' % param)
        txt = print_float(param)
    elif isinstance(param, list):
        txt = '['
        sep = ''
        for val in param:
            txt += sep + param_value_to_txt(val)
            sep = ', '
        txt += ']'
    else:
        printc("ERROR param2txt!for %s" % param, colorama.Fore.RED, verbose=VERBOSE)
    return txt


def zeros(nb_elements):
    return [0] * nb_elements


def save_data(data, file):
    if file[-7:] == ".pickle":
        pickle_save_data(data=data, file=file)
    elif file[-5:] == ".json":
        json_save_data(data=data, file=file)
    elif file[-7:] == ".python":
        python_save_data(data=data, file=file)
    else:
        assert False, "save_data: unknown file type"


def load_data(file):
    if file[-7:] == ".pickle":
        return pickle_load_data(file)
    elif file[-5:] == ".json":
        return json_load_data(file)
    elif file[-7:] == ".python":
        return python_load_data(file)
    else:
        assert False, "load_data: unknown file type"


def pickle_save_data(data, file):
    with open(file=file, mode="wb") as f:                           # open file in byte mode for pickle
        pickle.dump(data, f, pickle.HIGHEST_PROTOCOL)


def pickle_load_data(file):
    with open(file=file, mode="rb") as f:                           # open file in byte mode for pickle
        data = pickle.load(f)
    return data


def json_save_data(data, file):
    with open(file=file, mode="wt", encoding="utf-8") as f:         # open file in text mode "utf-8" for json
        json.dump(data, f)


def json_load_data(file):
    try:
        with open(file, 'r', encoding='utf-8') as file:
            data = json.load(file)
        return data
    except json.JSONDecodeError as e:
        printc(f"Error decoding JSON: {e}", color=colorama.Fore.RED)
    except FileNotFoundError as e:
        printc(f"File not found: {e}", color=colorama.Fore.RED)
    except Exception as e:
        printc(f"An error occurred: {e}", color=colorama.Fore.RED)

    # with open(file=file, mode="rt", encoding="utf-8") as f:         # open file in text mode "utf-8" for json
    #     data = json.load(f)
    # return data


def python_save_data(data, file):
    def write_data(line, column, pos, file_handle):     # WARNING: write_data is a recursive routine
        def write_line(local_line):
            local_line = local_line.rstrip()
            if local_line != "":
                file_handle.write("%s\n" % local_line)

        if n > pos:
            cur_char = string[pos]
            if cur_char in block_char.keys():
                end_block_char = block_char[cur_char]
            else:
                end_block_char = None
            line += cur_char
            write_line(line)
            column += 2
            line = (column * ' ')
            pos += 1
            while pos < n:
                cur_char = string[pos]
                if cur_char in block_char.keys():
                    pos, line = write_data(line=line, column=column, pos=pos, file_handle=file_handle)
                elif cur_char == end_block_char:
                    write_line(line)
                    column -= 2
                    line = "%s%s" % (column * ' ', end_block_char)
                    pos += 1
                    if pos == n:    # we have reached end of string
                        write_line(line)
                        line = ""
                    return pos, line
                elif cur_char in ['"', "'"]:
                    end_string_char = cur_char
                    line += cur_char
                    pos += 1
                    while pos < n:
                        cur_char = string[pos]
                        line += cur_char
                        pos += 1
                        if cur_char == end_string_char:
                            break
                        elif (cur_char == "\\") and (pos < n):
                            # ignore next character (character following backslash) for end_string_char comparison
                            cur_char = string[pos]
                            line += cur_char
                            pos += 1
                elif cur_char == ",":
                    line += ","
                    write_line(line)
                    line = (column * ' ')
                    pos += 1
                    while string[pos] == " ":
                        pos += 1
                else:
                    line += cur_char
                    pos += 1
        write_line(line)
        line = ""
        return n, line

    string = json.dumps(data)
    n = len(string)
    block_char = {"(": ")", "[": "]", "{": "}"}
    with open(file=file, mode="wt", encoding="utf-8") as f:       # open file in text mode "utf-8" for json
        write_data(line="", column=0, pos=0, file_handle=f)


def python_load_data(file):
    try:
        with open(file=file, mode="rt", encoding="utf-8") as f:  # open file in text mode "utf-8" for json
            string = f.read()

    except FileNotFoundError:
        print("file {} does not exist".format(file))
        return None

    data = ""
    n = len(string)
    pos = 0
    start_of_line = True
    while pos < n:
        if start_of_line:
            while (pos < n) and (string[pos] == " "):
                pos += 1
        if pos < n:
            cur_char = string[pos]
            if cur_char == "\n":
                start_of_line = True
            else:
                start_of_line = False
                data += cur_char
            pos += 1
    return json.loads(data)


def dump(data=None, file_without_extension="dump"):
    if data is not None:
        pickle_save_data(data=data, file="%s.pickle" % file_without_extension)
        json_save_data(data=data, file="%s.json" % file_without_extension)
        python_save_data(data=data, file="%s.python" % file_without_extension)


def printc(txt, color=colorama.Fore.BLACK, end='\n', verbose=True, file_handle=None):
    txt_with_colors = color + txt + colorama.Back.RESET + colorama.Fore.RESET
    if verbose:
        print(txt_with_colors, end=end, flush=True, file=file_handle)
    return txt_with_colors


def get_time():
    # return time.time()
    return timeit.default_timer()   # timeit.default_timer() is more accurate than time.time() on Windows platform


def get_current_datetime_string():
    today_datetime = datetime.datetime.today()
    return "%04d%02d%02d_%02d%02d%02d" % (today_datetime.year, today_datetime.month, today_datetime.day, today_datetime.hour, today_datetime.minute, today_datetime.second)


def get_current_time_string():
    today_datetime = datetime.datetime.today()
    return "%02d/%02d/%04d %02d:%02d:%02d.%03d" % (today_datetime.day, today_datetime.month, today_datetime.year, today_datetime.hour, today_datetime.minute, today_datetime.second, today_datetime.microsecond / 1000)
    # return "%02d:%02d:%02d.%03d" % (today_datetime.hour, today_datetime.minute, today_datetime.second, today_datetime.microsecond / 1000)


class Log:
    """This class implements logging features (over display and into a log file)"""

    def __init__(self, log_file=None, log_file_handle=None, log_with_colors=False, verbose=False):
        if log_file is None:
            self.log_file_handle = log_file_handle
        else:
            self.log_file_handle = open(log_file, "w")
        self.log_with_colors = log_with_colors
        self.verbose = verbose

    def close_file(self):
        if self.log_file_handle is not None:
            self.log_file_handle.close()
            self.log_file_handle = None

    def open_file(self, log_file):
        self.log_file_handle = open(log_file, "w")

    def log(self, txt, color=colorama.Fore.BLACK, end='\n', verbose=None):
        verbose = self.verbose if (verbose is None) else verbose
        txt_with_colors = printc(txt=txt, color=color, end=end, verbose=verbose)
        if self.log_file_handle is not None:
            if self.log_with_colors:
                self.log_file_handle.write(txt_with_colors + end)
            else:
                self.log_file_handle.write(txt + end)
            self.log_file_handle.flush()


def format_list(lst):
    n = len(lst)
    string = ""
    if n > 0:
        string = str(lst[0])
        for i in range(1, n):
            string += "," + str(lst[i])
    return string


def make_output_folder(folder_out, afe_str, use_sox, normalization_type=None, filter_type=None):
    mkdir(folder_out)
    folder_out = os.path.join(folder_out, afe_str)
    mkdir(folder_out)

    if use_sox:
        sox_folder = "sox"
    else:
        sox_folder = "no_sox"

    folder_out = os.path.join(folder_out, sox_folder)
    mkdir(folder_out)

    if normalization_type is None:
        norm_folder = "normalization_none"
    else:
        norm_folder = "normalization_" + normalization_type

    if filter_type is not None:
        norm_folder = norm_folder + "_" + filter_type

    folder_out = os.path.join(folder_out, norm_folder)
    mkdir(folder_out)
    return folder_out


def get_python_file_name(python_file, denomination="file"):
    python_file = os.path.normpath(python_file)
    python_file_name = python_file.replace('.py', '').replace('\\', '.')
    pythonpath = get_cur_python_path()
    module_file = os.path.join(pythonpath, python_file)
    if pythonpath is None:
        printc("Please set $PYTHONPATH ", color=colorama.Fore.RED)
        quit()
    elif denomination == "ID_CARD" and not is_file(module_file):
        txt = "The {} ArgsAlgoListn from $PYTHONPATH={} not from the folder where the script is executed!".format(denomination, pythonpath)
        printc(txt, color=colorama.Fore.RED)
        quit()
    return python_file_name


def replace_framed_text_in_file(file_path, new_text, aperture, closure, verbose=False):
    # Read the content of the file
    with open(file_path, 'r') as file:
        old_content = file.read()

    # Create a regex pattern to find the text between the aperture and closure
    pattern = re.compile(re.escape(aperture) + r'(.*?)' + re.escape(closure), re.DOTALL)

    # Replace the text between the aperture and closure with the new text
    new_content = pattern.sub(f"{aperture}\n{new_text}\n{closure}", old_content)

    # Write the updated content back to the file
    with open(file_path, 'w') as file:
        file.write(new_content)
    if verbose:
        printc(f"Content in {file_path} has been updated and framed successfully.", color=colorama.Fore.BLUE)
