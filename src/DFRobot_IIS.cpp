/*!
 * @file DFRobot_IIS.h
 * @brief DFRobot's IIS Module
 * @n IIS Module for playMusic and recordSound
 *
 * @copyright	[DFRobot](http://www.dfrobot.com), 2017
 * @copyright	GNU Lesser General Public License
 *
 * @author [Zhangjiawei](jiawei.zhang@dfrobot.com)
 * @version  V1.0
 * @date  2017-8-1
 */

#include "DFRobot_IIS.h"
#include "vfs_api.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include "esp_err.h"
#include "esp_log.h"

void DFRobot_IIS::init(int mode)
{
	if(mode==0){
    SDcard_init();
	I2C_Master_Init();
	}
	else if(mode==1){
    SDcard_init();
	}
	else 
    printf("No such mode ");
}

int DFRobot_IIS::playMusic(const char *filename)
{
	I2C_Setup_WAU8822_play();
    HANDLE_WAV wav = (HANDLE_WAV)calloc(1, sizeof(struct WAV));
    if (wav == NULL) {
      printf("playMusic(): Unable to allocate WAV struct.\n");
      goto error;
    }  
    wav->fp = fopen(filename, "rb");
    if (wav->fp == NULL) {
      printf("playMusic(): Unable to open wav file. %s\n", filename);
      goto error;
    }
    if (fread(&(wav->header.riffType), 1, 4, wav->fp) != 4) {
      printf("playMusic(): couldn't read RIFF_ID\n");
      goto error;  /* bad error "couldn't read RIFF_ID" */
    }
    if (strncmp("RIFF", wav->header.riffType, 4)) {
      printf("playMusic(): RIFF descriptor not found.\n") ;
      goto error;
    }
    fread(&(wav->header.riffSize), 4, 1, wav->fp);
    if (fread(&wav->header.waveType, 1, 4, wav->fp) !=4) {
      printf("playMusic(): couldn't read format\n");
      goto error;  /* bad error "couldn't read format" */
    }
    if (strncmp("WAVE", wav->header.waveType, 4)) {
      printf("playMusic(): WAVE chunk ID not found.\n") ;
      goto error;
    }
    if (fread(&(wav->header.formatType), 1, 4, wav->fp) != 4) {
      printf("playMusic(): couldn't read format_ID\n");
      goto error;  /* bad error "couldn't read format_ID" */
    }
    if (strncmp("fmt", wav->header.formatType, 3)) {
      printf("playMusic(): fmt chunk format not found.\n") ;
     goto error;
    }
    fread(&wav->header.formatSize, 4, 1, wav->fp);   
    fread(&(wav->header.compressionCode), 2, 1, wav->fp);
    fread(&(wav->header.numChannels), 2, 1, wav->fp);
    fread(&(wav->header.sampleRate), 4, 1, wav->fp);
    fread(&(wav->header.bytesPerSecond), 4, 1, wav->fp);
    fread(&(wav->header.blockAlign), 2, 1, wav->fp);
    fread(&(wav->header.bitsPerSample), 2, 1, wav->fp);
    while(1){
        if (fread(&wav->header.dataType1, 1, 1, wav->fp) != 1) {
        printf("playMusic(): Unable to read data chunk ID.\n");
        free(wav);
        goto error;
        }
        if (strncmp("d", wav->header.dataType1, 1) == 0) {
            fread(&wav->header.dataType2, 3, 1, wav->fp);
            if (strncmp("ata", wav->header.dataType2, 3) == 0){
                fread(&(wav->header.dataSize),4,1,wav->fp);
                break;
		    }
        }
	}
	printf("wav->header.sampleRate=%d \n",wav->header.sampleRate);
	printf("wav->header.bitsPerSample=%d \n",wav->header.bitsPerSample);
	I2S_MCLK_Init(wav->header.sampleRate);
    I2S_Master_Init(wav->header.sampleRate ,wav->header.bitsPerSample);
    i2s_set_sample_rates(I2S_NUM_0, wav->header.sampleRate);
	printf("set clock");
    while(fread(&wav->header.test, 1 , 800 , wav->fp)){
        char *buf=(char *)&wav->header.test;
        int bytes_left=800,bytes_written = 0;
        while(bytes_left > 0) {
            bytes_written = i2s_write_bytes(I2S_NUM_0 , buf , bytes_left , 0);
            bytes_left -= bytes_written;
			buf += bytes_written;
			printf("bytes_written=%d \n",bytes_written);
        }   
    }
	printf("playMusic over");
    i2s_stop(I2S_NUM_0);
	fclose(wav->fp);
	esp_vfs_fat_sdmmc_unmount();
    return 1;
error:
    if (wav) {
      if (wav->fp) {
        fclose(wav->fp);
        wav->fp = NULL;
      }
      free(wav);
    }
    return -1;
}

