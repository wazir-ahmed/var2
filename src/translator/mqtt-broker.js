var mosca = require('mosca');
var net = require('net');
var parser = require('./v2-parser');

const HOST = '127.0.0.1';
const PORT =  1121;
const MQTT_PORT =  8892;

const V2_VER = 1;
const V2_PACKT_PUB = 4;
const V2_PACKT_SUB = 6;

var settings = {
    port: MQTT_PORT,
};

function extract_topic(packet) {
    var topic =  packet.slice(1,5);
    var x = 0;
    for(var i = 0; i < topic.length; i++) {
        x = x << 8;
        x = x | topic[i];
    }
    x =  x >> 1;
    return x.toString();
}

function extract_payload(packet) {
    var payload = packet.slice(12);
    var x = Buffer.from(payload.buffer);
    return x;
}

function pack_sub_packet(topic) {
    var subPacket = new Buffer(5);
    subPacket.writeUInt8(parser.pack_ver_packt(V2_VER, V2_PACKT_SUB));
    subPacket.writeUInt32BE(parser.pack_addr_contf(parseInt(topic), 0), 1);
    return subPacket;
}

function pack_pub_packet(topic, payload) {
    var pubPacket = new Buffer(12 + payload.length);
    pubPacket.writeUInt8(parser.pack_ver_packt(V2_VER, V2_PACKT_PUB));
    pubPacket.writeUInt32BE(parser.pack_addr_contf(parseInt(topic), 1), 1);
    pubPacket.writeUInt32BE(payload.length, 5);
    pubPacket.writeUInt8(0, 9);
    pubPacket.writeUInt16BE(0, 10);
    pubPacket.write(payload, 12);
    return pubPacket;
}

var sock = new net.Socket();
var broker = new mosca.Server(settings);

sock.connect(PORT, HOST, function() {
    console.log("Broker has Connected to TCP Core..");
    sock.name = sock.remoteAddress + " : " + sock.remotePort;

    sock.on('data', function(data) {
        parser.packetify(sock, data);
        while(sock.packBuf.length > 0) {
            var packet = sock.packBuf.shift();
            var payload = extract_payload(packet);
            var topic = extract_topic(packet);
            var message = {
                topic: topic,
                payload: payload,
                qos: 0,
                retain: false
            };
            broker.publish(message);

            console.log('\n-------------');
            console.log('Broker has Published on TOPIC : ', topic, '\tPAYLOAD : ', payload);
            console.log('-------------\n');
        }
    });
});

broker.on('ready', function() {
    console.log('MQTT Broker is running..');
});

broker.on('clientConnected', function(client) {
    console.log('Client with ID - ', client.id, 'has connected..');
});

broker.on('subscribed', function(topic, client) {
    console.log('Client with ID - ', client.id, " has subscribed to TOPIC - ", topic);
    var subPacket = pack_sub_packet(topic); 
    sock.write(subPacket);
});

broker.on('unsubscribed', function(topic, client) {
    console.log('Client with ID - ', client.id, " has unsubscribed from TOPIC - ", topic);
});

broker.on('published',function(packet, client) {
    if(client) {
        console.log('\n-------------');
        console.log('Client with ID - ', client.id, " has Published to TOPIC - ", packet.topic);
        console.log("PAYLOAD : ", packet.payload);
        console.log('-------------\n');

        var pubPacket = pack_pub_packet(packet.topic, packet.payload.toString());   
        sock.write(pubPacket);
    }
});
