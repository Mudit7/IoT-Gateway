
#include <stdio.h>
#include <unistd.h>
#include<pthread.h>
#include<semaphore.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include "../inc/rs485.h"

void* thread_rx(void* arg);
void* thread_tx(void* arg);
pthread_t a_thread,b_thread;
pthread_t a_thread,b_thread;
char str[50];
int i=0, fd,cport_nr=4,bdrate=115200;
sem_t bin_sem;
int k=0; // for complete packet reception
void* thread1_result,*thread2_result;

//unsigned char buf[6]={0};
int main()
{
//  int i=0;
//  cport_nr=4,        /* /dev/ttyO4 */
  //cport_nr=16,        /* /dev/ttyUSB0 */
  //bdrate=9600;       /* 9600 baud */
 // bdrate=115200;       /* 115200 baud */

 char mode[]={'8','N','1',0};
int res;
res = KM_LCD_Init();
if(res<0)
{
  perror("LCD initialization failed");
  exit(EXIT_FAILURE);

}

KM_LCD_ClrScr();
KM_LCD_Str_XY(0,0,"Gateway->RS485");
res = sem_init(&bin_sem,0,1);
if (res != 0) {
        perror("Semaphore initialization failed");
        exit(EXIT_FAILURE);
    }

fd=open("/sys/class/gpio/export",O_WRONLY);
if(fd<0)
{
	perror("export open:");
	return -1;
}
write(fd,"60",4);

fd=open("/sys/class/gpio/gpio60/direction",O_RDWR);

if(fd<0)
{
	perror("direction open:");
	return -1;
}
write(fd,"out",3);

fd=open("/sys/class/gpio/gpio60/value",O_RDWR);

if(fd<0)
{
	write(1,"gpio60 value open failed",11);
	return -1;
}

 if(KM_Serial_OpenComport(cport_nr, bdrate, mode))
  {
    printf("Can not open comport\n");
    return(0);
  }
    res=pthread_create(&a_thread,NULL,thread_rx,(void*)0);
    	if(res<0)
    	{
		perror("thread_create failed : \n");
        	return -1;
   	 }
   res=pthread_create(&b_thread,NULL,thread_tx,(void*)0);
    	if(res<0)
    	{
        	perror("thread_create failed : \n");
        	return -1;
    	}

    res=pthread_join(a_thread,&thread1_result);
    	if(res<0)
   	 {
       		 perror("join failed:\n");
        	exit(EXIT_FAILURE);
    	 }
    	else
    	{
        	printf("thread joined\n");
    	}
   

   res=pthread_join(b_thread,&thread2_result);
    	if(res<0)
    	{
        	perror("join failed:\n");
        	exit(EXIT_FAILURE);
    	}
    	else
    	{
        	printf("thread joined\n");
    	}
    sem_destroy(&bin_sem);
return 0;
}


void* thread_rx(void* arg)
{
while(1)
{
sem_wait(&bin_sem);
sem_post(&bin_sem);  
//write(fd,"0",4);
char buf[6];
int  n = KM_Serial_PollComport(cport_nr, buf, 5);

if(n>=5)
{
printf("Node %c:Temperature=> %c%c\n", buf[1], buf[2],buf[3]);

KM_LCD_Str_XY(0,1,"Temp:");
HD44780_PutChar(buf[2]);
HD44780_PutChar(buf[3]);

KM_Serial_flushRX(cport_nr);
KM_Serial_flushTX(cport_nr);
}
usleep(10000);
 }
    pthread_exit(thread1_result);
}

void* thread_tx(void* arg)
{
while(1)
{
sem_wait(&bin_sem);
write(fd,"1",4);
usleep(1000);
strcpy(str,"<1T>\n");
KM_Serial_SendBuf(cport_nr, str,sizeof(str));
usleep(90000);
write(fd,"0",4);
usleep(100);
printf("sent: %s\n", str);
sem_post(&bin_sem);
KM_Serial_flushRX(cport_nr);
KM_Serial_flushTX(cport_nr);
usleep(2000000);
 }
pthread_exit(thread2_result);
}
