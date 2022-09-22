#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include"hal.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define HAL_DEFAUL_SECTOR_SIZE 512U 
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint16_t s_sector_size = HAL_DEFAUL_SECTOR_SIZE;/*number of bytes of a sector*/ 
static FILE *sp_file;/*Save the pointed file address*/
/*******************************************************************************
 * Code
 ******************************************************************************/
HAL_Error_t HAL_Init(const int8_t const *file_path)
{
    HAL_Error_t status = hal_success;/*Function statusism variable*/
    
    sp_file = fopen(file_path,"rb");
    if(NULL == sp_file)
    {
        status = hal_init_fail;
    }
    
    return status;/*Return status after processing*/
}

int32_t HAL_ReadSector(uint32_t index, uint8_t *buff)
{
    int32_t bytes_read = 0;/*Read byte number variable variable*/
    
    if(NULL != sp_file)
    {
        if(0 == fseek(sp_file,index*s_sector_size,SEEK_SET));
        {
            bytes_read = fread(buff,1,s_sector_size,sp_file);
        }
    }
    
    return bytes_read;/*Total number of bytes read successfully*/
}

int32_t HAL_ReadMultiSector(uint32_t index, uint16_t num, uint8_t *buff)
{
    int32_t bytes_read = 0;
    
    if(NULL != sp_file)
    {
        if(0 == fseek(sp_file, index*s_sector_size,SEEK_SET));
        {
            bytes_read = fread(buff,1,s_sector_size*num,sp_file);
        }
    }
    
    return bytes_read;
}

HAL_Error_t HAL_UpdateSectorSize(uint16_t update_sector_size)
{
    HAL_Error_t status = hal_success;
    
    if(0 == (update_sector_size % 512))
    {
        s_sector_size = update_sector_size;
        status = hal_success;
    }
    else
    {
        status = hal_update_sector_size_fail;
    }
    
    return status;
}

HAL_Error_t HAL_DeInit(void)
{
    HAL_Error_t status = hal_success;
    
    status = fclose(sp_file);/*Close file*/
    if(0 != status)
    {
        status = hal_close_fail;
    }
    
    return status;
}