int DFRobot_IIS::recordSound(const char *outputFilename, uint32_t samplerate, i2s_channel_t numchannels, i2s_bits_per_sample_t bitspersample)
{
  I2C_Setup_WAU8822_record();
  HANDLE_WAV wav = (HANDLE_WAV)calloc(1, sizeof(struct WAV));
  unsigned int size = 0;
  if (wav == NULL) {
    printf("recordSound(): Unable to allocate WAV struct.\n");
    return 1;
  }
  if (bitspersample != 16 && bitspersample != 24 && bitspersample != 32){
      printf("recordSound(): Invalid argument (bitsPerSample).\n");
      return 1;
  }
  wav->fp = fopen(outputFilename, "wb");
  if (wav->fp == NULL){
    printf("recordSound(): unable to create file %s\n", outputFilename);
    return 1;
  }
  strcpy(wav->header.riffType, "RIFF");
  wav->header.riffSize = 0;  
  strcpy(wav->header.waveType, "WAVE");
  strcpy(wav->header.formatType, "fmt ");
  wav->header.formatSize      =0x00000010;
  fwrite(&wav->header.riffType, 1, 20 , wav->fp);
  wav->header.compressionCode = 1;
  fwrite(&wav->header.compressionCode, 1 , 2 , wav->fp);
  wav->header.numChannels     = numchannels;
  fwrite(&wav->header.numChannels, 1, 2, wav->fp);
  wav->header.sampleRate      = samplerate;
  fwrite(&wav->header.sampleRate, 1, 4 , wav->fp);
  wav->header.blockAlign      = (short)(numchannels *(bitspersample >> 3));
  wav->header.bytesPerSecond  = (samplerate)*(wav->header.blockAlign);
  fwrite(&wav->header.bytesPerSecond, 1, 4 , wav->fp);
  fwrite(&wav->header.blockAlign, 1, 2 , wav->fp);
  wav->header.bitsPerSample   = bitspersample;
  fwrite(&wav->header.bitsPerSample, 1, 2 , wav->fp);
  strcpy(wav->header.dataType1, "d");
  strcpy(wav->header.dataType2, "ata");
  wav->header.dataSize        = 0;
  fwrite(&wav->header.dataType1 ,1,8, wav->fp);
  int bytes_written = 0;
  char *buf=(char *)&wav->header.test;
  I2S_MCLK_Init(samplerate);
  I2S_Slave_Init(samplerate,bitspersample);
  while((gpio_input_get()&0x00010000)) {
    bytes_written = i2s_read_bytes(I2S_NUM_0 ,buf, 800 , 100);
	wav->header.dataSize+=fwrite(buf, 1, bytes_written , wav->fp);
	printf("bytes_written=%d \n",bytes_written);
  }  
  printf("record done \n");
  wav->header.riffSize =wav->header.dataSize+44;
  fseek(wav->fp,4,0);
  fwrite(&wav->header.riffSize, 1,4, wav->fp);
  fseek(wav->fp,40,0);
  fwrite(&wav->header.dataSize, 1,4, wav->fp);
  fclose(wav->fp);
  esp_vfs_fat_sdmmc_unmount();
  printf("save data \n");
  return 0;
}

