#ifndef _HEADER_FAT_H_
#define _HEADER_FAT_H_

typedef struct
{
    uint8_t file_name[8];
    uint8_t extention[3];
    uint8_t attibute;
    uint32_t file_size;
    uint16_t year;
    uint16_t month;
    uint16_t day;
    uint16_t hours;
    uint16_t minutes;
    uint16_t seconds;
    uint32_t address_file;
    uint32_t cluster_source;
}FATFS_EntryData_struct_t;

typedef struct _FATFS_EntryNode_struct_t
{
    FATFS_EntryData_struct_t entry_data;
    struct _FATFS_EntryNode_struct_t *next;
}FATFS_EntryNode_struct_t;

typedef enum
{
    fatfs_success=0,
    fatfs_init_fail=1,
    fatfs_read_dir_fail=2,
    fatfs_read_file_fail=3,
    fatfs_denit_fail=4
}FATFS_ErrorCode_t;/*Report a program error*/

/*!
 * @brief:Open a file and select the open file mode
 *
 * @param1(*file_path):Path to file to open
 *
 * @return:Check if the file can be opened
 */
FATFS_ErrorCode_t FATFS_init(const int8_t const *file_path);

/*!
 * @brief:Read Directory
 *
 * @param1(start_cluster):The address of the starting cluster
 * @param2(**entry_list):List of entries in main
 *
 * @return(status):Check if the folder is readable
 */
FATFS_ErrorCode_t FATFS_ReadDirectory(const uint32_t start_cluster, FATFS_EntryNode_struct_t **entry_list);

/*!
 * @brief:Read data of a file
 *
 * @param1(*buffer):Pointer to point the data area
 * @param2(*current):pointer of list
 *
 * @return:Check if the file is readable
 */
FATFS_ErrorCode_t FATFS_ReadFile(uint8_t *buffer , FATFS_EntryNode_struct_t *current);

/*!
 * @brief:Read data of a file
 *
 * @param1:None
 *
 * @return:Check if the file is readable
 */
FATFS_ErrorCode_t FATFS_deinit(void);

#endif /*_HEADER_FAT_H_*/
