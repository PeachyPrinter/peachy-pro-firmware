#!/usr/bin/env python

"""
What does the full data structure look like

DFU Prefix - no problem
Target: has some data fields and a collection of images
Image: address, size, and data
DFU Suffix - no problem

"""

import struct
import StringIO
import zlib

class DfuseFile(object):
    prefix_fmt = "<5sBIB"
    suffix_fmt = "<HHHH3sBI"
    def __init__(self):
        self.signature = 'DfuSe'
        self.version = 1
        self.imagesize = 0
        self.targetcount = 0
        self.targets = []
        self.device = 0
        self.product = 0
        self.vendor = 0
        self.dfu = 0
        self.dfusignature = 'UFD'
        self.length = 16
        self.crc = 0
        
    def __str__(self):
        self.update_fields()
        parts = ["""Signature: %s Version: %02x ImageSize: %d TargetCount: %d""" % (self.signature, self.version, self.imagesize, self.targetcount)]
        for t in self.targets:
            parts.append("  %s" % (t,))
            for i in t.elements:
                parts.append("    %s" % (i,))
        parts.append("""Device %04x Product %04x Vendor %04x DFU %04x DFUSig "%s" Length %d CRC %08x""" % (self.device, self.product, self.vendor, self.dfu, self.dfusignature, self.length, self.crc))
        return "\n".join(parts)
            
    def from_file(self, f):
        header = f.read(11)
        self.signature, self.version, self.imagesize, self.targetcount = struct.unpack(self.prefix_fmt, header)
        self.parse_targets(f)
        suffix = f.read(16)
        self.device, self.product, self.vendor, self.dfu, self.dfusignature, self.length, self.crc = struct.unpack(self.suffix_fmt, suffix)

    def update_fields(self):
        self.targetcount = len(self.targets)
        imagesize = 11 # ourselves
        for t in self.targets:
            imagesize += t.size()
        self.imagesize = imagesize
            
    def to_file(self, f):
        outbuf = StringIO.StringIO()
        self.update_fields()
        header = struct.pack(self.prefix_fmt, self.signature, self.version, self.imagesize, self.targetcount)
        outbuf.write(header)
        for t in self.targets:
            t.to_file(outbuf)
            
        suffix = struct.pack(self.suffix_fmt, self.device, self.product, self.vendor, self.dfu, self.dfusignature, self.length, self.crc)
        outbuf.write(suffix)    

        # calculate CRC
        payload = outbuf.getvalue()[:-4]

        calculated = zlib.crc32(payload)
        # the python2 zlib seems to spit out the bit inverse of the CRC that dfu wants
        # and as a signed int
        calculated = ~calculated
        if calculated < 0:
            calculated += 2**32
        payload += struct.pack("I", calculated)
        print "Initial crc 0x%08x calculated 0x%08x" % (self.crc, calculated)
        self.crc = calculated

        f.write(payload)
        
    def parse_targets(self, f):
        for x in xrange(self.targetcount):
            target = DfuseTarget()
            target.from_file(f)
            self.targets.append(target)
            
class DfuseTarget(object):
    prefix_fmt = "<6sBI255sII"
    def __init__(self):
        self.signature = 'Target'
        self.altsetting = 0
        self.targetnamed = 0
        self.targetname = ''
        self.targetsize = 0
        self.nbelements = 0
        self.elements = []

    def update_fields(self):
        self.nbelements = len(self.elements)
        
    def size(self):
        size = 274
        imagesize = 0
        for e in self.elements:
            imagesize += e.size()
        self.targetsize = imagesize
        return size + imagesize
            
    def from_file(self, f):
        header = f.read(274)
        self.signature, self.altsetting, self.targetnamed, self.targetname, self.targetsize, self.nbelements = struct.unpack(self.prefix_fmt, header)
        self.targetname = self.targetname.strip()
        for x in xrange(self.nbelements):
            elem = DfuseImage()
            elem.from_file(f)
            self.elements.append(elem)

    def to_file(self, f):
        self.update_fields()
        header = struct.pack(self.prefix_fmt, self.signature, self.altsetting, self.targetnamed, self.targetname, self.targetsize, self.nbelements)
        f.write(header)
        for e in self.elements:
            e.to_file(f)
            
    def __str__(self):
        self.update_fields() 
        return """Target: Signature %s AltSetting %02x TargetNamed %x TargetName "%s" TargetSize %d NbElements %d""" % (self.signature, self.altsetting, self.targetnamed, self.targetname, self.targetsize, self.nbelements)
            

class DfuseImage(object):
    def __init__(self, address=0, data=""):
        self.address = address
        self.size_ = len(data)
        self.data = data

    def from_file(self, f):
        header = f.read(8)
        self.address, self.size_ = struct.unpack("<II", header)
        self.data = f.read(self.size_)

    def to_file(self, f):
        header = struct.pack("<II", self.address, self.size_)
        f.write(header)
        f.write(self.data)

    def size(self):
        return 8 + len(self.data)
        
    def __str__(self):
        return "ImageElement address %08x size %d" % (self.address, self.size_)

if __name__ == "__main__":
    dfu = DfuseFile()
    dfu.from_file(open("peachy.dfu", "rb"))
    print dfu
    print
    print
    dfu.to_file(open("roundtrip.dfu", "wb"))
    print dfu