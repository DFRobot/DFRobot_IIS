DFRobot_IIS Library for Arduino
---------------------------------------------------------

The library is used to play music recording and take photo

### Enter the audio mode 
  
    void init(int Audioplay);
   
### Enter the WAV file name in the SD card you want to play , press user key to stop and continue
    
    int playMusic(const char *filename);
	
### Enter the volume(from 0 to 100) to set volume when playmusic

    void setVolume(int volume);

### Enter the WAV file name you want to store in the SD card , press user key to stop record

    int recordSound(const char *outputFilename);

### Enter the camera mode and take photo 

    void init(int Camera);
 
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