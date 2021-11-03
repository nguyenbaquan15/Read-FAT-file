#ifndef __FAT__
#define __FAT__

/* Parameter in boot sector */
#define iSectorPerCluster    0x0D
#define iNumberBootSector    0x0E
#define iNumberTableFAT      0x10
#define iNumberEntryRoot     0x11
#define iNumberTotalSector   0x13
#define iSectorPerTableFAT   0x16

/* Parameter other */
#define SizeElementFat       1.5
#define DIR_ROOT_MASK        0x00
#define SUB_DIR_MASK         0x10
#define LONG_FILE_NAME       0x0F
#define FILE_ATTRIBUTE       0x00
#define EMPTY_ENTRY          0x00
#define EMPTY_FILE           0x00
#define ENTRY_BYTE_NUM       32
#define END_CLUSTER          0xFFF

#define DAY_MASK             0x1F
#define MONTH_MASK           0x1E0
#define YEAR_MASK            0xFE00
#define HOUR_MASK            0xF800
#define MINUTE_MASK          0x7E0
#define SECOND_MASK          0x1F

/* Struct used to save value of bootsector's parameter */
typedef struct BootSector
{
	char OemName[20];
	uint8_t FirstBoot;
	uint16_t FirstFAT;
	uint16_t FirstRoot;
	uint16_t FirstData;
	uint8_t SectorPerCluster;
	uint8_t NumRoot;

} BootSector_struct_t;

/* Struct used to save value of entry's parameter */
typedef struct Entry
{
	char FileName[20];
	uint16_t ClusterFirst;
	uint8_t Attribute;
	uint32_t SizeFile;
	uint16_t DateModified;
	uint16_t Hour;

} Entry_struct_t;

/* Open file and read boot sector */
uint8_t FAT_Init(BootSector_struct_t *Parameter);

/* End read file */
void FAT_De_Init(void);

/* Read parameter of Boot sector */
void FAT_ReadBootSector(BootSector_struct_t *Parameter);

/* Print Directory */
void FAT_PrintDir(uint16_t RootSector,uint8_t NumRootSector,uint8_t *CountEntry);

/* Read Entry that user select */
void FAT_ReadEntryUser(uint16_t RootSector, uint8_t NumRootSector, Entry_struct_t *Parameter, uint8_t Choose);

/* Print File */
void FAT_PrintFile(uint16_t DataSector, uint16_t FatSector, uint16_t ClusterFirst,uint8_t Number);

/* Convert Hexa to Dec */
uint16_t ConvertHexaDec(uint8_t Buffer[],uint8_t Position, uint8_t Number);

#endif /* __FAT__ */
