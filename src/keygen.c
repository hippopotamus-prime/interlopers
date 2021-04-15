/****************************************************************************
** Interlopers
** Copyright 2004 Aaron Curtis
** 
** This file is part of Interlopers.
** 
** Interlopers is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
** 
** Interlopers is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with Interlopers; if not, see <https://www.gnu.org/licenses/>.
****************************************************************************/

#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[])
{
	char name[41] = "";
	char hname[11];
	long key = 72094;
	long altkey = 72094;
	int i;

	if(argc >= 2)
	{
		strncpy(name, argv[1], 40);
	}

	//handango-ize the name...
	if(strlen(name) > 10)
	{
		strncpy(hname, name, 5);
		strcpy(&hname[5], &name[strlen(name)-5]);
	}
	else
	{
		strcpy(hname, name);
	}

	//calculate the correct reg code
	for(i = 0; i < strlen(hname); i++)
	{
		if(hname[i] != ' ')
		{
			key += hname[i]*57;
		}
	}

	//calculate the alternate reg code
	for(i = 0; i < strlen(name); i++)
	{
		if(name[i] != ' ')
		{
			altkey += name[i]*57;
		}
	}

	altkey &= 0xffff;
	key &= 0xffff;

	printf("Registration key for %s:\n", hname);
	printf("%5d\n", key);
	printf("Alternate key for %s:\n", name);
	printf("%5d\n", altkey);

	return 0;
}
