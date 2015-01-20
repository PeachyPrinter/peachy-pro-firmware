#!/usr/bin/env python

import serial
import move_pb2
import sys
import time

s = serial.Serial(sys.argv[1])

def send(msg):
    out = ['\x40', '\x02']  # \x02 is the message type for move
    for c in msg:
        if ord(c) in [0x40, 0x41, 0x42]:
            out.append('\x42')
            out.append('%c' % ((~ord(c)) & 0xFF),)
        else:
            out.append(c)
    out.append('\x41')
    s.write(''.join(out))
    print repr(msg)
    print repr(''.join(out))

fsd = 256*256-1
for i in xrange(100):
    for pt in [(0,0), (0,fsd), (fsd,fsd), (fsd, 0), (0.5*fsd, 0.5*fsd), 
               (0.25*fsd, 0.25*fsd)]:
        

        move = move_pb2.Move()
        move.id = 1234
        move.x = int(pt[0])
        move.y = int(pt[1])
        move.laserPower = 24

        send(move.SerializeToString())
        time.sleep(1)
