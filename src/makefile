# ***************************************************************************
# Interlopers
# Copyright 2004 Aaron Curtis
#
# This file is part of Interlopers.
#
# Interlopers is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# Interlopers is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Interlopers; if not, see <https://www.gnu.org/licenses/>.
# ***************************************************************************

ofiles = palm.o handlers.o game.o gfx.o prefs.o sound.o title.o
internalname = Interlopers
grcname = interlopers
creatorID = "INLO"
ccopts = -Wall -Werror -O2
pilrcopts =

ifdef options
	ccopts += -D$(options)
	pilrcopts += -D $(options)
endif

gray: prcname = interlopers_gray16
gray: prc clean
	ls -l *.prc

gray4: ccopts += -DGRAY4
gray4: pilrcopts += -D GRAY4
gray4: prcname = interlopers_gray4
gray4: prc clean
	ls -l *.prc

color: ccopts += -DCOLOR
color: pilrcopts += -D COLOR
color: prcname = interlopers_color
color: prc clean
	ls -l *.prc

debug: ccopts += -g
debug: veryclean prc clean
	ls -l *.prc

clean:
	rm -f *.bin *.grc $(grcname)

cleaner:
	rm -f *.bin *.grc $(grcname) gfx.o title.o

veryclean:
	rm -f *.bin *.grc $(grcname) *.o

prc: grc bin
	build-prc $(prcname).prc "$(internalname)" $(creatorID) *.bin *.grc $(buildopts)

bin: rc.rcp rc.h
	pilrc $(pilrcopts) rc.rcp

grc: $(ofiles)
	m68k-palmos-gcc $(ccopts) -o $(grcname) $(ofiles)
	m68k-palmos-obj-res $(grcname)

palm.o: palm.c *.h
	m68k-palmos-gcc $(ccopts) -c palm.c

handlers.o: handlers.c *.h
	m68k-palmos-gcc $(ccopts) -c handlers.c

game.o: game.c *.h
	m68k-palmos-gcc $(ccopts) -c game.c

gfx.o: gfx.c *.h
	m68k-palmos-gcc $(ccopts) -c gfx.c

prefs.o: prefs.c *.h
	m68k-palmos-gcc $(ccopts) -c prefs.c

sound.o: sound.c *.h
	m68k-palmos-gcc $(ccopts) -c sound.c

title.o: title.c *.h
	m68k-palmos-gcc $(ccopts) -c title.c
