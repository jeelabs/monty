# see https://www.pyinvoke.org
from invoke import task

envMods = { "PLATFORMIO_LIB_EXTRA_DIRS": "../../lib" }

@task
def native(c):
    """show struct sizes on native"""
    c.run("pio run -e native -s", pty=True, env=envMods)
    c.run(".pio/build/native/program")

@task
def stm32(c):
    """show struct sizes on stm32"""
    c.run("pio run -e nucleo-l432 -s", hide=True, env=envMods)
    c.run("pio device monitor -e nucleo-l432 --quiet", pty=True)
