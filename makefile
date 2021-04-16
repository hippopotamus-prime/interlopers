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

ifdef DEBUG
	ccopts += -g
endif

.PHONY: all gray gray4 color clean veryclean bin_gray4 bin_gray16 bin_color grc_gray4 grc_gray16 grc_color
.SECONDARY: obj.gray4 obj.gray16 obj.color

all: gray gray4 color

gray: interlopers_gray16.prc
	ls -l $<

gray4: interlopers_gray4.prc
	ls -l $<

color: interlopers_color.prc
	ls -l $<

clean:
	rm -rf bin.* grc.*

veryclean:
	rm -rf bin.* grc.* obj.* *.prc

interlopers_%.prc: bin_% grc_%
	build-prc $@ "$(internalname)" $(creatorID) bin.$*/*.bin grc.$*/*.grc $(buildopts)

bin_gray4: src/*.rcp src/rc.h | bin.gray4
	cd src && pilrc -D GRAY4 rc.rcp ../bin.gray4

bin_gray16: src/*.rcp src/rc.h | bin.gray16
	cd src && pilrc rc.rcp ../bin.gray16

bin_color: src/*.rcp src/rc.h | bin.color
	cd src && pilrc -D COLOR rc.rcp ../bin.color

grc_gray4: $(patsubst %.o,obj.gray4/%.o,$(ofiles)) | grc.gray4
	m68k-palmos-gcc $(ccopts) -DGRAY4 -o grc.gray4/$(grcname) $^
	cd grc.gray4 && m68k-palmos-obj-res $(grcname)

grc_gray16: $(patsubst %.o,obj.gray16/%.o,$(ofiles)) | grc.gray16
	m68k-palmos-gcc $(ccopts) -o grc.gray16/$(grcname) $^
	cd grc.gray16 && m68k-palmos-obj-res $(grcname)

grc_color: $(patsubst %.o,obj.color/%.o,$(ofiles)) | grc.color
	m68k-palmos-gcc $(ccopts) -DCOLOR -o grc.color/$(grcname) $^
	cd grc.color && m68k-palmos-obj-res $(grcname)

obj.gray4/%.o: src/%.c src/*.h | obj.gray4
	m68k-palmos-gcc $(ccopts) -DGRAY4 -c -o $@ $<

obj.gray16/%.o: src/%.c src/*.h | obj.gray16
	m68k-palmos-gcc $(ccopts) -c -o $@ $<

obj.color/%.o: src/%.c src/*.h | obj.color
	m68k-palmos-gcc $(ccopts) -DCOLOR -c -o $@ $<

bin.%:
	mkdir -p $@

grc.%:
	mkdir -p $@

obj.%:
	mkdir -p $@
