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
#define CONFIG_FILE "/home/km/KM_GIT/iot/gateway/config"      //abs location for config file
//#define CONFIG_FILE "/home/km/KM_GIT/iot/gateway/config_copy"      //abs location for config file

//#define NO_OF_NODES 2



void rx_hum_temp(int);
void* thread_rx(void* arg);
void* thread_tx(void* arg);
void* thread_config(void* arg);
pthread_t a_thread,b_thread, c_thread;

int flag_wait=0;

int i=0, fd,cport_nr2=4,cport_nr1=5,bdrate=115200;
void* thread1_result,*thread2_result,*thread3_result;	
int node_id=0;
char lcd_str[10],w_str[6],str_t[6],str_h[6];
//int flag_priority=0;
uint8_t reconnect = true;
char clientid[24];
int rc = 0;
char buf[30]; 
int data_flag=0;
int offline_cnt[10];                     //no of times nodes didn't respond (consequently)
int no_of_nodes=0;
int poll_time=0;                         //polling interval

struct mosquitto *mosq = NULL;
int keepalive = 60;
bool clean_session = true;
char data_buf[7];                         //clean packet data
char mosq_buf[30];                        //final publish data buffer
char func_code;

char rs485_uart[15];                      //dev file for rs485 uart (/dev/tty..)
char zigbee_uart[15];

bool zig_en=0;
bool rs_en=0;
char MQTT_HOST[MAXLEN];
int  MQTT_PORT;
char MQTT_TOPIC[MAXLEN];

sem_t bin_sem;

void param_config(void);
char* trim (char * );


void offline_func(int node)
{
	
	printf("Node %d Offline\n", node);
	sprintf(lcd_str,"Node %d Offline",node);
	KM_LCD_Str_XY(0,1,lcd_str);
	offline_cnt[node-1]=0;
	sprintf(mosq_buf,"%dO",node);
	int snd=mosquitto_publish(mosq,NULL,"/weather",strlen(mosq_buf),mosq_buf,0,0);
	if(snd!=0)
	{	
		perror("mosquitto_publish: ");
        	exit(EXIT_FAILURE);
	}
}

