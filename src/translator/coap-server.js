var net  = require('net');
var url  = require('url');
var qs   = require('querystring');
var parser = require('./v2-parser');
var coap        = require('coap')
  , server      = coap.createServer();

const HOST = '127.0.0.1';
const PORT =  1121;

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

server.on('get', function(req, res) {
  console.log('GET req received.\n', 'URL - ' + req.url);
  
  var addr = get_addr_from_url(req.url);
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
      res.write(payload.toString('utf8'));

      console.log('\n---------------');
      console.log('Server has sent data to client');
      console.log('ADDR - ', addr, '\tDATA : ', payload.toString('utf8'));
      console.log('---------------\n');
    }
    res.end();
    client.end();
  });
})

server.on('post', function(req, res) {
  console.log('POST req received.\n', 'URL - ' + req.url);
  console.log('Payload - \n' + req.payload);
  
  var addr = get_addr_from_url(req.url);

  var payload = qs.parse(req.payload.toString('utf8')).data;
  
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

  res.end('success');
})

server.on('request', function(req, res) {
  if(req.method == 'GET') 
    server.emit('get', req, res);
  else if (req.method == 'POST')
    server.emit('post', req, res);
})

// the default CoAP port is 5683
server.listen(function() {
  console.log('Server started..!');
})
