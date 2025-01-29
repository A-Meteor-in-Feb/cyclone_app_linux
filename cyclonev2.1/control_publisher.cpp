#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>

#include "serial.h"
#include "wit_c_sdk.h"
#include "REG.h"
#include <stdint.h>

#include "dds/dds.hpp"
#include "shutdownsignal.hpp"
#include "ControlData.hpp"
#include "partitionName.hpp"
#include "TimeStampLogger.h"


#define ACC_UPDATE		0x01
#define GYRO_UPDATE		0x02
#define ANGLE_UPDATE	0x04
#define MAG_UPDATE		0x08
#define READ_UPDATE		0x80

int count_sentMsg0 = 200;

static int fd, s_iCurBaud = 9600;
static volatile char s_cDataUpdate = 0;


const int c_uiBaud[] = {2400 , 4800 , 9600 , 19200 , 38400 , 57600 , 115200 , 230400 , 460800 , 921600};


static void AutoScanSensor(char* dev);
static void SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum);
static void Delayms(uint16_t ucMs);


int serial_read_data(int fd, unsigned char *val, int len){
    len=read(fd,val,len);
	  return len;
}

int serial_write_data(int fd, unsigned char *val, int len){
	  len=write(fd,val,len*sizeof(unsigned char));
	  return len;
}

int serial_open(char* dev, unsigned int baud){

    int fd = open(dev, O_RDWR|O_NOCTTY); 

    if (fd < 0){
        return fd;
    }

    if(isatty(STDIN_FILENO)==0) {
   	    printf("standard input is not a terminal device\n"); 
    }else{
	      printf("isatty success!\n"); 
    }

    struct termios newtio, oldtio; 
  
    if (tcgetattr(fd, &oldtio) != 0){
        perror("SetupSerial 1");
	      printf("tcgetattr(fd, &oldtio) -> %d\n",tcgetattr(fd, &oldtio)); 
        return -1; 
    } 
    
    bzero( &newtio, sizeof( newtio ) ); 
    newtio.c_cflag |= CLOCAL | CREAD;  
    newtio.c_cflag |= CS8; 
    newtio.c_cflag &= ~PARENB; 

    switch( baud ){
        case 2400:
            cfsetispeed(&newtio, B2400); 
            cfsetospeed(&newtio, B2400); 
            break; 
       
        case 4800: 
            cfsetispeed(&newtio, B4800); 
            cfsetospeed(&newtio, B4800); 
            break; 
    
        case 9600: 
            cfsetispeed(&newtio, B9600); 
            cfsetospeed(&newtio, B9600); 
            break; 
       
        case 115200: 
            cfsetispeed(&newtio, B115200); 
            cfsetospeed(&newtio, B115200); 
            break; 
    
        case 230400: 
            cfsetispeed(&newtio, B230400); 
            cfsetospeed(&newtio, B230400); 
            break; 
    
        case 460800: 
            cfsetispeed(&newtio, B460800); 
            cfsetospeed(&newtio, B460800); 
            break; 
    
        default: 
            cfsetispeed(&newtio, B9600); 
            cfsetospeed(&newtio, B9600); 
            break; 
    } 
    
    newtio.c_cflag &=  ~CSTOPB; 
    newtio.c_cc[VTIME]  = 0; 
    newtio.c_cc[VMIN] = 0; 
    tcflush(fd,TCIFLUSH); 

    if((tcsetattr(fd,TCSANOW,&newtio))!=0){
        perror("com set error"); 
        return -1; 
    }
 
    return fd; 
}

void serial_close(int fd){
    close(fd);
}


static void SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum){
    
    int i;

    for(i = 0; i < uiRegNum; i++){
        
        switch(uiReg){
            //case AX:
            //case AY:
            case AZ:
			          s_cDataUpdate |= ACC_UPDATE;
                break;
      
            //case GX:
            //case GY:
            case GZ:
				        s_cDataUpdate |= GYRO_UPDATE;
                break;

            //case HX:
            //case HY:
            case HZ:
				        s_cDataUpdate |= MAG_UPDATE;
                break;

            //case Roll:
            //case Pitch:
            case Yaw:
				        s_cDataUpdate |= ANGLE_UPDATE;
                break;
      
            default:
				        s_cDataUpdate |= READ_UPDATE;
			          break;
          }

		    uiReg++;
    }

}


