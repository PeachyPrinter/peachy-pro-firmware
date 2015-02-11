#!/usr/bin/env python

import serial
import move_pb2
import sys
import time
import math
import numpy

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

for x in xrange(65535):
    pt = move_pb2.Move(x=x, y=x, id=x, laserPower=0)
    send(pt.SerializeToString(), '\x02')
    meas = move_pb2.Measure(id=x, channel=0)
    send(meas.SerializeToString(), '\x03')
    res = s.read(10)
    print x, repr(res)
