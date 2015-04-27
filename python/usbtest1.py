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
    out = ['\x40', msgtype]  # \x02 is the message type for move
    for c in msg:
        if ord(c) in [0x40, 0x41, 0x42]:
            out.append('\x42')
            out.append('%c' % ((~ord(c)) & 0xFF),)
        else:
            out.append(c)
    out.append('\x41')
    sent += len(out)
    #print sent, repr(''.join(out))
    #s.write(''.join(out))
    
    handle.bulkWrite(2, ''.join(out))

def draw_heart(handle):
    fsd = 256*256-1
    sz = 256*4
    points = [ messages_pb2.Move(x=int(fsd/2.0 + -sz*(13*math.cos(t)-5*math.cos(2*t)-2*math.cos(3*t)-math.cos(4*t))), y=int(fsd/2.0 + sz*16*math.sin(t)**3), laserPower=10) for t in numpy.linspace(0, 6, 2048) ]

    for i in xrange(10000):
        if i % 100 == 0:
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
        msg.ParseFromString(resp[1:-1])
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
        if handle.kernelDriverActive(0):
            handle.detachKernelDriver(0)
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
