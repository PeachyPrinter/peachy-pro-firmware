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

i = 1
while True:
    x = None
    try:
        p = raw_input("Enter target power: ")
    except EOFError:
        break
    try:
        x = int(p)
    except ValueError:
        x = None
    if (x is None) or (x < 0) or (x > 255):
        print "Invalid value (enter a number between 0 and 255)"
        continue
    print "Setting laser power to %d" % (x,)

    pt = move_pb2.Move(x=32768, y=32768, id=i, laserPower=x)
    i += 1
    send(pt.SerializeToString(), '\x02')
    
print
