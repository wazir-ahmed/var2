var mqtt = require('mqtt');

var client = mqtt.connect('mqtt://localhost:8892');

var defaultTopic = '1234';

client.on('connect', function() {
    console.log('MQTT Client has connected to the MQTT Broker..');
    console.log('Client is publishing data on topic //1234//.. ');
    client.publish(defaultTopic, 'hello1');
    client.publish(defaultTopic, 'hello2');
    client.publish(defaultTopic, 'hello3');
    console.log('Client has finished publishing data on topic //1234//.. ');
});

client.on('message', function(topic, message) {
    console.log("Received message on TOPIC : ", topic, "\tPAYLOAD : ", message.toString(), "\n");
});
