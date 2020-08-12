#!/usr/bin/env python3

import os, sys, subprocess

verbose = 0

# generate qstr definition
qstrIndex = []

def QSTR_EMIT(block):
    return qstrIndex

def QSTR(block, off=0):
    out = []
    qstrIndex.clear()
    qstrIndex.append("   0, // %d" % off)
    sep='"\\0"'
    pos = 0
    for s in block:
        s = s.replace(sep, '')
        s = '"%s"' % s.split('"')[-2]
        out.append('%-21s %s // %d' % (s, sep, off))
        n = pos + len(eval(s)) + 1 # deal with backslashes
        off += 1
        qstrIndex.append("%4d, // %d" % (n, off))
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
def xTYPE(block, tag, *_):
    line = block[0].split()
    name, base = line[1], line[3:-1]
    base = ' '.join(base) # accept multi-word, e.g. protected
    out = [
        'struct %s : %s {' % (name, base),
        '    static auto create (Chunk const&,Type const* =nullptr) -> Value;',
        '    static Lookup const attrs;',
        '    static Type const info;',
        '    auto type () const -> Type const& override;',
        '    auto repr (Buffer&) const -> Value override;',
    ]
    if tag.startswith('<'):
        del out[1:3] # can't construct from the VM
    return out

# generate the header of an exposed type
def TYPE(block, tag, *_):
    line = block[0].split()
    name, base = line[1], line[3:-1]
    base = ' '.join(base) # accept multi-word, e.g. protected
    out = [
        'struct %s : %s {' % (name, base),
        '    static Value create (const TypeObj&, int argc, Value argv[]);',
        '    static const LookupObj attrs;',
        '    static const TypeObj info;',
        '    const TypeObj& type () const override;',
    ]
    if tag.startswith('<'):
        del out[1:3] # can't construct from the VM
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
def xBUILTIN_TYPES(block, fname):
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
    fmt1a = 'Type const %12s::info ("%s");'
    fmt1b = 'Type const %8s::info ("%s", %s::create, &%s::attrs);'
    fmt2 = 'auto %12s::type () const -> Type const& { return info; }'
    sep = True
    for tag, name, base in info:
        if tag.startswith('<'):
            out.append(fmt1a % (name, tag))
        else:
            if sep: out.append('')
            sep = False
            out.append(fmt1b % (name, tag, name, name))
    out.append('')
    for tag, name, base in info:
        out.append(fmt2 % name)
        if not tag.startswith('<'):
            builtins[1].append('{ "%s", &%s::info },' % (tag, name))
    return out

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
    fmt1a = 'const TypeObj %12s::info ("%s");'
    fmt1b = 'const TypeObj %8s::info ("%s", %s::create, &%s::attrs);'
    fmt2 = 'const TypeObj& %12s::type () const { return info; }'
    sep = True
    for tag, name, base in info:
        if tag.startswith('<'):
            out.append(fmt1a % (name, tag))
        else:
            if sep: out.append('')
            sep = False
            out.append(fmt1b % (name, tag, name, name))
    out.append('')
    for tag, name, base in info:
        out.append(fmt2 % name)
        if not tag.startswith('<'):
            builtins[1].append('{ "%s", &%s::info },' % (tag, name))
    return out

def BUILTIN_EMIT(block, sel):
    return builtins[sel]

def BUILTIN(block, name):
    builtins[0].append('static FunObj const f_%s (bi_%s);' % (name, name))
    builtins[1].append('{ "%s", &f_%s },' % (name, name))
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

