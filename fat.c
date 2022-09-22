#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdint.h>
#include "hal.h"
#include "fat.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FATFS_CONVERT_2_BYTES(X) (((*((X) + 1)) << 8)|(*(X)))
#define FATFS_CONVERT_4_BYTES(X) (((*((X) + 3))<<24)|((*((X) + 2)) << 16)|((*((X) + 1)) << 8)|(*(X)))
#define CONVERT_2BYTE(a,b)    (((b) << 8) | (a))
#define BYTE_PER_SECTOR            0x0BU
#define SECTOR_PER_CLUSTER         0x0DU
#define NUMBER_OF_FAT              0x10U
#define NUMBER_ENTRIES             0x11U
#define TOTAL_LOGICAL_SECTORS      0x13U
#define FAT_START                  0x0EU
#define FAT_SIZE_INDEX             0x16U
#define ATTIBLE                    0x0BU
#define CLUSTER                    0x1AU
#define TIME                       0x16U
#define FATFS_DATE                 0x18U
#define FATFS_FILE_SIZE            0x1CU
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
typedef struct
{
    uint32_t byte_sector;
    uint32_t entries_number;
    uint32_t sector_per_cluster;
    uint32_t fat_size;
    uint32_t start_root;
    uint16_t root_size;
    uint16_t start_fat;
    uint16_t start_data;
    uint32_t total_logical_sectors;
    uint16_t typy_of_fat;
    uint32_t end_of_file; 
}FATFS_Infor_struct_t;

/*!
 * @brief:Read and process the boot data area
 *
 * @param:None
 *
 * @return:Data is updated into struct s_fatfile_infor
 */
static void FATFS_HandleBootSector(void);

/*!
 * @brief:Delete old entries in the entry list
 *
 * @param:None
 *
 * @return:None
 */
static void DeleteEntryList(void);

/*!
 * @brief:Get data and add entries to list
 *
 * @param(*entry):The pointer points to the data area
 *
 * @return:Create more entries to the entry list in fat
 */
static void FATFS_AddEntryToTheList(uint8_t *entry);

/*!
 * @brief:Find the next cluster
 *
 * @param1(cluster_source):cluster can read from root area
 * @param2(*buffer):The pointer points to the data area
 *
 * @return:Create more entries to the entry list in fat
 */
static uint32_t FATFS_cluster_next(uint32_t cluster_source, uint8_t *buffer);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static FATFS_EntryNode_struct_t *sp_EntryList;/*List of entries in the fat . area*/
static FATFS_Infor_struct_t s_fatfile_infor;/*Data of the boot sector*/

/*******************************************************************************
 * Code
 ******************************************************************************/
 
static void FATFS_HandleBootSector(void)
{
    uint8_t *buff = NULL;/*The pointer points to the data location*/
    uint16_t sector_size = 0;/*Variable receiving updated value*/
    uint32_t total_cluster;/*Maximum number of clusters of managed fat tables*/
    
    buff = (uint8_t*)malloc(512);
    HAL_ReadSector(0,buff);/*Read data of area 0*/
    sector_size = FATFS_CONVERT_2_BYTES(&buff[BYTE_PER_SECTOR]);
    if(512 != sector_size )
    {
        HAL_UpdateSectorSize(sector_size);
    }
    /*Information of the boot area*/
    s_fatfile_infor.byte_sector = sector_size;/*Number of bytes of a sector*/
    s_fatfile_infor.sector_per_cluster = buff[SECTOR_PER_CLUSTER];
    s_fatfile_infor.fat_size = FATFS_CONVERT_2_BYTES(&buff[FAT_SIZE_INDEX]);
    s_fatfile_infor.entries_number = FATFS_CONVERT_2_BYTES(&buff[NUMBER_ENTRIES]);
    s_fatfile_infor.start_root = 1 + buff[NUMBER_OF_FAT]*s_fatfile_infor.fat_size;
    s_fatfile_infor.root_size = (s_fatfile_infor.entries_number * 32)/sector_size;
    s_fatfile_infor.start_data = s_fatfile_infor.start_root + s_fatfile_infor.root_size;
    s_fatfile_infor.start_fat = FATFS_CONVERT_2_BYTES(&buff[FAT_START]);
    s_fatfile_infor.total_logical_sectors = FATFS_CONVERT_2_BYTES(&buff[TOTAL_LOGICAL_SECTORS]);
    /*Test Fat12 or Fat 16*/
    total_cluster = ((s_fatfile_infor.total_logical_sectors - s_fatfile_infor.start_data)\
                                                 /s_fatfile_infor.sector_per_cluster)-2;
    if(total_cluster<4087)
    {
        s_fatfile_infor.typy_of_fat = 1;/*Assign FAT12 table to 1*/
        s_fatfile_infor.end_of_file = 0xFFFU;/*the end of fat12*/
    }
    else if((total_cluster>4087) && (total_cluster<65562))
    {
        s_fatfile_infor.typy_of_fat = 2;/*Assign FAT16 table to 2*/
        s_fatfile_infor.end_of_file = 0xFFFFU;/*the end of fat16*/
    }
    free(buff);
}

