var mqtt = require('mqtt');

var client = mqtt.connect('mqtt://localhost:8892');

var defaultTopic = '1234';

client.on('connect', function() {
    console.log('MQTT Client has connected to the MQTT Broker..');
    client.subscribe(defaultTopic, function(err) {
        if(!err) {
            console.log('Client has subscribed to TOPIC : ', defaultTopic);
        }
    });

});

client.on('message', function(topic, message) {
    console.log('\n-----------------');
    console.log("Received message on TOPIC : ", topic, "\tPAYLOAD : ", message.toString());
    console.log('-----------------\n');
});