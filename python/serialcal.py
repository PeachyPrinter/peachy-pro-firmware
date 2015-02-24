#!/usr/bin/env python

import serial
import move_pb2
import sys
import time
import math
import numpy
import struct

s = serial.Serial(sys.argv[1], timeout=2)
sent = 0
start = time.time()

def send(msg, msg_type):
    global sent
    out = ['\x40', msg_type]  # \x02 is the message type for move
    for c in msg:
        if ord(c) in [0x40, 0x41, 0x42]:
            out.append('\x42')
            out.append('%c' % ((~ord(c)) & 0xFF),)
        else:
            out.append(c)
    out.append('\x41')
    sent += len(out)
    s.write(''.join(out))

print '\t'.join(['x','ch01','ch23'])
for x in xrange(1, 65535):
    pt = move_pb2.Move(x=x, y=x, id=x, laserPower=0)
    send(pt.SerializeToString(), '\x02')

    results = [x]
    for y in [0x00, 0x03]:
      meas = move_pb2.Measure(id=x, channel=y)
      send(meas.SerializeToString(), '\x03')
      res = s.read(8)
      vals = struct.unpack('IhH', res)
      results.append(vals[1])

    print '\t'.join(str(r) for r in results)
