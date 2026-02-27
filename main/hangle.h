#ifndef _HANGLE_
#define _HAnGLE_

void matrixPrint(uint32_t XPOS, uint32_t YPOS, char *pChar, uint32_t color, uint8_t position);
void matrixPrint2(uint32_t XPOS, uint32_t YPOS, char *pChar, uint32_t color, uint8_t position);

typedef enum _HG_MEM
{
    HG_MEM_WORKER = 0,
    HG_MEM_WORK_NAME,
    HG_MEM_MSG1,
    
} HG_MEM;
void drawHangle(uint32_t XPOS, uint32_t YPOS, char *pChar, uint32_t color, uint8_t position);
void hangle_mem_write(char *pChar, uint8_t position);
#endif
