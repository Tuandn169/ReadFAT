#ifndef __HAL_H_
#define __HAL_H_

typedef enum
{
    hal_success=0,
    hal_init_fail=1,/*File opening failed*/
    hal_update_sector_size_fail=2,/*Update check failed*/
    hal_close_fail=3/*close file failed*/
}HAL_Error_t;

/*!
 * @brief:Open a file and select the open file mode
 *
 * @param1(*file_path):Path to file to open
 *
 * @return:Check if the file can be opened
 */
HAL_Error_t HAL_Init(const int8_t const *file_path);

/*!
 * @brief:Read data and save the read data of a sector
 *
 * @param1(Index):The index of the sector to be read
 * @param2(*buff):Pointer is used to point to the array to store readable data
 *
 * @return(bytes_read):Returns the total number of elements successfully read
 */
int32_t HAL_ReadSector(uint32_t index, uint8_t *buff);

/*!
 * @brief:read data and save read data of multiple sectors
 *
 * @param1(Index):The index of the sector to be read
 * @param2(num):Num is the number of sectors to read
 * @param3(*buff):Pointer is used to point to the array to store readable data
 *
 * @return(bytes_read):Returns the total number of elements successfully read
 */
int32_t HAL_ReadMultiSector(uint32_t index, uint16_t num, uint8_t *buff);

/*!
 * @brief:Updating the size of the sector
 *
 * @param1(update_sector_size):Value of sector after update
 *
 * @return:Check if the sector has been updated
 */
HAL_Error_t HAL_UpdateSectorSize(uint16_t update_sector_size);

/*!
 * @brief:Close a file
 *
 * @param:None
 *
 * @return:Check if the file has been closed successfully
 */
HAL_Error_t HAL_DeInit(void);

#endif /*__HAL_H_*/
