#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include "FAT.h"
#include "HAL.h"

/* Read value of an entry */
static void FAT_ReadEntry(uint8_t Entry[],Entry_struct_t *Parameter);

/* Get an entry from a buffer */
static void FAT_CutEntry(uint8_t Buffer[], uint8_t Entry[], uint8_t IndexEntry);

/* Get long file name and short file name from an entry */
static void FAT_ReadNameFile(uint8_t Entry[], char *FileName);

/* Joint name */
static void JoinName(char *s1,char *s2);

/* Calutator next cluster in FAT table */
static uint16_t FAT_ReadClusterNext(uint16_t FatSector,uint16_t ClusterFirst);

/* Print an entry */
static void FAT_PrintEntry(Entry_struct_t EntryValue, uint8_t Index, char *FileName);

uint8_t FAT_Init(BootSector_struct_t *Parameter)
{
	uint8_t check;
	check=HAL_OpenFile();
	HAL_ReadBytePerSector();
	FAT_ReadBootSector(Parameter);
	return check;

}

void FAT_De_Init(void)
{
	HAL_CloseFile();
}

void FAT_ReadBootSector(BootSector_struct_t *Parameter)
{
	uint8_t Buffer[BytePerSector];
	uint8_t SectorPerCluster;
	uint8_t NumberBootSector;
	uint8_t NumberTableFAT;
	uint16_t NumberEntryRoot;
	uint16_t NumberTotalSector;
	uint8_t SectorPerTableFAT;
	uint16_t NumberSectorRoot;

	HAL_ReadSector(Buffer,0);    /*  Read Boot sector */

	/* Read parameter in Boot sector */
	SectorPerCluster=ConvertHexaDec(Buffer,iSectorPerCluster,1);
	NumberBootSector=ConvertHexaDec(Buffer,iNumberBootSector,2);
	NumberTableFAT=ConvertHexaDec(Buffer,iNumberTableFAT,1);
	NumberEntryRoot=ConvertHexaDec(Buffer,iNumberEntryRoot,2);
	NumberTotalSector=ConvertHexaDec(Buffer,iNumberTotalSector,2);
	SectorPerTableFAT=ConvertHexaDec(Buffer,iSectorPerTableFAT,2);

	/* Calutator position sector of ares in FAT file */
	Parameter->FirstBoot=0;
	Parameter->FirstFAT=(Parameter->FirstBoot)+NumberBootSector;
	Parameter->FirstRoot=(Parameter->FirstFAT)+(NumberTableFAT*SectorPerTableFAT);
	NumberSectorRoot=(NumberEntryRoot*ENTRY_BYTE_NUM)/BytePerSector;
	Parameter->FirstData=(Parameter->FirstRoot)+NumberSectorRoot;

	Parameter->SectorPerCluster=SectorPerCluster;
	Parameter->NumRoot=NumberSectorRoot;

}

void FAT_PrintDir(uint16_t RootSector,uint8_t NumRootSector,uint8_t *CountEntry)
{
	uint8_t Buffer[BytePerSector*NumRootSector];
	uint8_t Entry[ENTRY_BYTE_NUM];
	uint8_t EntrySub=0;
	uint8_t EntryMain=0;
	uint8_t EntryEnd=0;
	uint8_t NextSector=0;
	uint8_t Index=0;
	uint8_t IndexEntry=1;
	char FileNameEntry[20];
	char LongFileName[100];
	Entry_struct_t EntryValue;
	uint8_t NumSubEntry;
	uint8_t IndexEntrySub;

	HAL_ReadMultiSector(Buffer,RootSector,NumRootSector);
	printf("Name\t\t\t\tDateModified\t\tType\t\tSize\n");

	while((EntryEnd==0))
	{
		FAT_CutEntry(Buffer,Entry,IndexEntry);                           /* Cut an entry in buffer */

		if((Entry[0x0B]==LONG_FILE_NAME)&&(Entry[0x00]!=EMPTY_ENTRY))    /* Handle Sub entry */
		{
			NumSubEntry=Entry[0x00]&0x1F;                                /* Calutator number Sub-entry */
			IndexEntrySub=IndexEntry+(NumSubEntry-1);
			while(IndexEntrySub>=IndexEntry)                             /* Read long file name in many Sub entry */
			{
				FAT_CutEntry(Buffer,Entry,IndexEntrySub);
				FAT_ReadNameFile(Entry,FileNameEntry);
				JoinName(LongFileName,FileNameEntry);
				IndexEntrySub--;
			}

			IndexEntry=IndexEntry+(NumSubEntry-1);
			EntrySub=1;
			if(NumSubEntry==1)
			{
				strcpy(LongFileName,FileNameEntry);                      /* Read long file name 1 Sub entry */
			}
		}

		/* After read long file name in Sub-entry, next read value in main entry */
		else if((Entry[0x0B]!=LONG_FILE_NAME)&&(Entry[0x00]!=EMPTY_ENTRY)&&(EntrySub==1))
		{
			EntryMain=1;
			EntrySub=0;
			FAT_ReadEntry(Entry,&EntryValue);
			Index++;
			FAT_PrintEntry(EntryValue,Index,LongFileName);              /* Print an entry in directory */
			strcpy(LongFileName,"");
		}

		/* Read short name and value in main entry */
		else if((Entry[0x0B]!=LONG_FILE_NAME)&&(Entry[0x00]!=EMPTY_ENTRY)&&(EntrySub==0))
		{
			FAT_ReadNameFile(Entry,FileNameEntry);
			EntryMain=1;
			FAT_ReadEntry(Entry,&EntryValue);
			Index++;
			FAT_PrintEntry(EntryValue,Index,FileNameEntry);
		}
		else
		{
			EntryEnd=1;              /* Entry end in root directory */
		}

		*CountEntry=Index;           /* Show index for user select */
		IndexEntry++;
	}
}

