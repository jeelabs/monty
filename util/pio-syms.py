import os
Import("env")

tag = env["PIOENV"]
pbd = env["PROJECT_BUILD_DIR"]
elf = f"{pbd}/{tag}/firmware.elf"
out = f"{pbd}/{tag}/syms.ld"

flashSegSize = 2048         # STM32L4-specific

# some globals need to be hidden from the next segment
hide = [
    '_Min_Stack_Size',
    '_Min_Heap_Size',
    '_rom_end_',
    '_ram_end_',
    '__bss_start__',
    '__bss_end__',
    '__exidx_start',
    '__exidx_end',
    '__libc_init_array',
    '__libc_fini_array',
    '_etext',
    '_init',
    '_fini',
    '_sidata',
    '_sdata',
    '_edata',
    '_sbss',
    '_ebss',
    '_siccmram',
    '_sccmram',
    '_eccmram',
    'g_pfnVectors',
    'init',
    'main',
    'SystemInit',
]

def nextMultipleOf(m, v):
    return v + (-v & (m-1)) # mult must be a power of 2

def gensyms(source, target, env):
    print(f"Extracting symbols to {out}")
    with os.popen(f"arm-none-eabi-readelf -s '{elf}'") as ifd:
        with open(out, "w") as ofd:
            syms = {}
            for line in ifd:
                fields = line.split()
                if len(fields) > 4 and fields[4] == 'GLOBAL':
                    name = fields[7]
                    value = fields[1]
                    if name in hide:
                        syms[name] = int(value, 16)
                    else:
                        print(f"{name} = 0x{value};", file=ofd)

            # calculate and save flash + ram size to skip in next segment
            romLimit = nextMultipleOf(flashSegSize, syms["_sidata"])
            print(f"_rom_end_ = 0x{romLimit:08x};", file=ofd)
            ramLimit = nextMultipleOf(8, syms["_ebss"])
            print(f"_ram_end_ = 0x{ramLimit:08x};", file=ofd)

env.AddPostAction(elf, gensyms)