static void Delayms(uint16_t ucMs){ 
    usleep(ucMs*1000);
}
 
	
static void AutoScanSensor(char* dev){
	
    int i, iRetry;
	  char cBuff[1];
	
	  for(i = 1; i < 10; i++){
		
        serial_close(fd);
		    s_iCurBaud = c_uiBaud[i];
		    fd = serial_open(dev , c_uiBaud[i]);
		
		    iRetry = 2;

		    do{
			      s_cDataUpdate = 0;
			      WitReadReg(AX, 3);
			      Delayms(200);
			
            while(serial_read_data(fd, (unsigned char*)cBuff, 1)){
				        WitSerialDataIn(cBuff[0]);
			      }
			
            if(s_cDataUpdate != 0){
				        printf("%d baud find sensor\r\n\r\n", c_uiBaud[i]);
				        return ;
			      }
			
            iRetry--;

		    }while(iRetry);

	  }

	  printf("can not find sensor\r\n");
	  printf("please check your connection\r\n");
}


void control_domain_publisher(int& vehicle, std::string& control_partition_name){

    //const std::string filename = "vehicle_imu.txt";

    std::string name = control_partition_name;

    std::cout << "start running publisher, partition: " << name << std::endl;

    if((fd = serial_open((char*)"/dev/ttyUSB0", 9600) < 0 )){
        std::cout << "open /dev/ttyUSB0 fail" << std::endl;
    } else {
        std::cout << "open /dev/ttyUSB0 success" << std::endl;
    }


    std::string vehicle_id = "vehicle" + std::to_string(vehicle);

    int control_domain = 1;

    dds::domain::DomainParticipant participant(control_domain);

    dds::topic::Topic<ControlData::imu_data> imu_topic(participant, "imu_data");

    dds::pub::qos::PublisherQos pub_qos;

	dds::core::StringSeq partition_name{ name };

	pub_qos << dds::core::policy::Partition(partition_name);

    dds::pub::Publisher vehicle_publisher(participant, pub_qos);

    dds::pub::DataWriter<ControlData::imu_data> imu_writer(vehicle_publisher, imu_topic);


    float fAcc[3], fGyro[3], fAngle[3];
    int i, ret;
    char cBuff[1];

    WitInit(WIT_PROTOCOL_NORMAL, 0x50);
    WitRegisterCallBack(SensorDataUpdata);
    AutoScanSensor((char*)"/dev/ttyUSB0");

    std::string timestamp;
    const auto frame_duration = std::chrono::milliseconds(50);

    //while(!shutdown_requested && count_sentMsg0 > 0){
    while(count_sentMsg0 > 0) {
        auto start_time = std::chrono::steady_clock::now();

        while(serial_read_data(fd, (unsigned char*)cBuff, 1)){
            WitSerialDataIn(cBuff[0]);
        }

        Delayms(1000);

        if(s_cDataUpdate){
            for(i = 0; i < 3; i++){
                fAcc[i] = sReg[AX+i] / 32768.0f * 16.0f;
                fGyro[i] = sReg[GX+i] / 32768.0f * 2000.0f;
                fAngle[i] = sReg[Roll+i] / 32768.0f * 180.0f;
            }

            if(s_cDataUpdate & ACC_UPDATE){
                s_cDataUpdate &= ~ACC_UPDATE;
            }

            if(s_cDataUpdate & GYRO_UPDATE){
                s_cDataUpdate &= ~GYRO_UPDATE;
            }

            if(s_cDataUpdate & ANGLE_UPDATE){
                s_cDataUpdate &= ~ANGLE_UPDATE;
            }

            if(s_cDataUpdate & MAG_UPDATE){
                s_cDataUpdate &= ~MAG_UPDATE;
            }

            ControlData::imu_data imu_data({ fAcc[0], fAcc[1], fAcc[2] }, { fGyro[0], fGyro[1], fGyro[2] }, { fAngle[0], fAngle[1], fAngle[2] }, { static_cast<double>(sReg[HX]), static_cast<double>(sReg[HY]), static_cast<double>(sReg[HZ]) });
            imu_writer.write(imu_data);

            timestamp = TimestampLogger::getTimestamp();
            //TimestampLogger::writeToFile(filename, timestamp);
            std::cout << "publish imu data at: " << timestamp << std::endl;

            count_sentMsg0 -= 1;

            auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time);
            if(elapsed_time < frame_duration){
                std::this_thread::sleep_for(frame_duration - elapsed_time);
            }
            
        }
    }

    std::cout << "publish all data" << std::endl;
    serial_close(fd);

}