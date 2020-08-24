#!/usr/bin/env python3

import os, re, sys, subprocess

verbose = 0

excIds = {}
excHier = []
excFuns = []
excDefs = []

def EXCEPTION(block, name, base=''):
    id = len(excHier)
    baseId = -1 if base == '' else excIds[base]
    excIds[name] = id
    excHier.append('{ %-29s, %2d }, // %2d -> %s' %
                        (q(name), baseId, id, base))
    excFuns.append('static auto e_%s (ArgVec const& args) -> Value {' % name)
    excFuns.append('    return Exception::create(%d, args);' % id)
    excFuns.append('}')
    excFuns.append('static Function const f_%s (e_%s);' % (name, name))
    excDefs.append('{ %-29s, f_%s },' % (q(name), name))
    excFuns.append('')
    return []

def EXCEPTION_EMIT(block, sel='h'):
    if sel == 'h':
        return excHier
    if sel == 'f':
        return excFuns[:-1]
    if sel == 'd':
        return excDefs

def VERSION(block):
    v = subprocess.getoutput('git describe --tags')
    return ['constexpr auto VERSION = "%s";' % v]

# generate qstr definition
qstrIndex = []
qstrLen = [8]
qstrMap = {'mty0...': 0}

def qid(s):
    if s in qstrMap:
        i = qstrMap[s]
    else:
        i = len(qstrMap)
        qstrMap[s] = i
        qstrLen.append(len(s) + 1)
    return i

def q(s):
    return 'Q(%3d,"%s")' % (qid(s), s)

def QSTR_EMIT(block, sel='i'):
    if sel == 'i':
        return qstrIndex
    out = []
    if sel in 'xv':
        qstrLen.append(0)
        i, n, s = 0, 2 * len(qstrLen), ''
        for x in qstrLen:
            s += '\\x%02X\\x%02X' % (n & 0xFF, n >> 8)
            i += 1
            n += x
            if i % 8 == 0:
                out.append('"%s"' % s)
                s = ''
        if s:
            out.append('"%s"' % s)
        if sel == 'v':
            out.append('')
    if sel in 'sv':
        e = ['%-22s "\\0" // %d' % ('"%s"' % k, v) for k, v in qstrMap.items()]
        out.extend(e)
    return out

def QSTR(block, off=0):
    out = []
    qstrIndex.clear()
    qstrIndex.append("   0, // %d" % off)
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
        qstrIndex.append("%4d, // %d" % (n, off))
        qstrLen.append(n-pos)
        pos = n
    return out

# generate size printers
def SIZES(block, *args):
    fmt = 'printf("%%4d = sizeof %s\\n", (int) sizeof (%s));'
    return [fmt % (t, t) for t in args]

# generate a right-side comment tag
def TAG(block, *args):
    text = ' '.join(args)
    return [(76-len(text)) * ' ' + '// ' + text]

# generate the header of an exposed type
def TYPE(block, tag, *_):
    line = block[0].split()
    name, base = line[1], line[3:-1]
    base = ' '.join(base) # accept multi-word, e.g. protected
    out = [
        'struct %s : %s {' % (name, base),
        '    static auto create (ArgVec const&,Type const* =nullptr) -> Value;',
        '    static Lookup const attrs;',
        '    static Type const info;',
        '    auto type () const -> Type const& override;',
        '    auto repr (Buffer&) const -> Value override;',
    ]
    if tag.startswith('<'):
        del out[1:3] # can't construct from the VM
        del out[-1]  # no need for default repr override
    return out

# enable/disable flags for current file
flags = {}

def ON(block, *args):
    for f in args:
        flags[f] = True

def OFF(block, *args):
    for f in args:
        if hasattr(flags, f):
            del flags[f]

# generate builtin function table
builtins = [[], []]

# parse the src/defs.h header
def BUILTIN_TYPES(block, fname):
    builtins[0].clear()
    builtins[1].clear()
    info = []
    with open(fname, 'r') as f:
        for line in f:
            line = line.strip()
            if line.startswith('//CG'):
                f1 = line.split()
                if len(f1) > 1 and f1[1] == 'type':
                    f2 = next(f).split()
                    info.append([f1[2], f2[1], f2[3]])
    info.sort()
    out = []
    fmt1a = 'Type const %12s::info (%s);'
    fmt1b = 'Type const %8s::info (%-15s, %6s::create, &%s::attrs);'
    fmt2 = 'auto %12s::type () const -> Type const& { return info; }'
    sep = True
    for tag, name, base in info:
        if tag.startswith('<'):
            out.append(fmt1a % (name, q(tag)))
        else:
            if sep: out.append('')
            sep = False
            out.append(fmt1b % (name, q(tag), name, name))
    out.append('')
    for tag, name, base in info:
        out.append(fmt2 % name)
        if not tag.startswith('<'):
            builtins[1].append('{ %-15s, %s::info },' % (q(tag), name))
    return out

def BUILTIN_EMIT(block, sel):
    return builtins[sel]

