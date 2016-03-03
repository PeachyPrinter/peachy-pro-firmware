#!/usr/bin/env python

import dfuse
import sys

"""
Signature: DfuSe Version: 01 ImageSize: 33061 TargetCount: 1
  Target: Signature Target AltSetting 00 TargetNamed 1 TargetName "Internal Flash t53" TargetSize 32776 NbElements 1
    ImageElement address 08000000 size 32768
Device 2200 Product df11 Vendor 0483 DFU 011a DFUSig "UFD" Length 16 CRC 9ac3ef52

File is 33077 bytes long - ImageSize field is file size - 16 bytes

vs.

Signature: DfuSe Version: 01 ImageSize: 0 TargetCount: 0
  Target: Signature Target AltSetting 00 TargetNamed 1 TargetName "Internal Flash" TargetSize 0 NbElements 0
    ImageElement address 08000000 size 26172
Device 0000 Product 0000 Vendor 0000 DFU 0000 DFUSig "UFD" Length 0 CRC 00000000

"""

binfile = open(sys.argv[1], 'rb').read()

dfu = dfuse.DfuseFile()
dfu.device = 0x2200
dfu.product = 0xdf11
dfu.vendor = 0x0483
dfu.dfu = 0x011a

image = dfuse.DfuseImage(address=0x08000000, data=binfile)

target = dfuse.DfuseTarget()
target.altsetting = 0
target.targetnamed = 1
target.targetname = "Internal Flash"
target.elements.append(image)
dfu.targets.append(target)

dfu.to_file(open(sys.argv[2], 'wb'))

print dfu