void I2C_Master_Init()
{
    i2c_port_t i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode             = I2C_MODE_MASTER;
    conf.sda_io_num       = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en    = GPIO_PULLUP_ENABLE;
    conf.scl_io_num       = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en    = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ ;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode,
                       I2C_MASTER_RX_BUF_DISABLE,
                       I2C_MASTER_TX_BUF_DISABLE, 0);
}

void I2C_WriteWAU8822(int8_t addr, int16_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte( cmd ,(ESP_SLAVE_ADDR) | WRITE_BIT           ,ACK_CHECK_EN);
    i2c_master_write_byte( cmd ,((addr<<1) |(data>>8))| WRITE_BIT      ,ACK_CHECK_EN);
    i2c_master_write_byte( cmd ,((int8_t) (data & 0x00ff) )| WRITE_BIT ,ACK_CHECK_EN);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 100);
    i2c_cmd_link_delete(cmd);
}

void I2C_Setup_WAU8822_play()
{
    I2C_WriteWAU8822(0,  0x000);   
    vTaskdelay(10);
    I2C_WriteWAU8822(1,  0x1FF);  
    I2C_WriteWAU8822(2,  0x1BF);   
    I2C_WriteWAU8822(3,  0x1FF);   
    I2C_WriteWAU8822(4,  0x010);   
    I2C_WriteWAU8822(5,  0x000);   
    I2C_WriteWAU8822(6,  0x00C);   
    I2C_WriteWAU8822(7,  0x000);   
    I2C_WriteWAU8822(10, 0x008);   
    I2C_WriteWAU8822(14, 0x108);  
    I2C_WriteWAU8822(15, 0x0FF);  
    I2C_WriteWAU8822(16, 0x1FF);  
    I2C_WriteWAU8822(45, 0x0bf);   
    I2C_WriteWAU8822(46, 0x1bf);   
    I2C_WriteWAU8822(47, 0x175);   
    I2C_WriteWAU8822(48, 0x175);   
    I2C_WriteWAU8822(50, 0x001);   
    I2C_WriteWAU8822(51, 0x001);   
    I2C_WriteWAU8822(52, 0x039);   
    I2C_WriteWAU8822(53, 0x139);   
    I2C_WriteWAU8822(54, 0x03f);   
    I2C_WriteWAU8822(55, 0x13f);   
}

void I2C_Setup_WAU8822_record()
{
    I2C_WriteWAU8822(0,  0x000);   
    vTaskdelay(10);
    I2C_WriteWAU8822(1,  0x1FF);  
    I2C_WriteWAU8822(2,  0x1BF);   
    I2C_WriteWAU8822(3,  0x1FF);  
    I2C_WriteWAU8822(4,  0x010);  
    I2C_WriteWAU8822(5,  0x000);   
    I2C_WriteWAU8822(6,  0x00D);   
    I2C_WriteWAU8822(7,  0x006);   
    I2C_WriteWAU8822(10, 0x008);  
    I2C_WriteWAU8822(14, 0x108);   
    I2C_WriteWAU8822(15, 0x0FF);   
    I2C_WriteWAU8822(16, 0x1FF);
    I2C_WriteWAU8822(44, 0x033);	
    I2C_WriteWAU8822(45, 0x0bf);   
    I2C_WriteWAU8822(46, 0x1bf);   
    I2C_WriteWAU8822(47, 0x175);   
    I2C_WriteWAU8822(48, 0x175);  
    I2C_WriteWAU8822(50, 0x001);  
    I2C_WriteWAU8822(51, 0x001);   
    I2C_WriteWAU8822(52, 0x039);   
    I2C_WriteWAU8822(53, 0x139);   
    I2C_WriteWAU8822(54, 0x140);   
    I2C_WriteWAU8822(55, 0x140);
}