def BUILTIN(block, name):
    builtins[0].append('static FunObj const f_%s (bi_%s);' % (name, name))
    builtins[1].append('{ %-20s, f_%s },' % (q(name), name))
    fmt = 'static Value bi_%s (int argc, Value argv[]) {'
    return [fmt % name]

# generate opcode switch entries
opdefs = []
opmulti = []
opraises = []

def OP_INIT(block):
    opdefs.clear()
    opmulti.clear()
    opraises.clear()
def OP_EMIT(block, sel=0):
    if sel == 'd':
        return opdefs
    if sel == 'm':
        return opmulti
    else:
        return opraises

def OP(block, typ='', multi=0):
    op = block[0].split()[1][2:]
    if 'q' in typ:
        fmt, arg, decl = ' %s', 'fetchQ()', 'char const* arg'
    elif 'v' in typ:
        fmt, arg, decl = ' %u', 'fetchV()', 'int arg'
    elif 'o' in typ:
        fmt, arg, decl = ' %d', 'fetchO()', 'int arg'
    elif 's' in typ:
        fmt, arg, decl = ' %d', 'fetchO()-0x8000', 'int arg'
    elif 'm' in typ:
        fmt, arg, decl = ' %d', 'ip[-1]', 'uint32_t arg'
    else:
        fmt, arg, decl = '', '', ''
    name = 'op' + op

    if 'm' in typ:
        opmulti.append('if ((uint32_t) (ip[-1] - Op::%s) < %d) {' % (op, multi))
        opmulti.append('    %s = ip[-1] - Op::%s;' % (decl, op))
        if 'op:print' in flags:
            opmulti.append('    printf("%s%s\\n", (int) arg);' % (op, fmt))
        opmulti.append('    %s(arg);' % name)
        opmulti.append('    break;')
        opmulti.append('}')
    else:
        opdefs.append('case Op::%s:%s' % (op, ' {' if arg else ''))
        if arg:
            opdefs.append('    %s = %s;' % (decl, arg))
        info = ', arg' if arg else ''
        if 'op:print' in flags:
            if fmt == ' %u': info = ', (unsigned) arg' # fix 32b vs 64b
            opdefs.append('    printf("%s%s\\n"%s);' % (op, fmt, info))
        if 'r' in typ:
            a = 'arg' if arg else '0'
            opdefs.append('    ctx->raise(Op::%s, %s);' % (op, a))
        else:
            a = 'arg' if arg else ''
            opdefs.append('    %s(%s);' % (name, a))
        opdefs.append('    break;')
        if arg:
            opdefs.append('}')

    out = ['void %s (%s) {' % (name, decl)]

    if 'r' in typ:
        out.append('    // note: called outside inner loop')
        opraises.append('case Op::%s:' % op)
        a = 'arg' if arg else ''
        opraises.append('    %s(%s); break;' % (name, a))

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
    result = []
    for line in lines:
        if line.strip()[0:4] != '//CG':
            result.append(line)
        else:
            if verbose:
                print(line) # TODO report line numbers
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
            if 'x' in flags and 'x'+name in globals():
                name = 'x'+name
            try:
                handler = globals()[name]
            except:
                print('unknown tag:', tag)
                return
            args, kwargs = parseArgs(params)
            replacement = handler(block, *args, **kwargs)
            if replacement is not None:
                block = replacement

            n = line.find('//CG')
            prefix = line[:n]

            if len(block) > 3:
                head = '//CG<'
            elif len(block) > 0:
                head = '//CG%d' % len(block)
            else:
                head = '//CG:'
            out = ' '.join([head, tag, *params])
            if comment:
                out += ' # ' + comment[0].strip()

            result.append(prefix + out)
            for s in block:
                result.append(prefix + s)
            if head == '//CG<':
                result.append(prefix + '//CG>')

    # perform in-line qstr lookup and replacement
    def qfix(m):
        return q(m.group(1))

    p = re.compile(r'\bQ\([ \d]*\d,"(.*?)"\)')
    return [p.sub(qfix, line) for line in result]

# process one source file, replace it only if the new contents is different
def processFile(d, f):
    path = os.path.join(d, f)
    flags.clear()
    if '/x' in path: # new code style
        flags['x'] = True
    if verbose:
        print(path)
    # TODO only process files if they have changed
    with open(path, 'r') as f:
        lines = [s.rstrip('\r\n') for s in f]
    result = processLines(iter(lines))
    if result and result != lines:
        print('rewriting:', path)
        with open(path, 'w') as f: # FIXME not safe
            for s in result:
                f.write(s+'\n')

if __name__ == '__main__':
    # args should be: first-files* root-dir last-files*
    first, root, last = [], None, []
    for f in sys.argv[1:]:
        if os.path.isdir(f):
            root = f
        elif not root:
            first.append(f)
        else:
            last.append(f)
    assert root, "no directory arg found"
    # process the first files
    for f in first:
        processFile(root, f)
    # process all files not listed as first or last
    for f in os.listdir(root):
        if not f in first and not f in last:
            if os.path.splitext(f)[1] in ['.h', '.c', '.cpp']:
                processFile(root, f)
    # process the last files
    for f in last:
        processFile(root, f)
