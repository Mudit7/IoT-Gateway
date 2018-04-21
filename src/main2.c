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
#include<signal.h>

#define MAXLEN 80
#define CONFIG_FILE "/home/km/KM_GIT/iot/gateway/config"
#define NO_OF_NODES 2



void rx_temp(int);
void rx_hum(int);
void* thread_rx(void* arg);
void* thread_tx(void* arg);
void* thread_WDT(void* arg);
pthread_t a_thread,b_thread, c_thread;

int i=0, fd,cport_nr=4,bdrate=115200;
void* thread1_result,*thread2_result,*thread3_result;
//int flag_func=0;	
int node_id=0;
char lcd_str[10],w_str[6],str_t[6],str_h[6];
//int flag_priority=0;
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

void rx_temp(int sig)
{
		printf("Rx_Temp\n");
		char lcd_str[16];
		int  n = KM_Serial_PollComport(cport_nr, buf, 5);
		if(n<5) 
		{
			sprintf(lcd_str,"Node %d Offline",node_id);
			KM_LCD_Str_XY(0,1,lcd_str);
			return;
		}
		printf("Node %c:Temperature=> %c%c\n", buf[1], buf[2],buf[3]);
		sprintf(temp_buf, "%c%c",buf[2],buf[3]);
		int snd=mosquitto_publish(mosq,NULL,MQTT_TOPIC,strlen(temp_buf),temp_buf,0,0);
		if(snd!=0)
		{	
			perror("mosquitto_publish: ");
        		exit(EXIT_FAILURE);
		}
		sprintf(lcd_str,"N1: Temp->%c%c",buf[2],buf[3]);
		KM_Serial_flushRX(cport_nr);
		KM_LCD_Str_XY(0,1,lcd_str);	
}
void rx_hum(int sig)
{	
		
		printf("Rx Hum\n");
		char lcd_str[16];
		int  n = KM_Serial_PollComport(cport_nr, buf, 5);
		if(n<5) 
		{
			sprintf(lcd_str,"Node %d Offline",node_id);
			KM_LCD_Str_XY(0,1,lcd_str);
			return;
		}
		printf("Node %c:Humidity=> %c%c\n", buf[1], buf[2],buf[3]);
		sprintf(hum_buf, "H%c%c",buf[2],buf[3]);
		int snd=mosquitto_publish(mosq,NULL,MQTT_TOPIC,strlen(hum_buf),hum_buf,0,0);
		if(snd!=0)
		{	
			perror("mosquitto_publish: ");
        		exit(EXIT_FAILURE);
		}
		sprintf(lcd_str,"N1: Hum->%c%c",buf[2],buf[3]);
		KM_Serial_flushRX(cport_nr);
	
}



int main()
{
char mode[]={'8','N','1',0};
int res;
param_config();	//Reading config file

res = mosquitto_lib_init();
if(res<0)
{
	perror("mosquitto_init:");
	return -1;
}

mosq = mosquitto_new(NULL, true, 0);
printf("mosq %p\n",mosq);

rc = mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, keepalive);
if(rc!=0)
{
	perror("mosquitto_connect:\n");
}
res = KM_LCD_Init();
if(res<0)
{
  perror("LCD initialization failed");
  exit(EXIT_FAILURE);

}

KM_LCD_ClrScr();
KM_LCD_Str_XY(0,0,"Gateway->RS485");
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

return 0;
}


void* thread_rx(void* arg)
{
	(void) signal(SIGALRM,rx_temp);
	(void) signal(SIGPROF,rx_hum);	
	while(1)
	{	
		printf("rx thread\n");	
		pause();
	}	
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	pthread_exit(thread1_result);
}

void* thread_tx(void* arg)
{
while(1)
{
	printf("tx thread\n");
	//Change Node id
	node_id=(node_id+1)%(NO_OF_NODES+1);
	if(node_id==0)	node_id++;
//	while(flag_priority!=0);
	//For Temperature
	KM_Serial_flushTX(cport_nr);
	write(fd,"1",4);
	usleep(100000);
	sprintf(str_t,"<%dT>\n",node_id);
	KM_Serial_SendBuf(cport_nr, str_t,sizeof(str_t));
	usleep(90000);
	write(fd,"0",4);
	usleep(100000);
	printf("sent: %s\n", str_t);
	pthread_kill(a_thread,SIGALRM);
		
	usleep(2000000);

	KM_Serial_flushTX(cport_nr);
	write(fd,"1",4);
	usleep(100000);
	sprintf(str_h,"<%dH>\n",node_id);
	KM_Serial_SendBuf(cport_nr, str_h,sizeof(str_h));
	usleep(90000);
	write(fd,"0",4);
	usleep(10000);
	printf("sent: %s\n", str_h);
	pthread_kill(a_thread,SIGPROF);

	usleep(3000000);
}
pthread_exit(thread2_result);
}

void* thread_WDT(void* arg)
{
while(1)
{
	write(fd,"1",4);
	usleep(1000);
	strcpy(w_str,"<0W>");
	KM_Serial_SendBuf(cport_nr,w_str,sizeof(w_str));
	usleep(90000);
	printf("sent %s\n",w_str);
	write(fd,"0",4);
	//flag_priority=1;
	usleep(1000);
	sleep(3);
}
pthread_exit(thread3_result);
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

 