void I2S_MCLK_Init(unsigned int SAMPLE_RATE)
{
    periph_module_enable(PERIPH_LEDC_MODULE);
    ledc_timer_bit_t bit_num = (ledc_timer_bit_t) 2;     
    int duty                 = pow(2, (int) bit_num) / 2;
    ledc_timer_config_t timer_conf;
    timer_conf.bit_num       = bit_num;
    timer_conf.freq_hz       =SAMPLE_RATE*256; 
    timer_conf.speed_mode    = LEDC_HIGH_SPEED_MODE;
    timer_conf.timer_num     = LEDC_TIMER_0;
    ledc_timer_config(&timer_conf);
    ledc_channel_config_t ch_conf;
    ch_conf.channel          = LEDC_CHANNEL_1;
    ch_conf.timer_sel        = LEDC_TIMER_0;
    ch_conf.intr_type        = LEDC_INTR_DISABLE; 
    ch_conf.duty             = duty;
    ch_conf.speed_mode       = LEDC_HIGH_SPEED_MODE;
    ch_conf.gpio_num         = I2S_MCLK ;  
    ledc_channel_config(&ch_conf);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1); 
}

void I2S_Master_Init(uint32_t SAMPLE_RATE,i2s_bits_per_sample_t BITS_PER_SAMPLE)
{
	i2s_config_t i2s_config  = {
        .mode                = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX) ,                                  
        .sample_rate         = SAMPLE_RATE,
        .bits_per_sample     = BITS_PER_SAMPLE,                                                     
        .channel_format      = I2S_CHANNEL_FMT_RIGHT_LEFT,    
        .communication_format= I2S_COMM_FORMAT_I2S,                                                     
        .intr_alloc_flags    = ESP_INTR_FLAG_LEVEL1                                
    };
	i2s_config.dma_buf_count = 5;
	i2s_config.dma_buf_len   = 100;
    i2s_pin_config_t pin_config = {
        .bck_io_num   =5,
        .ws_io_num    =17,
        .data_out_num =0,
        .data_in_num  =36                                                      
    };
	i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
}  

void I2S_Slave_Init(uint32_t SAMPLE_RATE,i2s_bits_per_sample_t BITS_PER_SAMPLE)                                               
{
    i2s_config_t i2s_config  = {
        .mode                = i2s_mode_t(I2S_MODE_SLAVE | I2S_MODE_RX),                                  
        .sample_rate         = SAMPLE_RATE, 
        .bits_per_sample     = BITS_PER_SAMPLE,         
        .channel_format      = I2S_CHANNEL_FMT_RIGHT_LEFT,                           
        .communication_format= I2S_COMM_FORMAT_I2S ,
        .intr_alloc_flags    = ESP_INTR_FLAG_LEVEL1                               
    };
	i2s_config.dma_buf_count = 5;
	i2s_config.dma_buf_len   = 100;
    i2s_pin_config_t pin_config = {
        .bck_io_num   = 5,
        .ws_io_num    = 17,
        .data_out_num =0,
        .data_in_num  = 36                                                      
    };
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
}

bool SDcard_init(const char * mountpoint)
{
	sdmmc_card_t* _card;
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5
    };
	esp_err_t ret = esp_vfs_fat_sdmmc_mount(mountpoint, &host, &slot_config, &mount_config, &_card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            log_e("Failed to mount filesystem. If you want the card to be formatted, set format_if_mount_failed = true.");
        } else if (ret == ESP_ERR_INVALID_STATE) {
            log_w("SD Already mounted");
            return true;
        } else {
            log_e("Failed to initialize the card (%d). Make sure SD card lines have pull-up resistors in place.", ret);
        }
        _card = NULL;
        return false;
    }
	printf("SD card init \n");
    return true;
}

unsigned int   LittleEndian32(unsigned int v){
    return ((v & 0x000000FF) << 24 | (v & 0x0000FF00) <<  8 | (v & 0x00FF0000) >>  8 | (v & 0xFF000000) >> 24);
}

unsigned short LittleEndian16(short v)       {
    return (short)(((v << 8) & 0xFF00) | ((v >> 8) & 0x00FF));
}