void FAT_ReadEntry(uint8_t Entry[],Entry_struct_t *Parameter)
{
	uint8_t ClusterFirst[4];

	/* Read 4 byte to calutator ClusterFirst in FAT table */
	ClusterFirst[0]=Entry[0x1A];
	ClusterFirst[1]=Entry[0x1B];
	ClusterFirst[2]=Entry[0x14];
	ClusterFirst[3]=Entry[0x15];
	Parameter->ClusterFirst=ConvertHexaDec(ClusterFirst,0,4);

	Parameter->Attribute=ConvertHexaDec(Entry,0x0B,1);              /* Read attribute: folder or file */
	Parameter->SizeFile=ConvertHexaDec(Entry,0x1C,4);               /* Read size */
	Parameter->DateModified=ConvertHexaDec(Entry,0x18,2);           /* Read time:day, month, year */
	Parameter->Hour=ConvertHexaDec(Entry,0x0E,2);                   /* Read time: hour, minute, second */
}

void FAT_CutEntry(uint8_t Buffer[], uint8_t Entry[], uint8_t IndexEntry)
{
	uint8_t i;
	uint16_t FirstIndex=0;

	FirstIndex=(IndexEntry-1)*ENTRY_BYTE_NUM;

	for(i=0;i<32;i++)
	{
		Entry[i]=Buffer[FirstIndex];
		FirstIndex++;
	}
}

void FAT_PrintEntry(Entry_struct_t EntryValue, uint8_t Index, char *FileName)
{
	char Attribute[10];
	uint8_t Day;
	uint8_t Month;
	uint16_t Year;
	uint8_t Hour;
	uint8_t Minute;
	uint8_t Second;
	
	if(EntryValue.Attribute==SUB_DIR_MASK)
	{
		strcpy(Attribute,"Folder");
	}
	else
	{
		strcpy(Attribute,"File");
	}

	/* Calutator time */
	Day=EntryValue.DateModified & DAY_MASK;
	Month=(EntryValue.DateModified & MONTH_MASK)>>5;
	Year=1980+((EntryValue.DateModified & YEAR_MASK)>>9);
	Second=(EntryValue.Hour & SECOND_MASK);
	Minute=(EntryValue.Hour & MINUTE_MASK)>>5;
	Hour=(EntryValue.Hour & HOUR_MASK)>>11;

	if(EntryValue.Attribute==SUB_DIR_MASK)
	{
		printf("%d.%s\t\t\t%d/%d/%d\t\t%s\t\t%d\n"\
	    ,Index,FileName,Day,Month,Year,Attribute,EntryValue.SizeFile);
	}
	else
	{
		printf("%d.%s\t\t\t%d/%d/%d %d:%d:%d\t%s\t\t%d\n"\
	,Index,FileName,Day,Month,Year,Hour,Minute,Second,Attribute,EntryValue.SizeFile);
	}

}

