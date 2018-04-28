from firebase import firebase
import time

firebase_url='https://iot-gateway-44d12.firebaseio.com/'
firebase = firebase.FirebaseApplication(firebase_url)

def getset_data():
    key=[]
    attri=[]
    attri.append('RS485_UART')
    key.append(str('/dev/ttyO4'))

    attri.append('ZIGBEE_UART')
    key.append(str('/dev/ttyO5'))
    
    result=firebase.get('nodes/number',None)
    key.append(str(result))
    attri.append('IoT_NO_OF_NODES')
    
    result=firebase.get('Gateway/RS485_EN',None)
    key.append(str(result))
    attri.append('IoT_RS485')

    result=firebase.get('Gateway/Zigbee_EN',None)
    key.append(str(result))
    attri.append('IoT_ZIGBEE')

    result=firebase.get('nodes/poll_time',None)
    attri.append('IoT_POLLTIME')
    key.append(str(result))

    attri.append('MQTT_HOST')
    key.append('localhost')

    attri.append('MQTT_PORT')
    key.append(str(1883))

    attri.append('MQTT_TOPIC')
    key.append(str('/weather'))


    global F
    for i in range(9):
        F.write(str(attri[i]))
        F.write('=')
        F.write(str(key[i]))
        F.write('\n')

while True:
    result=firebase.get('Config/Ready',None)
    if(result==1):
        print "updating now"
        firebase.put('Config','Ready',0)
        F = open("config","w") 
        F.truncate()
        getset_data()
        F.close()
    else:
       # print "python sleeping"
        time.sleep(2)
