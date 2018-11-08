
var express = require('express'),
    list = require('./request.js').Request; // see  template

var app = express.createServer();

app.use(express.static(__dirname + '/public')); // exposes index.html, per below

app.get('/request', function(req, res){
    // run your request.js script
    // when index.html makes the ajax call to www.yoursite.com/request, this runs
    // you can also require your request.js as a module (above) and call on that:
    res.send(list.getList()); // try res.json() if getList() returns an object or array
});

app.listen(80);

//Twilio part
const accountSid = 'AC89b77c7c958d00674ce7b94e94f4f431';
const authToken = '5b5e7ae18fed283255ad007c82600226';
const client = require('twilio')(accountSid, authToken);

client.messages
    .create({
        body: 'This is the ship that made the Kessel Run in fourteen parsecs?',
        from: '+16174874030',
        to: '+18572723466'
    })
    .then(message => console.log(message.sid))
    .done();


