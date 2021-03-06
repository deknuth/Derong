#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <linux/hiddev.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <netdb.h>  
#include <net/if.h>  
#include <arpa/inet.h>  
#include <sys/ioctl.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <errno.h> 
#include <linux/hidraw.h>
#include <linux/input.h>
#include <errno.h> 

const char *bus_str(int bus);
char dev_path[2][24] = {0};
typedef struct ui{
	unsigned long vid;
	unsigned long pid;
}USBINFO;

int get_dev_info(char *dev_name,USBINFO* uInfo)
{
    int fd;
    int i, res, desc_size = 0;
    char buf[256];
    struct hidraw_report_descriptor rpt_desc;
    struct hidraw_devinfo info;

    /* Open the Device with non-blocking reads. In real life,
     don't use a hard coded path; use libudev instead. */
    fd = open(dev_name, O_RDWR|O_NONBLOCK);

    if (fd < 0) {
        perror("Unable to open device");
        return 1;
    }

    memset(&rpt_desc, 0x0, sizeof(rpt_desc));
    memset(&info, 0x0, sizeof(info));
    memset(buf, 0x0, sizeof(buf));


    // Get Report Descriptor Size 
	//printf("%s info:\n",dev_name);
    // Get Raw Info 
    res = ioctl(fd, HIDIOCGRAWINFO, &info);
    if (res < 0) 
        perror("HIDIOCGRAWINFO");
	else 
	{
		uInfo->vid = info.vendor;
		uInfo->pid = info.product;
     //  printf("Raw Info:\n");
     //   printf("\tbustype: %d (%s)\n",info.bustype, bus_str(info.bustype));
     //   printf("\tvendor: 0x%04hx\n", info.vendor);
     //   printf("\tproduct: 0x%04hx\n", info.product);
    }
    close(fd);
    return 0;

}
const char *bus_str(int bus)
{
    switch (bus) {
    case BUS_USB:
        return "USB";
        break;
    case BUS_HIL:
        return "HIL";
        break;
    case BUS_BLUETOOTH:
        return "Bluetooth";
        break;
    case BUS_VIRTUAL:
        return "Virtual";
        break;
    default:
        return "Other";
        break;
    }
}

int get_number(void)
{
	char path[24] = {"/dev/input"};
	DIR* pDir = NULL;
	int number = 0;
	struct dirent *pFile = NULL;

	if ((pDir=opendir(path)) == NULL)
	{
		printf("Update: Open update directory error!");
		return 0;
	}
	else
	{	
		while((pFile = readdir(pDir)) != NULL)
		{
			if(pFile->d_type == 2)		//device
			{
				if(strstr(pFile->d_name,"event")!=NULL)
				{
					number++;
				}
			}
		}
	}
	closedir(pDir);
	return number;
}

int get_path(void)
{
	int event = 0;
	int count = 0;
	char path[24] = {"/dev/"};
	DIR* pDir = NULL;
	USBINFO* usb_info = (USBINFO *)malloc(sizeof(USBINFO));
	struct dirent *pFile = NULL;
	int number = get_number();
	if(number < 3)
		return 0;
	if ((pDir=opendir("/dev/")) == NULL)
	{
		printf("Update: Open update directory error!");
		return 0;
	}
	else
	{	
		while((pFile = readdir(pDir)) != NULL)
		{
			if(pFile->d_type == 2)		//device
			{
				if(strstr(pFile->d_name,"hidraw")!=NULL)
				{
					sprintf(path,"/dev/%s",pFile->d_name);
					get_dev_info(path,usb_info);
					if(usb_info->vid==3944 && usb_info->pid==22630)	// touch
					{	
						if(strcmp(pFile->d_name,"hidraw0") == 0)
						{	
							if(number > 4)
								event = 4;
							else
								event = 1;
						}
						else if(strcmp(pFile->d_name,"hidraw1") == 0)
						{
							if(number > 4)
								event = 5;
							else
								event = 2;
						}
						else
						{
							if(number > 4)
								event = 6;
							else
								event = 3;
						}
					}
					else if((usb_info->vid==2303) && (usb_info->pid==9))	// RFID
						snprintf(dev_path[0],20,"%s",path);
					else if(usb_info->vid==1155 && usb_info->pid==17)	// scan
						snprintf(dev_path[1],20,"%s",path);
					printf("vid: %lu\t,pid: %lu\n",usb_info->vid,usb_info->pid);
					memset(path,0x00,24);
				}
			}
		}
	}
	closedir(pDir);
	free(usb_info);
	return event;
}

int get_local_ip(const char *eth_inf, char *ip)  
{  
    int sd;  
    struct sockaddr_in sin;  
    struct ifreq ifr;  
  
    sd = socket(AF_INET, SOCK_DGRAM, 0);  
    if (-1 == sd)  
    {  
        printf("socket error: %s\n", strerror(errno));  
        return -1;        
    }  
  
    strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);  
    ifr.ifr_name[IFNAMSIZ - 1] = 0;  
      
    // if error: No such device  
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)  
    {  
        printf("ioctl error: %s\n", strerror(errno));  
        close(sd);  
        return -1;  
    }  
  
    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));  
    snprintf(ip, 16, "%s", inet_ntoa(sin.sin_addr));  
    close(sd);  
    return 0;  
}

int main(void)
{
	char ip[16] = {0};
	char c2[128] = {0};
	char c10[128] = {0};
	int event = 0;
	pid_t pid;
    event = get_path();
	get_local_ip("eth1",ip);
//	printf("ip: %s\n",ip);
	if(event != 0)
	{
		sprintf(c2,"2c export TSLIB_TSDEVICE=/dev/input/event%d",event);
		sprintf(c10,"10c export QWS_MOUSE_PROTO=tslib:/dev/input/event%d",event);
		printf("%s\n%s\n",c2,c10);
		printf("rfid path: %s\n",dev_path[0]);
		printf("scan path: %s\n",dev_path[1]);
		if ((pid = fork()) < 0) 
		{
			exit(0);
	   	//	perror("fork1");
		}
		else if (pid == 0)
			execl("/bin/sed","sed","-i",c2,"/home/qtdemo",(char *) 0);
		else
		{
			waitpid(pid);
			execl("/bin/sed","sed","-i",c10,"/home/qtdemo",(char *) 0);
			return 0;
		}
	}
	return 0;
/*
	if ((pid = fork()) < 0) 
    	perror("fork2");
	else if(pid == 0)
	{
		execl("/bin/sed","sed","-i",c10,"/home/qtdemo",(char *) 0);
		return 0;
	}

	else
	{
			sleep(1);
		//	system("/home/qtdemo");
	}
*/

}

