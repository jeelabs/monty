# see https://www.pyinvoke.org
from invoke import task

@task(default=True)
def native(c):
    """show struct sizes on native"""
    c.run("pio run -e native -s", pty=True)
    c.run(".pio/build/native/program")

@task
def stm32(c):
    """show struct sizes on stm32"""
    c.run("pio run -e nucleo-l432 -s", hide=True)
    c.run("pio device monitor -e nucleo-l432 --quiet", pty=True)
