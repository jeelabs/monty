#!/usr/bin/env python3

import os, re, sys, subprocess

class Flags: # make all missing attributes return ""
    def __getattribute__(self,name):
        try:
            return object.__getattribute__(self, name)
        except:
            return ""

flags = Flags()     # this is cleared for each new source file
cgCounts = 0        # number of CG directives seen in this file
verbose = 0         # 0=quiet, 1=stats, 2=full
arch  = ""          # current architecture, "" is common to all
archs = {}          # list of qstr details per architecture
mods  = {"": []}    # list of extension module names per architecture
funs  = {"": []}    # list of bound functions per type/module
meths = {"": []}    # list of bound methods per type/module
dirs  = {}          # map of scanned dirnames to path, see IF

# comment lines in or out, depending on a condition
def IF(block, typ, arg):
    include = typ == "dir" and arg in dirs
    out = []
    for line in block:
        line = line.lstrip(" /")
        if not include:
            line = "//" + line
        out.append(line)
    return out

# define the module name which applies to next bind/wrap/wrappers
def MODULE(block, mod):
    flags.mod = mod
    funs[mod] = []
    meths[mod] = []
    mods[arch].append(mod)
    return []

# generate the final code to define the current module
def MODULE_END(block):
    m = flags.mod
    assert m != ""
    flags.mod = ""
    return ["static Lookup const %s_attrs (%s_map);" % (m, m),
            "Module ext_%s (%s, %s_attrs);" % (m, q(m), m)]

# emit the definitions to find all known modules
def MOD_LIST(block, sel):
    out = []
    if sel == "d":
        seen = []
        for v in mods.values():
            for m in v:
                if m not in seen:
                    seen.append(m)
        seen.sort()
        for m in seen:
            out.append("extern Module ext_%s;" % m)
    if sel == "a":
        for mArch, mNames in mods.items():
            if mNames:
                if mArch:
                    out.append("#if %s" % mArch)
                for m in mNames:
                    # lookup in arch-specific map, must already exist
                    id = archs[mArch][1][m]
                    out.append('    { Q (%d,"%s"), ext_%s },' % (id, m, m))
            if mArch:
                out.append("#endif")
    return out

# bind a function, i.e. define a callable function object wrapper
def BIND(block, fun):
    funs[flags.mod].append(fun)
    return ["static auto f_%s (ArgVec const& args) -> Value {" % fun]

# bind a method, i.e. define a callable method object wrapper
def WRAP(block, typ, *methods):
    if typ not in funs:
        funs[typ] = []
        meths[typ] = []
    for m in methods:
        meths[typ].append(m)
    return block

# emit the wrapper code, for one type/module, or for the builtins by default
def WRAPPERS(block, typ=None):
    mod = typ or flags.mod
    out = []

    funs[mod].sort()
    for f in funs[mod]:
        out.append("static Function const fo_%s (f_%s);" % (f, f))

    if mod:
        names = [mod]
    else:
        names = list(meths.keys())
        names.sort()

    fmt1 = "static auto const m_%s_%s = Method::wrap(&%s::%s);"
    fmt2 = "static Method const mo_%s_%s (m_%s_%s);"
    for t in names:
        l = t.lower()
        meths[t].sort()
        for f in meths[t]:
            out += ["",
                    fmt1 % (l, f, t, f),
                    fmt2 % (l, f, l, f)]
        if t == "":
            continue
        out += ["",
                "static Lookup::Item const %s_map [] = {" % l]
        for f in funs[t]:
            if mod:
                out.append("    { %s, fo_%s }," % (q(f), f))
            else:
                out.append("    { %s, fo_%s_%s }," % (q(f), l, f))
        for f in meths[t]:
            out.append("    { %s, mo_%s_%s }," % (q(f), l, f))
        if mod == "" or typ:
            out += ["};",
                    "Lookup const %s::attrs (%s_map);" % (t, l)]

    del funs[mod]
    del meths[mod]
    if out and not out[0]:
        del out[0]
    return out

excIds = {}     # map exception name -> id
excHier = []    # per id, code to emit, w/ parent reference
excFuns = []    # per id, code defining an exception as callable function
excDefs = []    # per id, attribute table definition

# parse the exception names and hierarchy, for generating code elsewhere
def EXCEPTIONS(block):
    global excFuns
    out = []
    for line in block:
        id = len(excHier)
        name, _, base = line.split()
        name = name.strip(",")
        baseId = -1 if base == '-' else excIds[base]
        excIds[name] = id
        excHier.append('{ %-29s %2d }, // %2d -> %s' % (q(name) + ",", baseId, id, base))
        excFuns += ['static auto e_%s (ArgVec const& args) -> Value {' % name,
                    '    return Exception::create(E::%s, args);' % name,
                    '}',
                    'static Function const fo_%s (e_%s);' % (name, name)]
        excDefs.append('{ %-29s fo_%s },' % (q(name) + ",", name))
        out.append("%-20s // %s" % (name + ",", base))
    return out

