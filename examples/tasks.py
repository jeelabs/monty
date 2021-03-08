# see https://www.pyinvoke.org
from invoke import task

import os, sys

def getMontyDir():
    f = __file__
    for _ in range(2):
        f = os.path.dirname(f)
    return f

root = getMontyDir()
if " " in root:
    print("spaces in path not supported: '%s'" % root)
    sys.exit(1)
os.environ["MONTY_ROOT"] = root

@task(default=True)
def help(c):
    """Intro message and link to homepage"""
    for s in [
        '',
        'This is the "inv" cmdline tool for custom (out-of-tree) Monty builds',
        'Try "inv -l" for a list of all support commands or "inv -h" for help',
        'Monty is a stackless VM for µCs - homepage https://monty.jeelabs,org',
        'For source code and discussion, see https://github.com/jeelabs/monty',
        '',
    ]:
        print("   ", s)

@task
def blah(c):
    """blah blah blah"""
    c.run("echo BLAH...")

@task
def health(c):
    """Monty's health"""
    c.run("inv -r %s health" % root)

@task
def generate(c):
    """Monty's generate"""
    c.run("inv -r %s generate" % root)

sys.path.insert(0, ".")
try:
    from invconf import *
except ModuleNotFoundError: pass
