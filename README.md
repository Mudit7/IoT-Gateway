# IoT Gateway:
---------------------
The Gateway polls nodes for temperature and humidity in KM Protocol Format and uses MQTT Protocol to publish the same.
The MQTT subscriber then uploads this data to a MYSQL Database server.
Python script monitors the firebase server and updates the config file if any changes are encountered.

### Prerequisites:
---------------------
Publisher:

*libmosquitto*       ->  `sudo apt-get install libmosquitto`


Subscriber:

*libmysqlclient*     ->  `sudo apt-get install libmysqlclient`

*libmosquitto*       ->  `sudo apt-get install libmosquitto`

### KM IoT Protocol Format	Gateway-Node Communication
```
Gateway: 
tx: 	<start><node_id><function_code><end>
	start: '<'
	node_id: Id of the node
	function_code: 'T' for temperature and 'H' for Humidity
	end:'>'

rx: 	<start><node_id><function_code><data><end>
	start:'<'
	node_id: Id of the node transmitting the data
	function_code: 'T' or 'H'
	data:Temperature or humidity value
	end:'>'
```

## Deployment:
---------------------
run the automake.sh with superuser 

`sudo ./automake.sh`



## Authors:
---------------------
**Mudit Malpani**			-			(muditmalpani1997@gmail.com)

**Teerna Mukhopadhyay**			-			(teerna96@gmail.com)

**Kishore Kumar Boddu (Mentor)**	-			(kernelmasters@gmail.com)

