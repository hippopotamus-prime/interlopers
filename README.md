# Interlopers
> An open source game for PalmOS inspired by Space Invaders

## Overview
Interlopers is inspired by the classic Space Invaders, but
expands upon that game greatly. The objective is to prevent
slowly descending waves of aliens from reaching the bottom of
the screen (by shooting them of course - how else would you
do it?). You have barriers to shield you from the aliens' fire,
but then hiding will also give them more time to advance
downwards. 

The aliens will occasionally drop money when shot, which can
be spent on weapon upgrades and special equipment in the
between-level shop. Of course, the aliens counter this with
improvements of their own in later waves...

## Free Software
Interlopers is free software as described by the GNU General Public
License (v3). See LICENSE.md for details.

The game was originally released as shareware on mobilegamelab.com.
The open source version has had the registration check disabled so
everyone can enjoy the full game.

## System Requirements
Interlopers comes in 3 versions:

Version | Hardware
------- | --------
interlopers_color | 8-bit color devices
interlopers_gray4 | 2-bit grayscale devices
interlopers_gray16 | 4-bit grayscale devices

All require OS 3.5+

## Features

### Power Ups
When destroyed, the aliens will either drop money or one of
two power-ups. One recharges your shield (displayed at the
top-right of the screen) and the other provides energy for
your special equipment (top-left).

### Start On Any Wave
Interlopers allows you to start on any previously reached
wave (see the settings screen). However, on later waves,
the starting weapon would be of little use against the stronger
aliens.

So, the game records what weapons and equipment were used on
every wave (separately for each difficulty level). When
starting a new game, you automatically get the best arsenal
used in previous games. This shows up as a penalty in the
final score.

### Special Equipment
In addition to your main weapon, you have access to a variety
of other useful items via the shop. For example, you start
with a teleporter that sends you to a random location - useful
for avoiding enemy fire. Other items allow you to repair
your shield or repel enemy bullets, among other things.

Special equipment requires energy to run, which is in limited
supply. Your current energy is displayed at the top-left of
the screen, next to your money and shield. Energy can be
restored by collecting star power-ups that the aliens sometimes
drop.

## Controls
The controls can be reconfigured by selecting Keys on the title
screen or from the menu, but are set up as follows by default:

Button | Action
------ | ------
Date Book | Move left
Address Book | Move right
To Do List | Fire
Memo Pad | Use special equipment
Up | Fire
Down | Pause

Additionally, the stylus can be used to control player
movement.

A note on "Allow Key Events" - If checked, this allows
the OS to generate events when hard keys used by the game are
pressed.  This can cause conflicts with 3rd party software, but
is needed for some devices to function correctly (e.g. the
Treo 600's power button).

## Game Settings

### Difficulty 
Diffculty controls the speed of the aliens as well as how
often they fire at you. On Easy mode, trade-in values at the
shop are increased to 100%. Nightmare difficulty doubles the
damage of enemy fire and generates already-damaged barriers.
High scores and starting arsenal are stored separately for
each difficulty setting.

Changing the difficulty does not affect games in progress.

### Start Wave 
Start Wave determines on which wave new games will begin. Only
waves that have been previously reached are available. When
starting new games beyond the first wave, Interlopers will
automatically select the best weapon and equipment item that
were previously used on the starting wave.

## Program Settings
Setting | Description
------- | -----------
Sound Volume | Affects the volume of sounds played by the game. This will override the system preferences unless set to "System Setting".
Pen Control | Determines how the game responds to pen taps in the display area (but not the Graffiti area).
Confirm Retire | If checked, causes the game to ask before abandoning a game in progress.
Quick Restarts | If checked, causes "Try Again" to be automatically selected after you die.
Graffiti Control | Allows pen taps in the Graffiti area to control player movement when checked. This is not affected by the Pen Control setting. On some devices, it is possible that the Graffiti area might be detected incorrectly.
Compatibility Mode | If checked, alters some of the game's display code, such that it is more likely to function on all devices. Otherwise, faster assembly routines are used. On most newer devices, this option is automatically turned on and will not be displayed.

## Building
Interlopers was originally developed with [PRC Tools](http://prc-tools.sourceforge.net/), which
unfortunately has not been maintained for modern operating systems. An easy alternative to build
it is to use [prc-tools-remix](https://github.com/jichu4n/prc-tools-remix). The project offers pre-built
binaries for 64-bit Ubuntu/Debian systems and a convenient setup script to install the Palm SDK.

Run `make color`, `make gray`, and `make gray4` to build a .prc file for each
of the hardware variants. Transfer the .prc file to a device with appropriate
graphics hardware or the [Palm OS Emulator](https://sourceforge.net/projects/pose/).

