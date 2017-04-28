var parse_contf = function(x) {
    return x & 0b00000001;
}

var pack_ver_packt = function(ver, packt) {
    var x = ver << 4;
    x = x | packt;
    return x;
}

var pack_addr_contf = function(addr, contf) {
    var x = addr << 1;
    x = x | contf;
    return x;
}

var packetify = function(sock, data) {
    var newBuf, packet, contf, totalBufLen, payloadLen, packetLen;

     if(!sock.packBuf) {
        sock.packBuf = [];
    }
    
    if(sock.sockBuf) {
        totalBufLen = sock.sockBuf.length + data.length;
        newBuf = Buffer.concat([sock.sockBuf, data], totalBufLen); 
    }
    else {
        newBuf = data;
    }

    while(newBuf.length > 4) {
        contf = parse_contf(newBuf.readUInt8(4));
        if(contf == 0) {
            packet = Uint8Array.from(newBuf.slice(0, 5));
            sock.packBuf.push(packet);
            newBuf = newBuf.slice(5);
        }
        else {
            payloadLen = newBuf.readUInt32BE(5);
            packetLen = 12 + payloadLen;
            if(newBuf.length >= packetLen) {
                packet = Uint8Array.from(newBuf.slice(0, packetLen));
                sock.packBuf.push(packet);
                newBuf = newBuf.slice(packetLen);
            } 
            else {
                break;
            }
        }
    }
    sock.sockBuf = newBuf;
    return; 
}

module.exports = {
    parse_contf: parse_contf,
    pack_ver_packt: pack_ver_packt,
    pack_addr_contf: pack_addr_contf,
    packetify: packetify
}