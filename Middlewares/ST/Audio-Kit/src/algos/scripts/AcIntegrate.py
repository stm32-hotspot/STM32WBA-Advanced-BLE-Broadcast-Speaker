import signal
import sys
import colorama
import os
try:
    # Attempt to import the first custom module; if fail PYTHONPATH is not set
    import scripts.core.utils as utils
except ImportError as e:
    errStr = str(e)
    errModule = "No module named"
    txt = "Please run the set_pythonpath.sh script or set your $PYTHONPATH manually in your environment\n "


    if errModule in errStr and "scripts.core" not in errStr:
        txt = errStr +"\n"
        txt += "Please install the module: {}\n ".format(errStr.split(errModule)[-1])
    elif "scripts.core" in errStr:
        pass
    else:
        txt += "Currently PYTHONPATH=" + os.path.normpath(os.environ.get('PYTHONPATH')) + "\n"
        txt += errStr

    txt_with_colors = colorama.Fore.RED + txt + colorama.Back.RESET + colorama.Fore.RESET
    print(txt_with_colors)
    sys.exit(1)

utils.check_python_path()

from scripts.core.integrate_algo import Integrate


def __close_all(comment=""):
    print('todo. CLOSE ALL !')

def __signal_handler(sig, frame):
    print('You pressed Ctrl+C !')
    __close_all(comment=" (interrupted by user)")
    sys.exit(0)

# register call-back for Ctrl+C capture
signal.signal(signal.SIGINT, __signal_handler)

# utils.printc(os.getcwd(), color=colorama.Fore.YELLOW)
# txt = "Currently inside directory " + os.getcwd()
# utils.printc(txt=txt, color=colorama.Back.WHITE + colorama.Fore.BLUE, verbose=True)
# quit()
my_integration = Integrate()
my_integration.run()
my_integration.quit()