def xOP(block, typ='', multi=0):
    op = block[0].split()[1][3:]
    if 'q' in typ:
        fmt, arg, decl = ' %s', 'fetchQstr()', 'char const* arg'
    elif 'v' in typ:
        fmt, arg, decl = ' %u', 'fetchVarInt()', 'int arg'
    elif 'o' in typ:
        fmt, arg, decl = ' %d', 'fetchOffset()', 'int arg'
    elif 's' in typ:
        fmt, arg, decl = ' %d', 'fetchOffset()-0x8000', 'int arg'
    elif 'm' in typ:
        fmt, arg, decl = ' %d', 'ip[-1]', 'uint32_t arg'
    else:
        fmt, arg, decl = '', '', ''
    name = 'op_' + op

    if 'm' in typ:
        opmulti.append('if ((uint32_t) (ip[-1] - Op::%s) < %d) {' % (op, multi))
        opmulti.append('    %s = ip[-1] - Op::%s;' % (decl, op))
        if 'op:print' in flags:
            opmulti.append('    printf("%s%s\\n", (int) arg);' % (op, fmt))
        opmulti.append('    %s(arg);' % name)
        opmulti.append('    break;')
        opmulti.append('}')
    else:
        opdefs.append('case Op::%s: %s' % (op, '{' if arg else ''))
        if arg:
            opdefs.append('    %s = %s;' % (decl, arg))
        info = ', arg' if arg else ''
        if 'op:print' in flags:
            if fmt == ' %u': info = ', (unsigned) arg' # fix 32b vs 64b
            opdefs.append('    printf("%s%s\\n"%s);' % (op, fmt, info))
        if 'r' in typ:
            a = 'arg' if arg else '0'
            opdefs.append('    ctx.raise(Op::%s, %s);' % (op, a))
        else:
            a = 'arg' if arg else ''
            opdefs.append('    %s(%s);' % (name, a))
        opdefs.append('    break;')
        if arg:
            opdefs.append('}')

    out = ['void %s (%s) {' % (name, decl)]

    if 'r' in typ:
        out.append('    // note: called from outer loop')
        opraises.append('case Op::%s:' % op)
        a = 'arg' if arg else ''
        opraises.append('    %s(%s); break;' % (name, a))

    return out

def OP(block, typ=''):
    op = block[0].split()[1][3:]
    opdefs.append('case Op::%s:' % op)
    if typ == 'q':
        fmt, arg, decl = ' %s', 'fetchQstr()', 'char const* arg'
    elif typ == 'v':
        fmt, arg, decl = ' %u', 'fetchVarInt()', 'uint32_t arg'
    elif typ == 'o':
        fmt, arg, decl = ' %d', 'fetchOffset()', 'int arg'
    elif typ == 's':
        fmt, arg, decl = ' %d', 'fetchOffset()-0x8000', 'int arg'
    else:
        fmt, arg, decl = '', '', ''
    name = 'op_' + op
    opdefs.append('    %s(%s); break;' % (name, arg))
    out = ['void %s (%s) {' % (name, decl)]
    if 'op:print' in flags:
        info = ', arg' if arg else ''
        if fmt == ' %u': info = ', (unsigned) arg' # fix 32b vs 64b
        out.append('    printf("%s%s\\n"%s);' % (op, fmt, info))
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

    return result

# process one source file, replace it only if the new contents is different
def processFile(path):
    flags.clear()
    if '/x' in path: # new code style
        flags['x'] = True
    if verbose:
        print(path)
    # TODO only process files if they have changed
    with open(path, 'r') as f:
        lines = [s.rstrip() for s in f]
    result = processLines(iter(lines))
    if result and result != lines:
        print('rewriting:', path)
        with open(path, 'w') as f: # FIXME not safe
            for s in result:
                f.write(s+'\n')

# process all C/C++ sources and headers in specified directory
def processDir(dir):
    for name in os.listdir(dir):
        if os.path.splitext(name)[1] in ['.h', '.c', '.cpp']:
            processFile(os.path.join(dir, name))

if __name__ == '__main__':
    for d in sys.argv[1:]:
        processDir(d)

    p = os.path
    with open(p.join(p.dirname(sys.argv[0]), "version.h"), "w") as f:
        v = subprocess.getoutput('git describe --tags')
        f.write('#define VERSION "%s"\n' % v)
