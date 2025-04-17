Import('RTT_ROOT')
Import('rtconfig')
from building import *

cwd = GetCurrentDir()
src = []

CPPPATH = [cwd]
list = os.listdir(cwd)

objs=[]
if GetDepend('BLUETOOTH'):
    d='porting'
    path = os.path.join(cwd, d)
    objs = objs + SConscript(os.path.join(d, 'SConscript'))    

    d='service'
    path = os.path.join(cwd, d)
    objs = objs + SConscript(os.path.join(d, 'SConscript'))    

    if GetDepend('CFG_BT_HOST') or  GetDepend('CFG_BLE_HOST') or GetDepend('ZBT'):
        d='stack'
        path = os.path.join(cwd, d)
        objs = objs + SConscript(os.path.join(d, 'SConscript'))    
        if GetDepend('ZBT'):
            d='lib'
            path = os.path.join(cwd, d)
            objs = objs + SConscript(os.path.join(d, 'SConscript'))            
    else:
        d='lib'
        path = os.path.join(cwd, d)
        objs = objs + SConscript(os.path.join(d, 'SConscript'))    

Return('objs')
