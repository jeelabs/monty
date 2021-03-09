# see https://www.pyinvoke.org
from invoke import task

import os, sys

def getMontyDir():
    for d in ["..", "../monty"]:
        r = os.path.relpath(os.path.join(__file__, "..", d))
        if os.path.isfile(os.path.join(r, "src/devtasks.py")):
            return r
    print("Can't find the 'monty' source code folder (should be a sibling)")
    sys.exit(1)

if "MONTY_ROOT" not in os.environ:
    os.environ["MONTY_ROOT"] = getMontyDir()

@task(help={"name": "name of the new directory"})
def init(c, name):
    """Intialise a new Monty-based project"""
    c.run("echo INIT ...")

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

else: # only define the next tasks when in project (i.e. sub-) directories

    sys.path.insert(0, os.environ["MONTY_ROOT"])
    from src.devtasks import *

    @task
    def all(c):
        """(not defined - add to "monty-inv.py" if you want it)"""
        print('not yet, see "monty.inv.py" for details')

    # the filename has a dash in it, so a plain "from ... import *" won't work
    # (I'm not a fan of underscores_in_names, even less so in file names -jcw)
    if os.path.isfile("monty-inv.py"):
        import importlib
        sys.path.insert(0, ".")
        globals().update(vars(importlib.import_module("monty-inv")))
