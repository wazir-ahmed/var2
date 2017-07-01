var coap = require('coap')

function printRes(res) {
    console.log(res.payload.toString('utf8'))
}

function extractRes(res) {
    return res.payload.toString('utf8')
}

var get_req = coap.request('coap://localhost/1234')

get_req.on('response', function(res) {
    printRes(res)
})

get_req.end()