static void DeleteEntryList(void)
{
    FATFS_EntryNode_struct_t *current = sp_EntryList;
    FATFS_EntryNode_struct_t *previous = NULL;
    
    while(NULL != current)/*Browse the list*/
    {
        previous = current->next;
        free(current);/*Delete an entry*/
        current = previous;
    }
    sp_EntryList = NULL;
}

static void FATFS_AddEntryToTheList(uint8_t *entry)
{
    uint8_t i = 0;/*The counter variable of entry*/
    uint8_t i_extention = 0;/*Count of extention*/
    uint16_t entry_time = 0;/*time analysis*/
    FATFS_EntryNode_struct_t *previous = NULL;/*previous entry*/
    
    if((entry[0x0B] != 0x0F) && (entry != NULL))
    {
        FATFS_EntryNode_struct_t *currentNode = \
        (FATFS_EntryNode_struct_t*)malloc(sizeof(FATFS_EntryNode_struct_t));
        for(i=0;i < 8;i++)
        {
            currentNode -> entry_data.file_name[i] = entry[i];
        }
        currentNode->entry_data.attibute=entry[ATTIBLE];
        for(i=8;i < 11;i ++)
        {
            currentNode->entry_data.extention[i_extention]=entry[i];
            i_extention++;
        }
        /*Time*/
        entry_time = FATFS_CONVERT_2_BYTES(&entry[TIME]);
        currentNode->entry_data.hours=(entry_time>>11);
        currentNode->entry_data.minutes=((entry_time&0x3E0)>>5);
        currentNode->entry_data.seconds=((entry_time)&0x1F);
        /*Date*/
        entry_time = FATFS_CONVERT_2_BYTES(&entry[FATFS_DATE]);
        currentNode->entry_data.day=((entry_time) & 0x1F);
        currentNode->entry_data.month=((entry_time&0x1E0)>>5);
        currentNode->entry_data.year = (entry_time>>9)+1980;
        currentNode->entry_data.file_size = FATFS_CONVERT_4_BYTES(&entry[FATFS_FILE_SIZE]);
        if((entry[CLUSTER]==0x00)&&(entry[CLUSTER+1]==0x00))
        {
            currentNode->entry_data.address_file=s_fatfile_infor.start_root;
        }
        else
        {
            currentNode->entry_data.address_file=FATFS_CONVERT_2_BYTES(&entry[CLUSTER])\
                        *s_fatfile_infor.sector_per_cluster+s_fatfile_infor.start_data;
        }
        currentNode->entry_data.cluster_source=FATFS_CONVERT_2_BYTES(&entry[CLUSTER]);
        currentNode->next=NULL;
        /*Add new entry at the end of the entry list*/
        if(NULL == sp_EntryList)
        {
            sp_EntryList = currentNode;
        }
        else
        {
            previous = sp_EntryList;
            while(NULL != previous->next)
            {
                previous=previous->next;
            }
            previous->next = currentNode;
        }
    }
}

static uint32_t FATFS_cluster_next(uint32_t cluster_source, uint8_t *buffer)
{
    uint8_t firts_cluster=0x00;/*front cluster*/
    uint8_t second_cluster=0x00;/*adjacent cluster*/
    uint8_t index_sector=0x00;/*the index of sector*/
    uint16_t cluster_next=0x00;
    uint16_t byte_offset=0;/*number of bytes in an offset*/
    uint16_t offset=0;
    
    byte_offset = (cluster_source*3)/2+s_fatfile_infor.byte_sector;
    index_sector = byte_offset/s_fatfile_infor.byte_sector;
    offset = byte_offset%s_fatfile_infor.byte_sector;
    HAL_ReadSector(index_sector,buffer);
    if((s_fatfile_infor.byte_sector-1) == offset)
    /*Check if it's in the last position in the sector*/
    {
        firts_cluster = buffer[offset];/*Assign first_cluster to the last value of sector*/
        index_sector++;/*Jump to next sector*/
        HAL_ReadSector(index_sector,buffer);
        second_cluster = buffer[0];/*Assign second_cluster to the first value of the next sector*/
    }
    else/*If the first and second positions are both within a sec tor*/
    {
        firts_cluster = buffer[offset];
        second_cluster = buffer[offset+1];
    }
    if(s_fatfile_infor.typy_of_fat == 1)/*if it's fat 12*/
    {
        if(0 == (cluster_source%2))/*cluster_source is an even number*/
        {
            cluster_next = ((second_cluster)&0x0F) << 8 | firts_cluster;
        }
        else/*cluster_source is odd*/
        {
            cluster_next = ((second_cluster)<<4)|((firts_cluster)>>4);
        }
    }
    else if(2 == s_fatfile_infor.typy_of_fat)/*if it's fat 16*/
    {
        cluster_next = CONVERT_2BYTE(firts_cluster,second_cluster);
    }
    
    return cluster_next;/*Next Cluster*/
}

