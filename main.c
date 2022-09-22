#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include "fat.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
 /*******************************************************************************
 * Prototypes
 ******************************************************************************/
 /*!
 * @brief:Show directories
 *
 * @param1(*current):An entry
 * @param2(num):Count the number of entries
 *
 * @return:None
 */
void app_show_directory(FATFS_EntryNode_struct_t *current,uint32_t num);

/*!
 * @brief:Show error
 *
 * @param1(status):Status
 *
 * @return:None
 */
void show_error(uint32_t status);
 /*******************************************************************************
 * Variables
 ******************************************************************************/
 /*******************************************************************************
 * Code
 ******************************************************************************/

int main(void)
{
    FATFS_EntryNode_struct_t *current_directory = NULL;
    FATFS_EntryNode_struct_t *entry_list =NULL;/*List of entries*/
    uint8_t filename[50];/*Get data from user to check file exists*/
    uint32_t start_cluster = 0;/*The address of the starting cluster*/
    uint32_t n_o = 0;/*numerical order*/
    uint8_t *buffer;/*variable used to output data*/
    uint32_t chose = 0;/*Variables receive user choice*/
    FATFS_ErrorCode_t ret;
    do
    {
        printf("Open file:");
        fflush(stdin);
        gets(filename);
        ret = FATFS_init(filename);
    }while(ret == fatfs_init_fail);/*Check if file exists*/
    entry_list = (FATFS_EntryNode_struct_t*)malloc(sizeof(FATFS_EntryNode_struct_t));
    do
    {
        ret = FATFS_ReadDirectory(start_cluster, &entry_list);
        if(fatfs_read_dir_fail != ret)
        {
            current_directory = entry_list;
            app_show_directory(current_directory,n_o);// truyen entry num
            printf("Your choice:");
            scanf("%d",&chose);
            while(chose>1)
            {
                chose--;
                current_directory = current_directory->next;
            }
            if(current_directory->entry_data.attibute == 0x10)/*if it's a folder*/
            {
                start_cluster = current_directory->entry_data.cluster_source;
            }
            else
            {
                printf("--------------------------------------------------------------------------\n");
                buffer = (uint8_t*)malloc(512*14*sizeof(uint8_t));
                ret = FATFS_ReadFile(buffer, current_directory);
                if(fatfs_read_file_fail != ret)
                printf("\n");
                printf("--------------------------------------------------------------------------\n");
                free(buffer);
            }
        }
    }while(fatfs_success == ret);
    show_error(ret);
    free(entry_list);
    ret = FATFS_deinit();
    if(ret == fatfs_denit_fail)
    {
        show_error(ret);
    }
    return 0;
}

void app_show_directory(FATFS_EntryNode_struct_t *current, uint32_t no)
{
    FATFS_EntryNode_struct_t *pre = current;
    printf("--------------------------------------------------------------------------\n");
    printf("%2s%9s\t%5s\t%5s\t%14s%14s\n","No.","Name","ATTIBUTE","Time","Date","Size");
    while(pre != NULL) {
        no++;
        printf("%d|%0.9s",no,pre->entry_data.file_name);
        switch(pre->entry_data.attibute)
        {
            case 0x00:
                printf(".%0.6s\t",pre->entry_data.extention);
                printf("|File  |\t");
                break;
            case 0x01:
                printf("|Read Only|\t");
                break;
            case 0x02:
                printf("\t|Hidden|\t");
                break;
            case 0x04:
                printf("\t|System|\t");
                break;
            case 0x08:
                printf("\t|Volume Label|\t");
                break;
            case 0x10:
                printf("\t|Folder|\t");
                break;
            case 0x20:
                printf("\t|Archive|\t");
                break;
            case 0x40:
                printf("\t|Device|\t");
                break;
            default:
                printf("\t\t\t");
                break;
        }
        printf("%.2d:",pre->entry_data.hours);
        printf("%.2d:",pre->entry_data.minutes);
        printf("%.2d",pre->entry_data.seconds);
        printf("\t%.2d/",pre->entry_data.day);
        printf("%.2d/",pre->entry_data.month);
        printf("%.4d",pre->entry_data.year);
        printf("\t %d",pre->entry_data.file_size);
        printf("\n");
        pre=pre->next;
    }
    printf("--------------------------------------------------------------------------\n");
    printf("If your number choose > %d or < 0 then out program!!!\n",no);
}

void show_error(uint32_t status)
{
    switch(status)
    {
        case 1:
            printf("Error:Open file failed");
            break;
        case 2:
            printf("Error:Read directory failed");
            break;
        case 3:
            printf("Error:Read file failed");
            break;
        case 4:
            printf("Error:Close file failed");
            break;
    }
}
