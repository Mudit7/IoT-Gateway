/***************************************************************************
 *      Organisation    : Kernel Masters, KPHB, Hyderabad, India.          *
 *      Website         : www.kernelmasters.org                            *
 *      facebook page   : www.facebook.com/kernelmasters                   *
 *                                                                         *
 *  Conducting Workshops on - Embedded Linux & Device Drivers Training.    *
 *  -------------------------------------------------------------------    *
 *  Tel : 91-9949062828, Email : kishore@kernelmasters.org                 *
 ***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation. No warranty is attached; we cannot take *
 *   responsibility for errors or fitness for use.                         *
 ***************************************************************************/
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <mosquitto.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <my_global.h>
#include <mysql.h>
 
static int run = 1;
float adc_float;
 
struct mosquitto *mosq = NULL;
int keepalive = 60;
bool clean_session = true;

#define MAXLEN 80
#define CONFIG_FILE "/home/km/KM_GIT/iot/gateway/config" 
#define MYSQL_HOST "192.168.1.113"
#define MYSQL_USER "root"
#define MYSQL_PASSWD "teerna"

char MQTT_HOST[MAXLEN];
int  MQTT_PORT;
char MQTT_TOPIC[MAXLEN];

int no_of_nodes=2;  //default
int i=0;
MYSQL *con=NULL;
char recv_buf[100];

void param_config(void);
char* trim (char * );

void parse_options(int argc, char** argv); 

void finish_with_error(MYSQL *con) {
        fprintf(stderr, "%s\n", mysql_error(con));
        mysql_close(con);
        write(1,recv_buf,1);
        exit(1);
}

 
void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) {
        int size,fd1,val;
	/*char sql_query1[150]="use iotsmartgateway;\n update monitoring set `value`=";
	char sql_query2[40]=" where `parameters`=\"Temperature\";";
	char sql_query4[12]={0};
	char sql_query[150]={0};

	printf("got message '%.*s' for topic '%s'\n", message->payloadlen, (char*) message->payload, message->topic);
 
	//mosquitto_topic_matches_sub("/devices/wb-adc/controls/+", message->topic, &match);
	//if (match) {
	//printf("got message for ADC topic\n");
	// }
	val = atoi((char*)message->payload);
	printf("val:%d\n",val);*/
	
	 char sql_query[150]={0};
              //  write(1,"before sprintf\n",16);
		strcpy(recv_buf,message->payload);
		if(recv_buf[1]=='T')
                sprintf(sql_query,"UPDATE weather SET Temperature=%c%c,Date=NOW() WHERE Node_ID=%c",recv_buf[2],recv_buf[3],recv_buf[0]);
	        else if(recv_buf[1]=='H')
                sprintf(sql_query,"UPDATE weather SET Humidity=%c%c,Date=NOW() WHERE Node_ID=%c",recv_buf[2],recv_buf[3],recv_buf[0]);
		else if(recv_buf[1]=='O')
                sprintf(sql_query,"UPDATE weather SET Node_Status='%s',Date=NOW() WHERE Node_ID=%c","OFFLINE",recv_buf[0]);
		else
                sprintf(sql_query,"UPDATE weather SET Date=NOW() WHERE 1");
                
		if (mysql_query(con,sql_query)) {
                        finish_with_error(con);
                 };
                printf("got message '%.*s' for topic '%s'\n", message->payloadlen, (char*) message->payload, message->topic);
               // printf("val:%d\n",val);
}
 
int main() {

	uint8_t reconnect = true;
	char clientid[24];
	int rc = 0;

	char sql_query[150]={0};
        param_config(); //Reading config file
 
	mosquitto_lib_init();

	mosq = mosquitto_new(NULL, true, 0);
        printf("mosq:%p\n",mosq);
        
	con=mysql_init(NULL);
        if (con == NULL)
        {
                fprintf(stderr, "%s\n", mysql_error(con));
                exit(1);
        }

	rc = mosquitto_connect(mosq,"localhost",1883, keepalive);
	
	if (mysql_real_connect(con,MYSQL_HOST, MYSQL_USER, MYSQL_PASSWD,
          "testdb", 0, NULL, 0) == NULL)
        {
                finish_with_error(con);
        }

        if (mysql_query(con, "DROP TABLE IF EXISTS weather")) {
                finish_with_error(con);

        }

        if (mysql_query(con, "CREATE TABLE weather(Node_ID VARCHAR(1), Temperature VARCHAR(30),Humidity VARCHAR(30),Node_Status VARCHAR(30), Date DATE)")) {
                finish_with_error(con);
        }
	
		
	for(i=1;i<=no_of_nodes;i++)
	{
	sprintf(sql_query, "INSERT INTO weather VALUES ('%d','00','00','ACTIVE','2018-04-20')",i);
        if (mysql_query(con,sql_query)) 
                finish_with_error(con);
        } 
 	       
	mosquitto_message_callback_set(mosq, message_callback);

        mosquitto_subscribe(mosq, NULL,"/weather", 0);
 
        while(1)
	{
		rc = mosquitto_loop(mosq, -1, 1);
		if(run && rc)
		{
			printf("connection error!\n");
			sleep(10);
			mosquitto_reconnect(mosq);
            	}
        }
	mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	mysql_close(con);
	return rc;
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
                        strcpy (name, s);
                s = strtok (NULL, "=");
                if (s==NULL)
                        continue;
                else
                        strcpy (value, s);
                trim (value);

                if (strcmp(name, "IoT_NO_OF_NODES")==0){
                        no_of_nodes=atoi(value);
                        printf("Number of Nodes = \t%d\n",no_of_nodes);
                }
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