FATFS_ErrorCode_t FATFS_init(const int8_t *const file_path)
{
    FATFS_ErrorCode_t status = fatfs_success;
    HAL_Error_t ret = hal_success;
    
    ret = HAL_Init(file_path);
    if(ret == hal_success)
    {
        status = fatfs_success;
        FATFS_HandleBootSector();
        sp_EntryList = (FATFS_EntryNode_struct_t*)malloc(sizeof(FATFS_EntryNode_struct_t));
        sp_EntryList = NULL;
    }
    else
    {
        status = fatfs_init_fail;
    }
    
    return status;
}

FATFS_ErrorCode_t FATFS_ReadDirectory(const uint32_t const start_cluster, FATFS_EntryNode_struct_t **entry_list)
{
    uint32_t ret = 0;/*Get return value*/
    FATFS_ErrorCode_t status = fatfs_success;
    uint8_t *buffer;
    uint32_t i = 0;
    uint32_t first_cluster = 0;
    DeleteEntryList();
    if(start_cluster == 0)
    {
        first_cluster = s_fatfile_infor.start_root;/*Root sector address*/
    }
    else
    {
        first_cluster = (start_cluster-2)*s_fatfile_infor.sector_per_cluster+s_fatfile_infor.start_data;
    }
    buffer=(uint8_t*)malloc(s_fatfile_infor.byte_sector*s_fatfile_infor.entries_number*sizeof(uint8_t));
    ret = HAL_ReadMultiSector(first_cluster,s_fatfile_infor.root_size,buffer);
    if(ret != 0 )/*If I read it without error*/
    {
        while(0x00U != buffer[i])/*If first value is 0x00 exit while*/
        {
            FATFS_AddEntryToTheList(&buffer[i]);
            i = i + 32;
        }
        *entry_list = sp_EntryList;
        free(buffer);
    }
    else
    {
        status = fatfs_read_dir_fail;
    }
    
    return status;
}

FATFS_ErrorCode_t FATFS_ReadFile(uint8_t *buffer , FATFS_EntryNode_struct_t *current)
{
    uint32_t i;
    uint32_t current_next;
    uint16_t clusterToSector;
    uint16_t currentAddress;
    uint32_t index_sector;/*the variable count the number of sectors on the cluster*/
    uint32_t end_of_file = 0;
    FATFS_ErrorCode_t status = fatfs_success;
    uint32_t ret = 0;

    if(s_fatfile_infor.typy_of_fat == 1)
    {
        end_of_file = s_fatfile_infor.end_of_file;
    }
    else if(s_fatfile_infor.typy_of_fat == 2)
    {
        end_of_file = s_fatfile_infor.end_of_file;
    }
    currentAddress=current->entry_data.address_file;
    current_next=current->entry_data.cluster_source;
    for(index_sector=0; index_sector<s_fatfile_infor.sector_per_cluster; index_sector++)
    {
        ret = HAL_ReadSector(currentAddress+index_sector,buffer);
        if(0 != ret)
        {
            for(i=0; i<512; i++)
            {
                if(0x00 != buffer[i] )
                {
                    printf("%c",buffer[i]);
                }
                else if(0x00 == buffer[i])
                {
                    break;
                }
            }
        }
        else
        {
            status = fatfs_read_file_fail;
        }
    }
    while(end_of_file != current_next)
    {
        current_next = FATFS_cluster_next(current_next,buffer);
        if(end_of_file != current_next)
        {
            clusterToSector=(current_next-2)*s_fatfile_infor.sector_per_cluster+s_fatfile_infor.start_data;
            for(index_sector=0; index_sector<s_fatfile_infor.sector_per_cluster; index_sector++)
            {
                ret = HAL_ReadSector(clusterToSector+index_sector,buffer);
                if(ret != 0 )
                {
                    for(i=0; i<s_fatfile_infor.byte_sector; i++)
                    {
                        if(0x00U != buffer[i])
                        {
                            printf("%c",buffer[i]);
                        }
                    }
                }
                else
                {
                    status = fatfs_read_file_fail;
                }
            }
        }
    }
    
    return status;
}

FATFS_ErrorCode_t FATFS_deinit(void)
{
    FATFS_ErrorCode_t status = fatfs_success;
    
    if(hal_success == HAL_DeInit())
    {
        free(sp_EntryList);
    }
    else
    {
        status = fatfs_denit_fail;
    }   
    return status;
}
