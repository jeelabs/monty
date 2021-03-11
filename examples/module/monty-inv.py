# see https://www.pyinvoke.org

'''
This script is loaded by `inv` to define additional tasks for this area.
The example below defines a new "all" task. Simply remove everything else.
Here is a better "all" task (simply remove everything else in this script):

    @task(native, python, default=True)
    def all(c):
        """compile natively and run the Python tests"""

With the above, "inv" will compile a native build and run a basic Python
test (verify/hello.py). Adjust as needed to your own preferred workflow.
Check out the PyInvoke documentation for details: https://www.pyinvoke.org
'''

@task(default=True)
def all(c):
    """this is a demo "all" task, as defined in monty-inv.py"""

    print("""
        This message comes from the default "monty-inv.py" demo script.
        More info at https://monty.jeelabs.org - quick summary: inv -l

        Until you change "minty-inv.py", just enter: inv native python
    """)
