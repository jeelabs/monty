from invoke import task

@task
def bleep(c):
    """this is a bleeper"""
    c.run("echo BLEEP")