# emit various parts of the exception glue code and data
def EXCEPTION_EMIT(block, sel='h'):
    if sel == 'h':
        return excHier
    if sel == 'f':
        return excFuns
    if sel == 'd':
        return excDefs

# define a git version (not used anymore, it messes up the commit hash)
def VERSION(block):
    v = subprocess.getoutput('git describe --tags')
    return ['constexpr auto VERSION = "%s";' % v]

# generate qstr definition
qstrMap = {}    # map string to id
qstrLen = []    # per id, the symbol length + 1

# deal with one qstr map per architecture, called on each switchover
def qarch(name):
    global qstrLen, qstrMap
    archs[arch] = (qstrLen, qstrMap)
    qLen, qMap = archs[""]
    qstrLen = qLen.copy()
    qstrMap = qMap.copy()

# look up a qstr id, define a new one if needed
def qid(s):
    if s in qstrMap:
        i = qstrMap[s]
    else:
        i = len(qstrMap) + 1
        qstrMap[s] = i
        qstrLen.append(len(s) + 1)
    return i

# generate a qstr reference, with the proper ID filled in
def q(s, a=None):
    return 'Q(%3d,"%s")' % (qid(s), s)

# calculate the string hash, must produce the same result as the C++ version
def hash(s):
    h = 5381
    for b in s.encode():
        h = (((h<<5) + h) ^ b) & 0xFFFFFFFF
    return h

# emit all collected qstr information in varyvec-compatible format
def QSTR_EMIT(block):
    out = []
    for qArch, (qLen, qMap) in archs.items():
        if qArch == "":
            offset = len(qLen)
            continue
        out.append("#if %s" % qArch)

        num = len(qLen)
        qLen.insert(0, num)
        qLen.append(0)
        num += 2

        i, n, s = 0, 2 * num, ''
        for x in qLen:
            s += '\\x%02X\\x%02X' % (n & 0xFF, n >> 8)
            i += 1
            n += x
            if i % 8 == 0:
                out.append('    "%s"' % s)
                s = ''
        if s:
            out.append('    "%s"' % s)
        if verbose:
            print("\t%s: %d qstrs, %s bytes" % (qArch, i-1, n))
        out += ['    // offsets [0..%d], hashes [%d..%d], %d strings [%d..%d]' %
                                (2*i-1, 2*i, 2*i+num-3, i-2, 2*i+num-2, n-1),
                "#endif"]

    for qArch, (qLen, qMap) in archs.items():
        hashes = map(hash, qMap)
        if qArch != "":
            hashes = list(hashes)[offset:]
            out.append("#if %s" % qArch)
        i, s, h = 0, '', {}
        for x in hashes:
            s += '\\x%02X' % (x & 0xFF)
            i += 1
            h[x & 0xFF] = True
            if i % 16 == 0:
                out.append('    "%s"' % s)
                s = ''
        if s:
            out.append('    "%s"' % s)
        if qArch != "":
            out.append("#endif")

    out.append('    // end of 1-byte hashes, start of string data:')
    for qArch, (qLen, qMap) in archs.items():
        pairs = qMap.items()
        if qArch != "":
            pairs = list(pairs)[offset:]
            out.append("#if %s" % qArch)
        for k, v in pairs:
            out.append('    %-22s "\\0" // %d' % ('"%s"' % k, v))
        if qArch != "":
            out.append("#endif")

    return out

def QSTR(block, off=0):
    out = []
    sep='"\\0"'
    pos = 0
    for s in block:
        s = s.replace(sep, '')
        s = s.split('"')[-2]
        qstrMap[s] = off
        s = '"%s"' % s
        out.append('%-21s %s // %d' % (s, sep, off))
        n = pos + len(eval(s)) + 1 # deal with backslashes
        off += 1
        qstrLen.append(n-pos)
        pos = n
    return out

# generate a right-side comment tag
def TAG(block, *args):
    text = ' '.join(args)
    return [(76-len(text)) * ' ' + '// ' + text]

# generate builtin function table
publicTypes = []
hiddenTypes = []

