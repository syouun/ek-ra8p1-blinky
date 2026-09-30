#!/usr/bin/env python3
"""Build check of mtk3bsp2_ra8p1_ek without e2 studio (same compiler flags as the e2 studio Debug configuration).

    python3 tools/build_gcc.py [build_dir]

Requirements: Arm GNU Toolchain 13.2.1 (arm-none-eabi-gcc) in PATH, or set ARM_GCC_BIN to its bin directory.
The linker files that e2 studio normally generates into Debug/ (fsp_gen.ld, memory_regions.ld, bsp_linker_info.h)
are taken from tools/linker/. The official way to build and flash is e2 studio (see docs/setup_guide.md);
this script is for quick compile/link checks, e.g. on Linux or in CI.

Copyright (c) 2026 Makoto Tanaka (syouun)
SPDX-License-Identifier: MIT
"""
import os, sys, shutil, subprocess, concurrent.futures as cf

HERE = os.path.dirname(os.path.abspath(__file__))
TC = os.environ.get('ARM_GCC_BIN', '')
def tool(name):
    return os.path.join(TC, name) if TC else name
GCC, GXX = tool('arm-none-eabi-gcc'), tool('arm-none-eabi-g++')

proj = os.path.join(os.path.dirname(HERE), 'mtk3bsp2_ra8p1_ek')
bdir = os.path.abspath(sys.argv[1]) if len(sys.argv) > 1 and not sys.argv[1].startswith('--') else os.path.join(os.path.dirname(HERE), 'build')
os.makedirs(bdir, exist_ok=True)
for f in ('fsp_gen.ld', 'memory_regions.ld', 'bsp_linker_info.h'):
    shutil.copy(os.path.join(HERE, 'linker', f), bdir)
CPU = ['-mthumb', '-mfloat-abi=hard', '-mcpu=cortex-m85+nopacbti']
COMMON = CPU + ['-O0', '-fmessage-length=0', '-fsigned-char', '-ffunction-sections', '-fdata-sections',
                '-fno-strict-aliasing', '-Wall', '-g']
DEFS = ['-D_RENESAS_RA_', '-D_RA_CORE=CPU0', '-D_RA_ORDINAL=1', '-D_RAFSP_EK_RA8P1_']
INC_C = ['ra_cfg/fsp_cfg/bsp', '.', 'ra_gen', 'ra_cfg/fsp_cfg', 'src', 'ra/fsp/inc', 'ra/fsp/inc/api', 'ra/fsp/inc/instances',
         'ra/arm/CMSIS_6/CMSIS/Core/Include', 'mtk3_bsp2', 'mtk3_bsp2/config', 'mtk3_bsp2/include',
         'mtk3_bsp2/mtkernel/kernel/knlinc', 'ra/fsp/src/r_drw', 'ra/fsp/src/r_mipi_csi', 'ra/fsp/src/r_vin',
         'ra/fsp/src/rm_ethosu', 'ra/tes/dave2d/inc', 'ra/npu/ethos-u-core-driver/include', 'ra/arm/CMSIS-DSP/Include',
         'ra/arm/CMSIS-DSP/PrivateInclude', 'ra/arm/CMSIS-NN/Include', 'ra/arm/CMSIS-View/EventRecorder/Include',
         'ra/arm/CMSIS-View/EventRecorder/Config']
INC_CXX = INC_C + ['src/ai_application/tflite_headers']
INC_AS = INC_C[:19]

def incs(lst):
    return ['-I' + (bdir if p == '.' else os.path.join(proj, p)) for p in lst]

def extra(rel):
    if rel.startswith('ra/npu') or rel.startswith('ra/tes'):
        return ['-w']
    if rel.startswith('ra/arm'):
        return ['-w', '-O2']
    return []

def cmd_for(rel):
    src = os.path.join(proj, rel)
    obj = os.path.join(bdir, 'obj', rel + '.o')
    ext = os.path.splitext(rel)[1]
    if ext == '.c':
        c = [GCC] + COMMON + DEFS + incs(INC_C) + ['-std=c99', '-Wno-stringop-overflow', '-Wno-format-truncation',
             '-flax-vector-conversions', '--param=min-pagesize=0'] + extra(rel) + ['-c', '-o', obj, '-x', 'c', src]
    elif ext in ('.cpp', '.cc'):
        c = [GXX] + COMMON + DEFS + ['-DTF_LITE_STATIC_MEMORY'] + incs(INC_CXX) + ['-std=c++17', '-fno-exceptions', '-fno-rtti',
             '-Wno-stringop-overflow', '-Wno-format-truncation', '-flax-vector-conversions', '--param=min-pagesize=0'] + extra(rel) + ['-c', '-o', obj, src]
    elif ext in ('.S', '.s', '.asm'):
        c = [GCC] + COMMON + ['-x', 'assembler-with-cpp'] + DEFS + incs(INC_AS) + ['-c', '-o', obj, src]
    else:
        return None
    return obj, c

sources = []
for top in ['Application', 'mtk3_bsp2', 'ra', 'ra_gen', 'src']:
    for root, dirs, files in os.walk(os.path.join(proj, top)):
        for f in files:
            rel = os.path.relpath(os.path.join(root, f), proj).replace(os.sep, '/')
            if rel in ('Application/app_i2c.c', 'Application/app_adc.c'):
                continue
            if os.path.splitext(f)[1] in ('.c', '.cpp', '.cc', '.S', '.s', '.asm'):
                sources.append(rel)

only = sys.argv[sys.argv.index('--only') + 1:] if '--only' in sys.argv else None
if only:
    sources = [s for s in sources if s in only]

def run(rel):
    r = cmd_for(rel)
    if r is None:
        return rel, None, 0, ''
    obj, c = r
    os.makedirs(os.path.dirname(obj), exist_ok=True)
    p = subprocess.run(c, capture_output=True, text=True)
    return rel, obj, p.returncode, p.stderr

objs, fails = [], 0
with cf.ThreadPoolExecutor(os.cpu_count() or 4) as ex:
    for rel, obj, rc, err in ex.map(run, sources):
        if obj is None:
            continue
        if err.strip():
            print('----', rel); print(err.strip())
        if rc:
            fails += 1
        else:
            objs.append(obj)
print(f'compiled {len(objs)} / {len(sources)} files, {fails} failed')
if fails or only:
    sys.exit(1 if fails else 0)

elf = os.path.join(bdir, 'mtk3bsp2_ra8p1_ek.elf')
link = [GXX] + COMMON + ['-T', 'fsp.ld', '-T', os.path.join(proj, 'mtk3_bsp2/etc/linker/mtkernel.ld'), '-Xlinker', '--gc-sections',
       '-L', os.path.join(proj, 'script'), '-L', bdir, '-Wl,-Map,' + os.path.join(bdir, 'mtk3bsp2_ra8p1_ek.map'),
       '--specs=nano.specs', '--specs=nosys.specs', '-o', elf, '-Wl,--start-group'] + objs + ['-Wl,--end-group']
p = subprocess.run(link, capture_output=True, text=True)
print(p.stderr.strip()[-6000:])
print('LINK', 'OK' if p.returncode == 0 else 'FAILED')
if p.returncode == 0:
    print(subprocess.run([tool('arm-none-eabi-size'), elf], capture_output=True, text=True).stdout)
sys.exit(p.returncode)
