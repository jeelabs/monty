# see https://www.pyinvoke.org
from invoke import task

import configparser, io, os, subprocess, sys
from src.runner import compileIfOutdated, compareWithExpected, printSeparator

os.environ["MONTY_VERSION"] = subprocess.getoutput("git describe --tags --always")

# parse the platformio.ini file and any extra_configs it mentions
config = configparser.ConfigParser()
config.read('platformio.ini')
if "platformio" in config:
    if "extra_configs" in config["platformio"]:
        for f in config["platformio"]["extra_configs"].split():
            config.read(f)

@task(help={"host": "hostname for rsync (required)",
            "dest": "destination directory (default: monty-test)"})
def x_rsync(c, host, dest="monty-test"):
    """copy this entire area to the specified host"""
    c.run("rsync -av --exclude .git --exclude .pio . %s:%s/" % (host, dest))

@task
def x_tags(c):
    """update the (c)tags file"""
    c.run("ctags -R lib src test")

@task
def x_version(c):
    """show git repository version"""
    print(os.environ["MONTY_VERSION"])

@task(help={"verbose": "produce some extra output for debugging"})
def generate(c, verbose=False):
    """pass source files through the code generator"""
    # construct codegen args from the [codegen] section in platformio.ini
    cmd = ["src/codegen.py"]
    if verbose:
        cmd.append("-v")
    cmd += ["qstr.h", "lib/monty/", "builtin.cpp"]
    for k, v in config["codegen"].items():
        if v:
            cmd.append("+%s" % k.upper())
            cmd += v.split()
    cmd.append("qstr.cpp")
    c.run(" ".join(cmd))

@task(generate, help={"file": "name of the .py or .mpy file to run"})
def native(c, file="pytests/hello.py"):
    """run script using the native build  [pytests/hello.py]"""
    c.run("pio run -e native -s", pty=True)
    cmd = ".pio/build/native/program"
    if file:
        cmd += " " + compileIfOutdated(file)
    c.run(cmd)

@task
def test(c):
    """run C++ tests natively"""
    c.run("pio test -e native", pty=True)

@task(generate,help={"tests": "specific tests to run, comma-separated"})
def python(c, tests=""):
    """run Python tests natively          [in pytests/: {*}.py]"""
    c.run("pio run -e native -s", pty=True)
    num, match, fail, skip = 0, 0, 0, 0

    if tests:
        files = [t + ".py" for t in tests.split(",")]
    else:
        files = os.listdir("pytests")
        files.sort()
    for fn in files:
        if fn.endswith(".py"):
            if not tests and fn[1:2] == "_" and fn[0] != 'n':
                skip += 1
                continue # skip non-native tests
            num += 1
            py = "pytests/" + fn
            try:
                mpy = compileIfOutdated(py)
            except FileNotFoundError as e:
                printSeparator(py, e)
                fail += 1
            else:
                try:
                    r = c.run(".pio/build/native/program %s" % mpy,
                              timeout=2.5, hide=True)
                except Exception as e:
                    r = e.result
                    msg = "[...]\n%s\n" % r.tail('stdout', 5).strip("\n")
                    if r.stderr:
                        msg += r.stderr.strip()
                    else:
                        msg += "Unexpected exit: %d" % r.return_code
                    printSeparator(py, msg)
                    fail += 1
                else:
                    if compareWithExpected(py, r.stdout):
                        match += 1

    print(f"{num} tests, {match} matches, {fail} failures, {skip} skipped")

@task(generate)
def flash(c):
    """build embedded and re-flash attached µC"""
    try:
        # only show output if there is a problem, not the std openocd chatter
        r = c.run("pio run", hide=True, pty=True)
        out = r.stdout.split("\n", 1)[0]
        if out:
            print(out) # ... but do show the target info
    except Exception as e:
        # captured as stdout due to pty, which allows pio to colourise
        print(e.result.stdout, end="", file=sys.stderr)
        sys.exit(1)

@task
def upload(c):
    """run C++ tests, uploaded to µC"""
    c.run("pio test", pty=True)

@task(help={"tests": "specific tests to run, comma-separated"})
def runner(c, tests=""):
    """run Python tests, sent to µC       [in pytests/: {*}.py]"""
    match = "{%s}" % tests if "," in tests else (tests or "*")
    c.run("src/runner.py pytests/%s.py" % match, pty=True)

@task
def builds(c):
    """show µC build sizes, w/ and w/o assertions or Python VM"""
    c.run("pio run -t size | tail -8 | head -2")
    c.run("pio run -e noassert | tail -7 | head -1")
    c.run("pio run -e nopyvm | tail -7 | head -1")

@task
def clean(c):
    """delete all build results"""
    c.run("rm -rf .pio examples/*/.pio monty.bin")

@task
def examples(c):
    """build each of the example projects"""
    examples = os.listdir("examples")
    examples.sort()
    for ex in examples:
        if os.path.isdir("examples/%s" % ex):
            if os.path.isfile("examples/%s/platformio.ini" % ex):
                print(ex)
                c.run("pio run -d examples/%s -t size -s" % ex, warn=True)

@task(help={"offset": "flash offset (default: 0x0)",
            "upload": "upload to flash i.s.o. saving to file"})
def mrfs(c, offset=0, upload=False):
    """create Minimal Replaceable File Storage image [rom.mrfs]"""
    #c.run("cd lib/mrfs/ && g++ -std=c++11 -DTEST -o mrfs mrfs.cpp")
    #c.run("lib/mrfs/mrfs wipe && lib/mrfs/mrfs save pytests/*.mpy" )
    if upload:
        c.run(f"src/mrfs.py -u %s pytests/*.py" % offset, pty=True)
    else:
        c.run(f"src/mrfs.py -o rom.mrfs pytests/*.py")

@task
def health(c):
    """verify proper toolchain setup"""
    c.run("uname -sm")
    c.run("python3 --version")
    c.run("inv --version")
    c.run("pio --version")
    c.run("mpy-cross --version")
    #c.run("which micropython || echo NOT FOUND: micropython")

@task
def serial(c):
    """serial terminal session, use in separate window"""
    c.run("pio device monitor -b115200 --echo", pty=True)

@task(clean, test, python, upload, flash, runner, builds)
def all(c):
    """shorthand for: clean test python upload flash runner builds"""
