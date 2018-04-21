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
#include<mosquitto.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>

#define MAXLEN 80
#define CONFIG_FILE "/home/km/KM_GIT/iot/gateway/config"
#define NO_OF_NODES 2

void* thread_rx(void* arg);
void* thread_tx(void* arg);
void* thread_WDT(void* arg);
pthread_t a_thread,b_thread, c_thread;

int i=0, fd,cport_nr=4,bdrate=115200;
sem_t bin_sem;
int k=0; // for complete packet reception
void* thread1_result,*thread2_result,*thread3_result;
int flag_func=0;	
int node_id=0;
char lcd_str[10],w_str[6],str_t[6],str_h[6];
int flag_priority=0;
//unsigned char buf[6]={0};
uint8_t reconnect = true;
char clientid[24];
int rc = 0;
char buf[30]; 

struct mosquitto *mosq = NULL;
int keepalive = 60;
bool clean_session = true;
char temp_buf[30],hum_buf[30];

char MQTT_HOST[MAXLEN];
int  MQTT_PORT;
char MQTT_TOPIC[MAXLEN];


void param_config(void);
char* trim (char * );

int main()
{
char mode[]={'8','N','1',0};
int res;
param_config();	//Reading config file

res = mosquitto_lib_init();
if(res!=0)
{
	perror("mosquitto_init:");
	return -1;
}
bool e=true;

//mosquitto_lib_cleanup();

mosq = mosquitto_new(NULL, true, 0);
   if(mosq==NULL)
        perror("mosq_new: ");
printf("mosq:%p\n",mosq);

rc = mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, keepalive);
if(rc!=0)
{
	perror("mosquitto_connect fails\n");
}
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
        exit(EXIT_FAILURE);
}
write(fd,"60",4);

fd=open("/sys/class/gpio/gpio60/direction",O_RDWR);

if(fd<0)
{
	perror("direction open:");
        exit(EXIT_FAILURE);
}
write(fd,"out",3);

fd=open("/sys/class/gpio/gpio60/value",O_RDWR);

if(fd<0)
{
	write(1,"gpio60 value open failed",11);
        exit(EXIT_FAILURE);
}

 if(KM_Serial_OpenComport(cport_nr, bdrate, mode))
  {
    printf("Can not open comport\n");
        exit(EXIT_FAILURE);
  }


 int snd=mosquitto_publish(mosq,NULL,MQTT_TOPIC,10,"mudmalp",0,0);
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

  res=pthread_create(&c_thread,NULL,thread_WDT,(void*)0);
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

   res=pthread_join(c_thread,&thread3_result);
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
//	printf("Rx Thread\n");
	
	char buf[6];
	int  n = KM_Serial_PollComport(cport_nr, buf, 5);


	if(n>=5)
	{
		sprintf(lcd_str,"N%c=>",buf[1]);
		KM_LCD_Str_XY(0,1,lcd_str);
		
	    if(flag_func==0)
	     {
		printf("Node %c:Temperature=> %c%c\n", buf[1], buf[2],buf[3]);
		sprintf(temp_buf, "T%c%c",buf[2],buf[3]);
		printf(temp_buf);
		int snd=mosquitto_publish(mosq,NULL,"/weather",strlen(temp_buf),temp_buf,0,1);
		if(snd!=0)
		{	
			perror("mosquitto_publish: ");
        		exit(EXIT_FAILURE);
		}
		KM_LCD_Str_XY(5,1,"Temp:");
	      }	
	    if(flag_func==1)
	      {
		printf("Node %c:Humidity=> %c%c\n", buf[1], buf[2],buf[3]);
		sprintf(hum_buf, "H%c%c",buf[2],buf[3]);
		printf("%s\n",hum_buf);
		int snd=mosquitto_publish(mosq,NULL,MQTT_TOPIC,strlen(hum_buf),hum_buf,0,1);
		if(snd!=0)
		{	
			perror("mosquitto_publish: ");	
        		exit(EXIT_FAILURE);
		}
		KM_LCD_Str_XY(5,1,"Hum :");
	      }

		HD44780_PutChar(buf[2]);
		HD44780_PutChar(buf[3]);
		KM_Serial_flushRX(cport_nr);
		//KM_Serial_flushTX(cport_nr);
	}
	flag_priority=0;
	sem_post(&bin_sem);  
	usleep(10000);
 }
