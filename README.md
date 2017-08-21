DFRobot_IIS Library for Arduino
---------------------------------------------------------

The library is used to play music and recording

### Enter the audio mode or camera mod
  
    void init(int mode);
   
### Enter the WAV file name in the SD card you want to play
    
    int playMusic(const char *filename);
	
### Enter the volume(from 0 to 100) to set volume when playmusic

    void setVolume(int volume);

### Enter the WAV file name you want to store in the SD card 

    int recordSound(const char *outputFilename);
	
### Enter the BMP file name you want to store in the SD card
   
    void takephoto(const char *pictureFilename);   
 
 * @file playmusic.ino and record.ino
 * @brief DFRobot's IIS Module
 * @n These two examples are used to play music and recording
 *
 * @n [Get the module here](等上架后添加商品购买链接)
 * @n This example Set the volume size and play music snippet.
 * @n [Connection and Diagram](等上架后添加wiki链接)
 *
 * @copyright	[DFRobot](http://www.dfrobot.com), 2017
 * @copyright	GNU Lesser General Public License
 *
 * @author [Zhangjiawei<jiawei.zhang@dfrobot.com>]
 * @version  V1.0
 * @date  2017-8-1
 * @https://github.com/DFRobot/DFRobot_IIS