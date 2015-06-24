#!/usr/bin/env python

import sys
import usb1
import libusb1
import time
import messages_pb2
import numpy
import math

#   idVendor           0x0483 STMicroelectronics
#  idProduct          0x5740 STM32F407

sent = 0
start = time.time()

def send(handle, msg, msgtype='\x02'):
    global sent
    out = ''.join([msgtype, msg])  # \x02 is the message type for move
    sent += len(out)
    handle.bulkWrite(2, out)

def draw_heart(handle):
    fsd = 4*256*256-1
#    points = [ messages_pb2.Move(x=int(t), y=int(t), laserPower=100) for t in numpy.linspace(0, fsd, 2048) ]
    points = [ messages_pb2.Move(x=int(fsd), y=int(0), laserPower=100) for t in numpy.linspace(0, fsd, 2048) ]

    print "First point %d, %d" % (points[0].x, points[0].y)
    print "First point %r" % (points[0].SerializeToString(),)
    print "max", max(points, key=lambda p: max(p.x, p.y))
    print "min", min(points, key=lambda p: min(p.x, p.y))

    for i in xrange(10000):
        print "Sending batches of %d points" % (len(points,))
        if i % 10 == 0:
            print sent, (sent / (time.time() - start))
        for pt in points:
            send(handle, pt.SerializeToString())

def get_id(handle):
    # Empty identify message
    send(handle, '', msgtype='\x07') 
    print "message sent, waiting for response"
    resp = ''

    # And get the response
    for x in xrange(10):
      try:
        print x,
        resp = handle.bulkRead(3, 64, timeout=10)
        if resp:
          break
      except (libusb1.USBError,), e:
        if e.value == -7:
          time.sleep(1) #timeout
        else:
            raise
    if resp:
        print "Got response of %d bytes" % (len(resp),)
        msg = messages_pb2.IAm()
        msg.ParseFromString(resp[1:])
        print msg
    else:
        print "No response"

def find():
    context = usb1.USBContext()
    return context.getByVendorIDAndProductID(0x16d0, 0x0af3)

def main():
    dev = find()
    if not dev:
        print "Couldn't find device"
        return
    handle = dev.open()
    try:
        handle.claimInterface(0)
        get_id(handle)
        draw_heart(handle)
    except (Exception,), e:
        print "Got exception", repr(e)
        raise
    finally:
      pass
      # handle.resetDevice()

if __name__ == '__main__':
    main()