# generate the header of an exposed type
def TYPE(block, tag, *_):
    line = block[0].split()
    name, base = line[1], line[line.index(":")+1:-1]
    base = ' '.join(base) # accept multi-word, e.g. protected
    out = [
        block[0].strip(),
        '    static auto create (ArgVec const&,Type const* =nullptr) -> Value;',
        '    static Lookup const attrs;',
        '    static Type info;',
        '    auto type () const -> Type const& override { return info; }',
        '    void repr (Buffer&) const override;',
    ]
    if tag.startswith('<'):
        hiddenTypes.append([tag,name,base])
        del out[1:3] # can't construct from the VM
        del out[-1]  # no need for default repr override
    else:
        publicTypes.append([tag,name,base])
    return out

# emit the info definitions
def TYPE_INFO(block):
    out = []
    hiddenTypes.sort()
    for tag, name, base in hiddenTypes:
        out.append('Type %12s::info (%s);' % (name, q(tag)))
    out.append("")
    publicTypes.sort()
    fmt = 'Type %8s::info (%-15s %6s::attrs, %5s::create);'
    for tag, name, base in publicTypes:
        out.append(fmt % (name, q(tag) + ",", "&" + name, name))
    return out

# generate builtin function attributes
def TYPE_BUILTIN(block):
    out = []
    publicTypes.sort()
    for tag, name, base in publicTypes:
        out.append('{ %-15s %s::info },' % (q(tag) + ",", name))
    return out

# enable/disable flags for current file

def ON(block, *args):
    for f in args:
        setattr(flags, f, True)

def OFF(block, *args):
    for f in args:
        setattr(flags, f, "")

# generate opcode switch entries
opDefs = []     # opcodes of type q)str, v)arint, o)ffset, and s)igned
opMulti = []    # opcodes of type m)ulti are emitted separately

def OP_INIT(block):
    opDefs.clear()
    opMulti.clear()

def OP_EMIT(block, sel=0):
    if sel == 'd':
        return opDefs
    if sel == 'm':
        return opMulti

def OP(block, typ='', multi=0):
    global opMulti
    op = block[0].split()[1][2:]
    if 'q' in typ:
        fmt, arg, decl = ' %s', 'fetchQ()', 'Q arg'
    elif 'v' in typ:
        fmt, arg, decl = ' %u', 'fetchV()', 'int arg'
    elif 'o' in typ:
        fmt, arg, decl = ' %d', 'fetchO()', 'int arg'
    elif 's' in typ:
        fmt, arg, decl = ' %d', 'fetchO()-0x8000', 'int arg'
    elif 'm' in typ:
        fmt, arg, decl = ' %d', '_ip[-1]', 'uint32_t arg'
    else:
        fmt, arg, decl = '', '', ''
    name = 'op' + op

    if 'm' in typ:
        opMulti += ['if ((uint32_t) (_ip[-1] - Op::%s) < %d) {' % (op, multi),
                    '    %s = _ip[-1] - Op::%s;' % (decl, op)]
        if flags.op_print:
            opMulti.append('    printf("%s%s\\n", (int) arg);' % (op, fmt))
        opMulti += ['    %s(arg);' % name,
                    '    break;',
                    '}']
    else:
        opDefs.append('case Op::%s:%s' % (op, ' {' if arg else ''))
        if arg:
            opDefs.append('    %s = %s;' % (decl, arg))
        info = ', arg' if arg else ''
        if flags.op_print:
            if fmt == ' %s': info = ', (char const*) arg' # convert from qstr
            if fmt == ' %u': info = ', (unsigned) arg' # fix 32b vs 64b
            opDefs.append('    printf("%s%s\\n"%s);' % (op, fmt, info))
        opDefs.append('    %s(%s);' % (name, 'arg' if arg else ''))
        if 's' in typ:
            opDefs.append('    loopCheck(arg);')
        opDefs.append('    break;')
        if arg:
            opDefs.append('}')

    out = ['void %s (%s) {' % (name, decl)]

    return out

# parse the py/runtime0.h header
def BINOPS(block, fname, count):
    out = []
    with open(fname, 'r') as f:
        for line in f:
            line = line.strip()
            if line.startswith('MP_BINARY_OP_'):
                out.append(line.split()[0][13:].title().replace('_', ''))
                if len(out) >= count:
                    break
    return out

# parse the py/bc0.h header
def OPCODES(block, fname):
    defs = []
    bases = {}
    with open(fname, 'r') as f:
        for line in f:
            if line.startswith('#define'):
                fields = line.split()
                if len(fields) > 3:
                    if fields[3] == '//':
                        bases['('+fields[1]] = '('+fields[2]
                    if fields[3] == '+':
                        fields[2] = bases[fields[2]]
                        val = eval(''.join(fields[2:5]))
                        key = fields[1][6:].title().replace('_', '')
                        defs.append((val, key))
    defs.sort()
    return ['%-22s = 0x%02X,' % (k, v) for v, k in defs]

