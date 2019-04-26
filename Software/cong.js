// Modules
var express = require('express');
var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);
var dgram = require('dgram');
const path = require('path');
const accountSid = 'AC4c5669b65a5adfd26d1b2082a2b8baf4';
const authToken = '0c26716141831599965530caea4df03d';
var twilio = require('twilio');
var client = new twilio(accountSid, authToken);

// ---------- HTTP Web Server Part ----------- // 
app.use(express.static(path.join(__dirname, 'public')));
app.set('view engine', 'ejs');
// // css link requirement
app.use(express.static("."));

app.get('/', function(req, res){
  res.sendFile(__dirname + '/home.html');
});

// ---------------- Listen to HTTP msg ------------------ //

var http_client = require('http');
var fs = require('fs');

const WEB_CLIENT_PORT = 4000

// Loading the index file . html displayed to the client
var server = http_client.createServer(function(req, res) {
    fs.readFile('./home.html', 'utf-8', function(error, content) {
        res.writeHead(200, {"Content-Type": "text/html"});
        res.end(content);
    });
});

var web_io = require('socket.io').listen(server);

// When a client connects, we note it in the console
web_io.sockets.on('connection', function (socket) {
    console.log('A web client is connected!');
});


// Listening msg receive from web client socket
var web_msg
web_io.sockets.on('connection', function (socket) {
    socket.emit('message', 'You are connected!');

    // When the server receives a 鈥渕essage鈥� type signal from the client   
    socket.on('message', function (message) {
        web_msg=message.toString('utf8')
        console.log('Web socket listen on * ' + WEB_CLIENT_PORT);
        console.log('Receive msg from Web client: ' + web_msg);
        setTimeout(UDP_send, 200);
    }); 
});

server.listen(WEB_CLIENT_PORT);



// --------- HTTP Web Server Client connection ------ //
var clientConnected = 0;
io.on('connection', function(data){
    console.log("a user connected");
    clientConnected = 1;
    // readDB();
    data.on('disconnect', function(){
        console.log('user disconnected');
    });
});

// ----------- http connection ----------- //

http.listen(8080,function(){
    console.log('web server listen on *: 8080');
})

//------------------- UDP CLIENT AND SERVER PART -------------------- //
// ---------------- Send UDP Message to ESP ------------------ //

// MAC UDP Host & Port
var MAC_PORT = 3000;
var MAC_HOST = '192.168.0.13';
// ESP UDP Host & Port
var ESP_PORT = 3000;
var ESP_HOST = '192.168.1.106';

var udp_server = dgram.createSocket('udp4');
var udp_client = dgram.createSocket('udp4');

//create UDP server to listen msg from ESP32
udp_server.on('listening', function () {
    var address = udp_server.address();
    console.log('UDP Server listening on ' + address.address + ":" + address.port);
});

var received_msg;
// listening the msg sent from the ESP
udp_server.on('message', function (message, remote) {

    // convert the hex buffer to htf8 string format
    received_msg = message.toString('utf8');

    setTimeout(send, 1000);

      // Send Ok acknowledgement
    udp_server.send("Ok!",remote.port,remote.address,function(error){
        if(error){
          console.log('MEH!');
        }
        else{
          console.log('Sent: Ok!');
        }
    });
    console.log(remote.address + ':' + remote.port +' - ' + message);
});

// Bind server to port and IP
udp_server.bind(MAC_PORT, MAC_HOST);


// ---------- send msg to ESP32 ------ //
// to send the control signal to the remote car
function UDP_send(){
    // forward command
    if (web_msg==='send_msg'){
        /*var message = new Buffer('f');
        udp_client.send(message, 0, message.length, ESP_PORT, ESP_HOST, function(err, bytes) {
            if (err) throw err;
            console.log('UDP message sent to ' + ESP_HOST +':'+ ESP_PORT);*/
            
            client.messages.create({

        body: '测试',
        from: '+18572148417',
        to: '+16304926259'
    })
    
        .then(message => console.log(message.sid));
        //});
    }
    // backward command
    
}

