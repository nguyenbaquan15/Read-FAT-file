#include <stdio.h>
#include <stdint.h>
#include "HAL.h"
#include "FAT.h"

FILE *file;
char FileName[20]="file3.ima";
uint16_t BytePerSector;

uint8_t HAL_OpenFile(void)
{
	uint8_t check;
	file=fopen(FileName,"rb");
	if(file!=NULL)
	{
		check=1;
	}
	else
	{
		check=0;
	}
}

void HAL_CloseFile(void)
{
	fclose(file);
}

void HAL_ReadBytePerSector(void)
{
	uint8_t Buffer[2];
	fseek(file,0x0B,SEEK_SET);
	fread(Buffer,1,sizeof(Buffer),file);
	BytePerSector=ConvertHexaDec(Buffer,0,2);
}

void HAL_ReadSector(uint8_t Buffer[], uint16_t Sector)
{
	fseek(file,Sector*BytePerSector,SEEK_SET);
	fread(Buffer,1,BytePerSector,file);
	rewind(file);
}

void HAL_ReadMultiSector(uint8_t Buffer[], uint16_t Sector, uint8_t Number)
{
	fseek(file,Sector*BytePerSector,SEEK_SET);
	fread(Buffer,1,Number*BytePerSector,file);
	rewind(file);
}

