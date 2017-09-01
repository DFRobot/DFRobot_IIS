DFRobot_IIS Library for Arduino
---------------------------------------------------------

The library is used to Play music file from SD card,Record sound and save in SD card,take photo
and save in SD card 

### Choose audio mode or camera mode

    void init(uint8_t mode);
    mode:AUDIO CAMERA

### Enter the volume(from 0 to 99) to set volume when playMusic

    void setSpeakersVolume(uint8_t volume);
    void setHeadphonesVolume(uint8_t volume);

### Init Music Player

    void initPlayer();

### Enter the WAV file name in the SD card you want to play

    int playMusic(const char *filename);

### Control the music player

    void playerControl(uint8_t cmd);
    cmd:PLAY PAUSE STOP

### Init Recorder

    void initRecorder();

### Enter the WAV file name you want to store in the SD card 

    int recordSound(const char *outputFilename);

### Control the recorder

    void recorderControl(uint8_t cmd);
    cmd:BEGIN STOP 

### Enter the BMP file name you want to store in the SD card

    void takePhoto(const char *pictureFilename);

### Init SD card

    bool SDcard_Init(void);

### Open file in SD card in wirte mode or read mode

    bool SDcard_Open(const char* SDfilename,uint8_t mode)
    mode:SD_Write SD_Read

### Write data to file in SD card

    bool SDcard_Write(const char* data);

### Read data from SD card

    void SDcard_Read();

### Save data and close file

    bool SDcard_Close();

 * @file playWAV.ino record.ino camera.ino SDcard.ino
 * @brief DFRobot's IIS Module
 * @n These three examples are used to play music record and takephoto 
 *
 * @n [Get the module here](等上架后添加商品购买链接)
 * @n [Connection and Diagram](等上架后添加wiki链接)
 *
 * @copyright    [DFRobot](http://www.dfrobot.com), 2017
 * @copyright    GNU Lesser General Public License
 *
 * @author [Zhangjiawei<jiawei.zhang@dfrobot.com>]
 * @version  V1.0
 * @date  2017-8-1
 * @https://github.com/DFRobot/DFRobot_IIS