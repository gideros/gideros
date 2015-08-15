#!/usr/bin/python

import sys
import os

ROOT_PATH = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(ROOT_PATH + "/../libxmp-python")

from pyxmp import *

argc = len(sys.argv)
argv = sys.argv

if argc < 2:
	print "Usage: %s <module> [channels]" % (os.path.basename(argv[0]))
	sys.exit(1)

try:
	player = Player()
	mod = Module(argv[1], player)
except IOError as (errno, strerror):
	sys.stderr.write("%s: %s\n" % (argv[1], strerror))
	sys.exit(1)

player.start(8000, 0)
mi = mod.get_info()
fi = FrameInfo()

channels = []

if argc > 2:
	for i in range(argc - 2):
		channels.append(int(argv[i + 2]))
else:
	channels = range(mi.mod[0].chn)

while player.play_frame():
	player.get_frame_info(fi)
	if fi.loop_count > 0:
		break

	for i in channels:
		ci = fi.channel_info[i]
		print "%d %d %d %d %d %d %d %d %d" % (fi.time, fi.row,
			fi.frame, i, ci.period, ci.volume, ci.instrument,
			ci.pan, ci.sample)


player.end()
mod.release()
