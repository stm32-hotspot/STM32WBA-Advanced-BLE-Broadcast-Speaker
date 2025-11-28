import os
import shutil
import sys
import colorama
import json
import fnmatch

VERBOSE = True


def printc(txt, color=colorama.Fore.BLACK, end='\n', verbose=True, file_handle=None):
    txt_with_colors = color + txt + colorama.Back.RESET + colorama.Fore.RESET
    if verbose:
        print(txt_with_colors, end=end, flush=True, file=file_handle)
    return txt_with_colors

def rm_tree(folder, verbose=VERBOSE):
    if os.path.isdir(folder) is True:
        printc("Erasing folder: " + folder, verbose=verbose)
        shutil.rmtree(folder)


def rm_file(file, verbose=VERBOSE):
    if os.path.isfile(file):
        printc("Erasing file: " + file, verbose=verbose)
        os.remove(file)


def is_dir(folder, verbose=VERBOSE):
    if not os.path.isdir(folder):
        printc("Folder not found " + folder, colorama.Fore.MAGENTA, verbose=verbose)
        return False
    return True


def is_file(file, verbose=VERBOSE):
    if not os.path.isfile(file):
        printc("File not found " + file, colorama.Fore.MAGENTA, verbose=verbose)
        return False
    return True


def mkdir(folder, verbose=VERBOSE):
    if not os.path.isdir(folder):
        printc("Create folder: " + folder, verbose=verbose)
        os.mkdir(folder)  # , mode=0o777)




def ignore_patterns(patterns, fw_relative_path):
    def _ignore_patterns(path, names):
        ignored_names = []
        for name in names:
            absolute_name = os.path.join(path, name)
            relative_name = os.path.relpath(absolute_name, fw_relative_path)
            if relative_name in patterns:
                ignored_names.append(name)
        for pattern in patterns:
            ignored_names.extend(fnmatch.filter(names, pattern))
        return ignored_names
    return _ignore_patterns

def homogenize_dict_paths(d):
    homogenized_dict = {}
    for key, value in d.items():
        # Normalize both the key and the value
        normalized_key = os.path.normpath(key)
        normalized_value = os.path.normpath(value)
        homogenized_dict[normalized_key] = normalized_value
    return homogenized_dict

def copy_files_and_folders(source_folder, destination_folder, infos, ignore_list=None):
    """
    Copy specified files and folders from source to destination with the possibility to change destination name.

    Args:
    - source_folder (str): Path to the source package.
    - destination_folder (str): Path to the destination package.
    - infos (dict): Dictionary where keys are source paths (relative to source_folder) and values are destination names (relative to destination_folder).

    Raises:
    - FileNotFoundError: If the destination folder does not exist.
    """
    for src, dest in infos.items():
        src_path = os.path.join(source_folder, src)
        dest_path = os.path.join(destination_folder, dest)

        if is_dir(src_path, verbose=False) :
            if is_dir(dest_path, verbose=False):
                printc(f"Folder '{dest_path}' already exists. Installation stopped. Please clean your destination folder or edit your configuration file!", color=colorama.Fore.RED, verbose=True)
                quit()

            if ignore_list is None:
                shutil.copytree(src_path, dest_path)
            else:
                shutil.copytree(src_path, dest_path,ignore=ignore_patterns(ignore_list, src))
            printc(f"Folder '{src_path}' copied to '{dest_path}'", color=colorama.Fore.YELLOW, verbose=True)

        elif is_file(src_path, verbose=False):
            dest_dir = os.path.dirname(dest_path)
            mkdir(dest_dir, verbose=True)
            shutil.copy2(src_path, dest_path)
            printc(f"File '{src_path}' copied to '{dest_path}'", color=colorama.Fore.YELLOW, verbose=True)

        else:
            printc(f"Warning: '{src_path}' does not exist and will be skipped.", color=colorama.Fore.YELLOW, verbose=True)

def clean_files_and_folders(destination_folder, infos):
    if not os.path.exists(destination_folder):
        raise FileNotFoundError(f"Destination folder '{destination_folder}' does not exist.")

    for dest in infos.values():
        dest_path = os.path.join(destination_folder, dest)
        if is_dir(dest_path, verbose=False):
            rm_tree(dest_path)
        elif is_file(dest_path, verbose=False):
            rm_file(dest_path)
        else:
            printc(f"Warning: '{dest_path}' does not exist and will be skipped during clean.", color=colorama.Fore.YELLOW)


if __name__ == "__main__":
    if len(sys.argv) < 2 or len(sys.argv) > 3:
        printc("Usage: python script.py <config_file> [--clean]", color=colorama.Fore.RED)
        sys.exit(1)

    config_file = sys.argv[1]
    clean_option = '--clean' in sys.argv

    ignore_list = {
        # dir & file names to be removed in any place of the tree temp files (item 1. from help, see above)
        '__pycache__',
        '.git',
        '.idea',
        '*.bak',
        '*.lst',
        '.settings',
        '.gitignore',
    }
    with open(config_file, 'r') as file:
        config = json.load(file)

    source_folder = os.path.normpath(config.get("source_folder"))
    destination_folder = os.path.normpath(config.get("destination_folder"))
    installer_infos = homogenize_dict_paths(config.get("installer_infos"))



    if not source_folder or not destination_folder or not installer_infos:
        printc(
            "Error: Invalid configuration file. Please ensure it contains 'source_folder', 'destination_folder', and 'installer_infos'.",
            color=colorama.Fore.RED)
        sys.exit(1)

    if clean_option:
        clean_files_and_folders(destination_folder, installer_infos)
    else:
        if not os.path.exists(destination_folder):
            mkdir(destination_folder)
            printc(f"Destination folder '{destination_folder}' created", color=colorama.Fore.BLUE,verbose=VERBOSE)

        copy_files_and_folders(source_folder, destination_folder, installer_infos, ignore_list)