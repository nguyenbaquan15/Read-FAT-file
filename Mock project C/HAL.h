#ifndef __HAL__
#define __HAL__

/* Extern varible BytePerSector */
extern uint16_t BytePerSector;

/* Open File */
uint8_t HAL_OpenFile(void);

/* Close File */
void HAL_CloseFile(void);

/* Read Byte per sector */
void HAL_ReadBytePerSector(void);

/* Read 1 sector */
void HAL_ReadSector(uint8_t Buffer[], uint16_t Sector);

/* Read multi sector */
void HAL_ReadMultiSector(uint8_t Buffer[], uint16_t Sector, uint8_t Number);

#endif /* __HAL__ */