# used to test codegen parameter handling
def ARGS(block, *args, **kwargs):
    out = []
    if args: out.append('// %s' % repr(args))
    if kwargs: out.append('// %s' % repr(kwargs))
    return out

# perform simple shell-like parsing of '-option value' pairs in the arg list
def parseArgs(params):
    args = []
    kwargs = {}
    it = iter(params)
    for p in it:
        v = p
        if p.startswith('-'):
            v = next(it)
        try: v = int(v)
        except ValueError:
            try: v = float(v)
            except ValueError: pass
        if p.startswith('-'):
            kwargs[p[1:]] = v
        else:
            args.append(v)
    return args, kwargs

# loop over all source lines and generate inline code at each //CG mark
def processLines(lines):
    global cgCounts
    result = []
    linenum = 0
    for line in lines:
        linenum += 1
        if line.strip()[0:4] != '//CG':
            result.append(line)
        else:
            cgCounts += 1
            if verbose > 1:
                print("%6d: %s" % (linenum, line.strip()))
            request, *comment = line.split('#', 1)
            head, tag, *params = request.split()
            block = []
            if len(head) == 4:
                pass
            elif head[4] == ':':
                pass
            elif head[4] == '<':
                while True:
                    s = next(lines)
                    if s.strip()[0:4] == '//CG':
                        if s.strip()[4] != '>':
                            raise 'missing //CG>, got: ' + s
                        break
                    block.append(s)
            else:
                count = int(head[4:])
                for _ in range(count):
                    s = next(lines)
                    if s[0:4] == '//CG':
                        raise 'unexpected //CG, got: ' + s
                    block.append(s)

            name = tag.replace('-','_').upper()
            try:
                handler = globals()[name]
            except:
                print('unknown tag:', tag)
                return

            n = line.find('//CG')
            prefix = line[:n]

            args, kwargs = parseArgs(params)
            try:
                replacement = handler(block, *args, **kwargs)
            except FileNotFoundError:
                replacement = None
                print('not found, keep as is:', args[0])
            if replacement is None:
                prefix = ''
            else:
                block = replacement

            if len(block) > 3:
                head = '//CG<'
            elif len(block) > 0:
                head = '//CG%d' % len(block)
            else:
                head = '//CG:'
            out = ' '.join([head, tag, *params])
            if comment:
                out += ' # ' + comment[0].strip()

            result.append(line[:n] + out)
            for s in block:
                result.append(prefix + s)
            if head == '//CG<':
                result.append(line[:n] + '//CG>')

    # perform in-line qstr lookup and replacement
    def qfix(m):
        return q(m.group(1))

    p = re.compile(r'\bQ\([ \d]*\d,"(.*?)"\)')
    return [p.sub(qfix, line) for line in result]

# process one source file, replace it only if the new contents is different
def processFile(path):
    global flags, cgCounts
    flags = Flags()
    cgCounts = 0
    if verbose:
        print(path + ":")
    with open(path, 'r') as fd:
        lines = [s.rstrip('\r\n') for s in fd]
    result = processLines(iter(lines))
    if verbose and cgCounts > 0:
        print("%8d CG directives" % cgCounts)
    if result and result != lines:
        print('rewriting:', path)
        with open(path, 'w') as fd: # FIXME not safe
            for s in result:
                fd.write(s+'\n')

if __name__ == '__main__':
    # args should be: [-v] first* dirs* middle* [+ARCH dirs+]* last*
    del sys.argv[0]
    while sys.argv and sys.argv[0] == "-v":
        del sys.argv[0]
        verbose += 1

    # identify the separately-processed files
    sepFiles, root = [], None
    for arg in sys.argv:
        if os.path.isdir(arg):
            bn = os.path.basename(arg.rstrip("/"))
            dirs[bn] = arg
            if not root:
                root = arg # remember first one
        if arg[0] != "+":
            sepFiles.append(arg)
    assert root, "no directory arg found"

    # process all specified args in order
    for arg in sys.argv:
        if arg[0] == "+":
            qarch(arg[1:])
            arch = arg[1:]
            mods[arch] = []
            if verbose:
                print("<GROUP %s>" % arch)
            continue
        path = os.path.join(root, arg)
        if os.path.isfile(path):
            if verbose and arch:
                print("<GROUP>")
            if arch != "":
                qarch("")
            arch = ""
            processFile(path)
        else:
            files = os.listdir(arg)
            files.sort();
            for f in files:
                if not f in sepFiles:
                    if os.path.splitext(f)[1] in ['.h', '.c', '.cpp']:
                        processFile(os.path.join(arg, f))
