var coap = require('coap')
var qs = require('querystring')

function printRes(res) {
    console.log(res.payload.toString('utf8'))
}

function extractRes(res) {
    return res.payload.toString('utf8')
}

var post_req = coap.request({
    hostname: 'localhost',
    method: 'POST',
    pathname: '/1234',
    options: { "Content-Format": "application/json" }
})

var payload = {
    data: "hello"
}
  
post_req.write(qs.stringify(payload))

post_req.on('response', function(res) {
    if(extractRes(res) === 'success') {
        console.log('The post request is successful..')
    }
    else {
        console.log('The post request failed..')
        console.log('actual res -' + extractRes(res))
    }
    
})

post_req.end()
