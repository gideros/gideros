#!/usr/bin/python
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4

import sys
import os
import md5


ROOT_PATH = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(ROOT_PATH + "/../libxmp-python")

from pyxmp import *


def display_header(m):
    print m.name
    print m.type
    print m.pat, m.trk, m.chn, m.ins, m.smp, m.spd, m.bpm, m.len, m.rst, m.gvl
    if m.len > 0:
        print ' '.join(map(str, [ m.xxo[i] for i in range(m.len) ]))

def display_envelope(e):
    print e.flg, e.npt, e.scl, e.sus, e.sue, e.lps, e.lpe
    if e.npt > 0:
        print ' '.join(map(str, [ e.data[i] for i in range(e.npt * 2) ]))

def display_subinstrument(s):
    print s.vol, s.gvl, s.pan, s.xpo, s.fin, s.vwf, s.vde, s.vra, s.vsw, s.rvv, s.sid, s.nna, s.dct, s.dca, s.ifc, s.ifr

def display_instrument(i):
    print i.vol, i.nsm, i.rls, i.name
    display_envelope(i.get_envelope(Xmp.VOL_ENVELOPE))
    display_envelope(i.get_envelope(Xmp.FREQ_ENVELOPE))
    display_envelope(i.get_envelope(Xmp.PAN_ENVELOPE))
    print ' '.join(map(str, [ i.map[j].ins for j in range(Xmp.MAX_KEYS) ]))
    print ' '.join(map(str, [ i.map[j].xpo for j in range(Xmp.MAX_KEYS) ]))
    
    for j in range(i.nsm):
        display_subinstrument(i.get_subinstrument(j))

def display_instruments(m):
    for j in range(m.ins):
        display_instrument(m.get_instrument(j))

def display_pattern(p, chn):
    print p.rows, ' '.join(map(str, [ p.index[i] for i in range(chn) ]))

def display_patterns(m):
    for j in range(m.pat):
        display_pattern(m.get_pattern(j), m.chn)

def display_track(t):
    m = md5.new()
    for j in range(t.rows):
        m.update(t.event[j])
    print t.rows, m.hexdigest()

def display_tracks(m):
    for j in range(m.trk):
        display_track(m.get_track(j))

def display_sample(s):
    m = md5.new()
    m.update(s.get_data())
    print s.len, s.lps, s.lpe, s.flg, m.hexdigest(), s.name

def display_samples(m):
    for j in range(m.smp):
        display_sample(m.get_sample(j))

def display_channel(c):
    print c.pan, c.vol, c.flg

def display_channels(m):
    for j in range(m.chn):
        display_channel(m.get_channel(j))

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print 'Usage: {0} <module>'.format(os.path.basename(sys.argv[0]))
        sys.exit(1)
    
    try:
        module = Module(sys.argv[1])
    except IOError, error:
        sys.stderr.write('{0}: {1}\n'.format(sys.argv[1], error.strerror))
        sys.exit(1)
    
    display_header(module)
    display_instruments(module)
    display_patterns(module)
    display_tracks(module)
    display_samples(module)
    display_channels(module)
