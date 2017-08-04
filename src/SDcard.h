#ifndef _SDCARD_H_
#define _SDCARD_H_



#include <stdio.h>
#include <string.h>
extern "C" {
#include "driver/i2c.h"
#include "driver/i2s.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
}

bool SDcard_init();

#endif