mosquitto_destroy(mosq);
mosquitto_lib_cleanup();
pthread_exit(thread1_result);
}

void* thread_tx(void* arg)
{
while(1)
{
	//Change Node id
//	node_id=(node_id+1)%(NO_OF_NODES+1);
	if(node_id==0)	node_id++;
	while(flag_priority!=0);
	//For Temperature
	sem_wait(&bin_sem);
	KM_Serial_flushTX(cport_nr);
	write(fd,"1",4);
	usleep(1000);
	sprintf(str_t,"<%dT>\n",node_id);
	KM_Serial_SendBuf(cport_nr, str_t,sizeof(str_t));
	usleep(90000);
	write(fd,"0",4);
	usleep(100);
	printf("sent: %s\n", str_t);
	flag_func=0;
//	KM_Serial_flushRX(cport_nr);

	sem_post(&bin_sem);
	
	usleep(2000000);

	//For Humidity
	sem_wait(&bin_sem);
	KM_Serial_flushTX(cport_nr);
	write(fd,"1",4);
	usleep(1000);
	sprintf(str_h,"<%dH>\n",node_id);
	KM_Serial_SendBuf(cport_nr, str_h,sizeof(str_h));
	usleep(90000);
	write(fd,"0",4);
	usleep(100);
	printf("sent: %s\n", str_h);
	flag_func=1;
//	KM_Serial_flushRX(cport_nr);

//	usleep(300000);
	sem_post(&bin_sem);


	usleep(5000000);
}
pthread_exit(thread2_result);
}

void* thread_WDT(void* arg)
{
while(1)
{
	sem_wait(&bin_sem);
	write(fd,"1",4);
	usleep(1000);
	strcpy(w_str,"<0W>");
	KM_Serial_SendBuf(cport_nr,w_str,sizeof(w_str));
	usleep(90000);
	printf("sent %s\n",w_str);
	write(fd,"0",4);
	flag_priority=1;
	usleep(1000);
	sem_post(&bin_sem);
	sleep(3);
}
pthread_exit(thread3_result);
}

void param_config(void) {
	char *s, buff[256];
	FILE *fp = fopen (CONFIG_FILE, "r");
	if (fp == NULL)
	{
		return;
	}

	/* Read next line */
	while ((s = fgets (buff, sizeof buff, fp)) != NULL)
	{
		/* Skip blank lines and comments */
		if (buff[0] == '\n' || buff[0] == '#')
			continue;

		/* Parse name/value pair from line */
		char name[MAXLEN], value[MAXLEN];
		s = strtok (buff, "=");
		if (s==NULL)
			continue;
		else
			strncpy (name, s, MAXLEN);
		s = strtok (NULL, "=");
		if (s==NULL)
			continue;
		else
			strncpy (value, s, MAXLEN);
		trim (value);

		/* Copy into correct entry in parameters struct */
		if (strcmp(name, "MQTT_HOST")==0){
			strncpy (MQTT_HOST, value, MAXLEN);
			printf("MQTT_HOST:\t%s\n",MQTT_HOST);
		}
		else if (strcmp(name, "MQTT_PORT")==0){
			MQTT_PORT = atoi(value);
			printf("MQTT_PORT:\t%d\n",MQTT_PORT);
		}
		//      strncpy (parms->flavor, value, MAXLEN);
		else if (strcmp(name, "MQTT_TOPIC")==0){
			strncpy (MQTT_TOPIC, value, MAXLEN);
			printf("MQTT_TOPIC:\t%s\n",MQTT_TOPIC);
		}
		else
			printf ("WARNING: %s/%s: Unknown name/value pair!\n",
					name, value);
	}

	/* Close file */
	fclose (fp);
}


char* trim (char * s)
{
	/* Initialize start, end pointers */
	char *s1 = s, *s2 = &s[strlen (s) - 1];

	/* Trim and delimit right side */
	while ( (isspace (*s2)) && (s2 >= s1) )
		s2--;
	*(s2+1) = '\0';

	/* Trim left side */
	while ( (isspace (*s1)) && (s1 < s2) )
		s1++;

	/* Copy finished string */
	strcpy (s, s1);
	return s;
}

