# see https://www.pyinvoke.org
from invoke import task

import os, sys

dry = "-R" in sys.argv or "--dry" in sys.argv

def getMontyDir():
    f = os.path.realpath(__file__)
    for d in ["..", "../monty"]:
        r = os.path.relpath(os.path.join(f, "..", d))
        if os.path.isfile(os.path.join(r, "src/devtasks.py")):
            return r
    print("Can't find the 'monty' source code folder, bailing out")
    sys.exit(1)

if "MONTY_ROOT" not in os.environ:
    os.environ["MONTY_ROOT"] = getMontyDir()
root = os.environ["MONTY_ROOT"]

def inRoot(f):
    return os.path.join(root, f)

@task(help={"name": "name of the new directory"})
def init(c, name):
    """Intialise a new Monty-based project"""
    orig = os.path.join(root, "examples/template/")
    c.run("mkdir %s" % name)
    c.run("cp -a %s %s" % (orig, name))
    base = os.path.relpath(root, name)
    if not dry:
        with open(os.path.join(name, "monty-pio.ini"), "w") as f:
            for s in [
                "[platformio]",
                "src_dir = %s" % os.path.join(base, "src"),
                "",
                "[env]",
                "lib_extra_dirs = %s" % os.path.join(base, "lib"),
            ]:
                print(s, file=f)
        print("Ready, the next step is: cd %s && inv -l" % name)

if os.path.isfile("tasks.py"):

    @task(default=True)
    def help(c):
        """Intro message and link to homepage"""
        for s in """
          This is the "inv" cmdline tool for custom (out-of-tree) Monty builds
          Use "inv -l" for a list of available commands, and "inv -h" for help
          Monty is a stackless VM for µCs - homepage https://monty.jeelabs.org
          For source code and discussion, see https://github.com/jeelabs/monty
        """.split("\n"):
            print(" ", s.strip())

else: # only define the other tasks if inside project (i.e. sub-) directories

    sys.path.insert(0, root)
    from src.devtasks import *

    # don't use import but execute directly in this context - this lets tasks
    # defined in "monty-inv.py" depend directly on all the tasks defined above
    if os.path.isfile("monty-inv.py"):
        with open("monty-inv.py") as f:
            exec(f.read())
