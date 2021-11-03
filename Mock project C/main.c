#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "FAT.h"
#include "HAL.h"

int main()
{
	int choose;
	uint8_t Check;
	BootSector_struct_t Sector;
	Entry_struct_t Entry;
	uint16_t CurrentSector;
	uint8_t CountEntry;

	Check=FAT_Init(&Sector);    /* Open file and read basic parameter of boot sector */
	if(Check==0)
	{
		printf("Error file!Check file!");
	}
	else
	{
		CurrentSector=Sector.FirstRoot;
    	FAT_PrintDir(CurrentSector,Sector.NumRoot,&CountEntry);    /* First: Show Root directory */

		while(1)
		{
			printf("\nPlease Choose:");
        	fflush(stdin);
			scanf("%d",&choose);
			if((choose<1)||(choose>CountEntry))                    /* Check select of user */
			{
				printf("Select wrong!\n");
			}
			else
			{
				FAT_ReadEntryUser(CurrentSector,Sector.NumRoot,&Entry,choose);    /* Read select of user */
        		if((Entry.Attribute&SUB_DIR_MASK)==SUB_DIR_MASK)
        		{
        			if(Entry.ClusterFirst==DIR_ROOT_MASK)
        			{
        				CurrentSector=Sector.FirstRoot;    /* Come back root directory */
					}
					else
					{
						/* Jump into Sub-directory */
						CurrentSector=(Sector.FirstData+((Entry.ClusterFirst-2)*Sector.SectorPerCluster));
					}
				}
				else
				{
					/* Print file */
					printf(".......................................................................\n");
					FAT_PrintFile(Sector.FirstData,Sector.FirstFAT,Entry.ClusterFirst,Sector.SectorPerCluster);
					printf("\n.......................................................................\n");
					printf(" \n\n\n");
				}
				printf(" \n");
				FAT_PrintDir(CurrentSector,Sector.NumRoot,&CountEntry);    /* Show current directory */
			}	
		}
	}

	FAT_De_Init();

	return 0;
}

