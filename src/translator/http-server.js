var net  = require('net');
var http = require('http');
var url  = require('url');
var qs   = require('querystring');
var parser = require('./v2-parser');

const HOST = '127.0.0.1';
const PORT =  1121;
const HTTP_PORT = 8080;

const V2_VER = 1;
const V2_PACKT_PUB = 4;
const V2_PACKT_FET = 5;

function get_addr_from_url(reqUrl) {
  var x = url.parse(reqUrl).pathname;
  x = x.replace('/',0);
  x = parseInt(x);
  return x;
}

function extract_payload(packet) {
  var payload = packet.slice(12);
  var x = Buffer.from(payload.buffer);
  return x;
}

function pack_fetch_packet(addr) {
  var fetPacket = new Buffer(5);
  fetPacket.writeUInt8(parser.pack_ver_packt(V2_VER, V2_PACKT_FET));
  fetPacket.writeUInt32BE(parser.pack_addr_contf(addr, 0), 1);
  return fetPacket;
}

function pack_pub_packet(addr, payload) {
    var pubPacket = new Buffer(12 + payload.length);
    pubPacket.writeUInt8(parser.pack_ver_packt(V2_VER, V2_PACKT_PUB));
    pubPacket.writeUInt32BE(parser.pack_addr_contf(addr, 1), 1);
    pubPacket.writeUInt32BE(payload.length, 5);
    pubPacket.writeUInt8(0, 9);
    pubPacket.writeUInt16BE(0, 10);
    pubPacket.write(payload, 12);
    return pubPacket;
}

function http_get(request, response) {
  console.log('GET request - ', request.url);

  var addr = get_addr_from_url(request.url);
  var fetPacket = pack_fetch_packet(addr);

  var client = new net.Socket();
  client.connect(PORT, HOST, function() {
    client.write(fetPacket);
  });

  client.on('data', function(data) {
    parser.packetify(client, data);
    while(client.packBuf.length > 0) {
      var packet = client.packBuf.shift();
      var payload = extract_payload(packet);
      response.write(payload.toString('utf8'));

      console.log('\n---------------');
      console.log('Server has sent data to client');
      console.log('ADDR - ', addr, '\tDATA : ', payload.toString('utf8'));
      console.log('---------------\n');
    }
    response.end();
    client.end();
  });
}

function http_post(request, response) {
  console.log('POST request - ', request.url);

  var addr = get_addr_from_url(request.url);

  var postData = '';
  request.on('data', function(data) {
    postData += data;
  });

  request.on('end', function() {
    var payload = qs.parse(postData).data;
    var pubPacket = pack_pub_packet(addr, payload);
    
    console.log('\n---------------');
    console.log('Server has received data from client');
    console.log('ADDR - ', addr, '\tDATA : ', payload.toString('utf8'));
    console.log('---------------\n');

    var client = new net.Socket();
    client.connect(PORT, HOST, function() {
      client.write(pubPacket);
      client.end();
    });

    response.end('success');
  });
}

var server = http.createServer(function(request, response) {
  if(request.method == 'GET') {
    http_get(request, response);
  }
  else if(request.method == 'POST') {
    http_post(request, response);
  }
});

server.listen(HTTP_PORT);
console.log('HTTP Server running...');