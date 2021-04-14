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
