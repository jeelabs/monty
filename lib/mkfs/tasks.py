# see https://www.pyinvoke.org
from invoke import task

@task
def run(c):
    c.run("g++ -std=c++11 -DMAIN mkfs.cpp")
    c.run("./a.out && rm a.out")
