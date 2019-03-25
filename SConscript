# RT-Thread building script for GuiEngine

import os
import rtconfig
from building import *

cwd = GetCurrentDir()

src = Glob('src/*.c')
CPPPATH = [cwd + '/include']

if GetDepend('GUIENGINE_IMAGE_TJPGD'):
    src += Glob('libraries/tjpgd1a/*.c')
    CPPPATH += [cwd + '/libraries/tjpgd1a']

if GetDepend('GUIENGINE_IMAGE_LODEPNG'):
    if rtconfig.ARCH == 'sim':
            src += Glob('libraries/lodepng/*.c')
            CPPPATH += [cwd + '/libraries/lodepng']
    else:
        if GetDepend('RT_USING_LIBC'):
            src += Glob('libraries/lodepng/*.c')
            CPPPATH += [cwd + '/libraries/lodepng']

group = DefineGroup('GuiEngine', src, depend = ['PKG_USING_GUIENGINE'], CPPPATH = CPPPATH)

if GetDepend('GUIENGINE_USING_DEMO'):
    group = group + SConscript(os.path.join('example', 'SConscript'))

if GetDepend('GUIENGINE_USING_TTF'):
    group = group + SConscript(os.path.join('libraries/freetype-2.6.2', 'SConscript'))

Return('group')