void rx_hum_temp(int sig)                  //To receive the data from uart buffer
{ 		int n1,n2;	
		
		sem_wait(&bin_sem);
		//printf("Rx signal\n");
		char lcd_str[16];
		usleep(500000);
		if(zig_en)
		 n1 = KM_Serial_PollComport(cport_nr1, buf, 20);             //uart2 
		if(rs_en&&n1<5)
		 n2 = KM_Serial_PollComport(cport_nr2, buf, 20);             //uart4

//		printf("buffer:%s\n",buf);
		
		if((n1<4)&&(n2<4)) 
		{

			KM_Serial_flushRX(cport_nr1);
			KM_Serial_flushRX(cport_nr2);
			for(i=0;i<20;i++)
			buf[i]=0;
		        offline_cnt[node_id-1]++;
			if(offline_cnt[node_id-1]>=5)
			{
				offline_func(node_id);
			}
			sem_post(&bin_sem);

			return;
		}
		//extract data
		int k=0;
		for(i=0;i<10;i++)	
		{
			if(buf[i]=='<')
			{
				data_buf[k]=buf[i];
				data_flag=1;
				continue;
			}
			if(buf[i]=='>')
			{	
				k++;
				data_buf[k]=buf[i];
				data_flag=0;
				data_buf[k+1]=0;
				break;
			}
			if(data_flag==1)
			{
				k++;
				data_buf[k]=buf[i];
			}
		
			
		}
		//printf("data_buf=%s\n",data_buf);
		if(k<5||k>7) //bad packet
		{
			printf("Bad Packet %d\n",k);	
			KM_Serial_flushRX(cport_nr1);
			KM_Serial_flushRX(cport_nr2);
		}
		else
		{
			offline_cnt[node_id-1]=0;
		}
		
		 // check if data received is temperature or humidity

		if(data_buf[2]=='T')
		{	
			printf("Node %c:Temperature=> %c%c\n", data_buf[1], data_buf[3],data_buf[4]);
			sprintf(mosq_buf, "%dT%c%c",node_id,data_buf[3],data_buf[4]);
			sprintf(lcd_str,"N%c: Temp->%c%c  ",data_buf[1],data_buf[3],data_buf[4]);
		}
		if(data_buf[2]=='H')
		{
			printf("Node %c:Humidity=> %c%c\n", data_buf[1], data_buf[3],data_buf[4]);
			sprintf(mosq_buf, "%dH%c%c",node_id,data_buf[3],data_buf[4]);
			sprintf(lcd_str,"N%c: Hum ->%c%c  ",data_buf[1],data_buf[3],data_buf[4]);
		}

		// finally publish the received data

		int snd=mosquitto_publish(mosq,NULL,"/weather",strlen(mosq_buf),mosq_buf,0,0);
		if(snd!=0)
		{	
			perror("mosquitto_publish: ");
//        		exit(EXIT_FAILURE);
		}

		KM_Serial_flushRX(cport_nr1);
		KM_Serial_flushRX(cport_nr2);
		sem_post(&bin_sem);	
		KM_LCD_Str_XY(0,1,lcd_str);	
		
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
sem_init(&bin_sem,0,1);
mosq = mosquitto_new(NULL, true, 0);
printf("mosq %p\n",mosq);

rc = mosquitto_connect(mosq,"localhost",1883 , keepalive);
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
KM_LCD_Str_XY(0,0,"Gateway->");
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

 if(KM_Serial_OpenComport(cport_nr1, bdrate, mode))
  {
    printf("Can not open comport - uart2\n");
        exit(EXIT_FAILURE);
  }
 if(KM_Serial_OpenComport(cport_nr2, bdrate, mode))
  {
    printf("Can not open comport - uart4\n");
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
    res=pthread_create(&c_thread,NULL,thread_config,(void*)0);
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
	(void) signal(SIGALRM,rx_hum_temp);
	while(1)
	{	
	//	printf("rx thread\n");	
		pause();
	}	
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	pthread_exit(thread1_result);
}

void* thread_tx(void* arg)
{ 
 char func_code;
while(1)
{	
 	sem_wait(&bin_sem);
       // printf("tx thread\n");
        //Change Node id
        if(func_code=='H')
        node_id=(node_id+1)%(no_of_nodes+1);
        if(node_id==0)  node_id++;
        func_code=(func_code=='T')?'H':'T';

        KM_Serial_flushTX(cport_nr1);
        KM_Serial_flushTX(cport_nr2);
        write(fd,"1",4);
        usleep(10000);
        sprintf(str_t,"<%d%c>\n",node_id,func_code);
        if(zig_en)
        KM_Serial_SendBuf(cport_nr1, str_t,sizeof(str_t));  //send through zigbee and rs485
        if(rs_en)
        KM_Serial_SendBuf(cport_nr2, str_t,sizeof(str_t));
        usleep(100000);
        write(fd,"0",4);
       // printf("sent: %s\n", str_t);
        usleep(300000);
        pthread_kill(a_thread,SIGALRM);
	
	sem_post(&bin_sem);

        sleep(poll_time);               //Poll Interval

}
pthread_exit(thread2_result);
}

void *thread_config(void *arg)
{
while(1){
 //printf("config thread\n");
 sem_wait(&bin_sem);
 param_config();
 sem_post(&bin_sem);
 //printf("post\n");
 sleep(10);
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
	printf("\n\n***************Reading Configuration****************\n");
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
                        strcpy (name, s);
                s = strtok (NULL, "=");
                if (s==NULL)
                        continue;
                else
                        strcpy (value, s);
                trim (value);

                /* Copy into correct entry in parameters struct */
                if (strcmp(name, "MQTT_HOST")==0){
                        strcpy (MQTT_HOST,value);
   //                     printf("MQTT_HOST:\t%s\n",MQTT_HOST);
                }
                else if (strcmp(name, "MQTT_PORT")==0){
                        MQTT_PORT = atoi(value);
     //                   printf("MQTT_PORT:\t%d\n",MQTT_PORT);
                }
                                                                                                                           
                //      strncpy (parms->flavor, value, MAXLEN);
                else if (strcmp(name, "MQTT_TOPIC")==0){
                        strcpy (MQTT_TOPIC, value);
       //                 printf("MQTT_TOPIC:\t%s\n",MQTT_TOPIC);
                }
                //	IoT Configuration
		
		else if (strcmp(name, "RS485_UART")==0){
                        strcpy (rs485_uart, value);   
         //               printf("RS485_UART = \t%s\n",rs485_uart);
                }
		else if (strcmp(name, "ZIGBEE_UART")==0){
                        strcpy (zigbee_uart, value);
           //             printf("ZIGBEE_UART = \t%s\n",zigbee_uart);
                }
		else if (strcmp(name, "IoT_NO_OF_NODES")==0){
                        no_of_nodes=atoi(value);
                        printf("Number of Nodes = \t%d\n",no_of_nodes);
                }
		else if (strcmp(name, "IoT_ZIGBEE")==0){
                        zig_en=atoi(value);
                        printf("Zigbee enable:\t%d\n",zig_en);
                }
                else if (strcmp(name, "IoT_RS485")==0){
                        rs_en=atoi(value);
                        printf("RS485 enable:\t%d\n",rs_en);
                }
                else if (strcmp(name, "IoT_POLLTIME")==0){
                        poll_time=atoi(value);
                        printf("Poll Time :\t%d\n",poll_time);
                }
                else
                        printf ("WARNING: %s/%s: Unknown name/value pair!\n",
                                        name, value);
        }
	printf("****************************************************\n\n");
        /* Close file */
        fclose (fp);
}