void FAT_ReadNameFile(uint8_t Entry[], char *FileName)
{
	uint8_t i;
	uint8_t j;

	if(Entry[0x0B]==LONG_FILE_NAME)
	{
		for(i=0,j=1;j<11;i++,j+=2)               /* Read 10 byte of long file name */
		{
			FileName[i]=Entry[j];
		}

		for(i=5,j=0x0E;j<0x1A;i++,j+=2)          /* Read 12 byte next of long file name */
		{
			FileName[i]=Entry[j];
		}
	}
	else
	{
		for(i=0;i<11;i++)
		{
			
			FileName[i]=Entry[i];                /* Read short name */
		}
	}
}

void JoinName(char *s1,char *s2)
{
	int i,j;

	for(i=strlen(s1),j=0;i<strlen(s1)+strlen(s2);i++,j++)    /* Joint 2 string */
	{
		*(s1+i)=*(s2+j);
	}
}

void FAT_ReadEntryUser(uint16_t RootSector, uint8_t NumRootSector, Entry_struct_t *Parameter, uint8_t Choose)
{
	uint8_t Buffer[BytePerSector*NumRootSector];
	uint8_t Entry[ENTRY_BYTE_NUM];
	uint8_t Index=0;
	uint8_t IndexEntry=1;
	uint8_t EntryEnd=0;

	HAL_ReadMultiSector(Buffer,RootSector,NumRootSector);
	while(EntryEnd==0)
	{
		FAT_CutEntry(Buffer,Entry,IndexEntry);
		if((Entry[0x0B]==LONG_FILE_NAME)&&((Entry[0x00]!=EMPTY_ENTRY)))
		{
			/* do nothing */
		}
		else if((Entry[0x0B]!=LONG_FILE_NAME)&&((Entry[0x00]!=EMPTY_ENTRY)))
		{
			Index++;
			if(Choose==Index)                        /* Check entry with select user */
			{
				FAT_ReadEntry(Entry,Parameter);
			}
			else
			{
				/* do nothing */
			}
		}
		else
		{
			EntryEnd=1;
		}
		IndexEntry++;
	}
}

void FAT_PrintFile(uint16_t DataSector, uint16_t FatSector, uint16_t ClusterFirst,uint8_t SectorPerCluster)
{
	uint8_t Buffer[BytePerSector*SectorPerCluster];
	uint16_t Next;
	uint16_t i=0;

	Next=ClusterFirst;
	while(Next!=0xFFF)
	{
		HAL_ReadMultiSector(Buffer,DataSector+(Next-2)*SectorPerCluster,SectorPerCluster);
		while(i<BytePerSector)
		{
			if(Buffer[i]!=0x07)
			{
				printf("%c",Buffer[i]);
			}
			else
			{
				/* do nothing */
			}
			i++;
			
		}
		i=0;
		Next=FAT_ReadClusterNext(FatSector,Next);

	}
}

uint16_t FAT_ReadClusterNext(uint16_t FatSector,uint16_t ClusterFirst)
{
	uint16_t ByteFat;
	uint16_t Value;
	uint16_t Temp1;
	uint16_t Temp2;
	uint8_t Buffer[512];
	uint8_t Index;
	uint16_t CurrentByte;

	ByteFat= ClusterFirst*SizeElementFat;             /* Calutator position byte clusterFirst in FAT table */

	Index=ByteFat/BytePerSector;                      /* Calutator position byte in which sector */

	HAL_ReadSector(Buffer,FatSector+Index);           /* Read sector contain clusterFirst */
	CurrentByte=ByteFat-(Index*BytePerSector);

	if(ClusterFirst%2==0)
	{
		Temp1=(Buffer[CurrentByte+1]&0x0F);
		Temp2=Buffer[CurrentByte];
		Value=(Temp1*256)+Temp2;
	}
	else
	{
		Temp1=(Buffer[CurrentByte]&(0xF0));
		Temp2=Buffer[CurrentByte+1];
		Value=(Temp2*16)+(Temp1/16);
	}

	return Value;
}

uint16_t ConvertHexaDec(uint8_t Buffer[],uint8_t Position, uint8_t Number)
{
	uint16_t result=0;
	uint8_t i;
	for(i=0;i<Number;i++)
	{
		result=result+(Buffer[Position+i]*pow(256,i));    /* Convert hexa to decimal */
	}

	return result;
}
