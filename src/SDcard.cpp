#include "SDcard.h"
#include <stdio.h>
#include <string.h>



void SDcard_init()
{
    
    sdmmc_card_t* card;
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5
    };
    esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
}

