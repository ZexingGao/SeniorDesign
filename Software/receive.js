/*var http = require('http');
var express = require('express');
const MessagingResponse = require('twilio').twiml.MessagingResponse;
var app = require('express')();

const accountSid = 'AC4c5669b65a5adfd26d1b2082a2b8baf4';
const authToken = '0c26716141831599965530caea4df03d';
var twilio = require('twilio');
var client = new twilio(accountSid, authToken);
//const app = express();

app.post('/sms', (req, res) => {
	const twiml = new MessagingResponse();

	client.messages
  .create({
  	body: 'SBSBSBSBSBSBSB!!!!!',
  		from: '+18572148417',
  		to: '+18572723466'
  		})
	 .then(message => console.log(message.sid))
	 .done();

	  res.writeHead(200, {'Content-Type': 'text/xml'});
  res.end(twiml.toString());
});

http.createServer(app).listen(1337, () => {
  console.log('Express server listening on port 1337');
});
*/
var express = require('express');
var bodyParser = require('body-parser');
var app = express();
app.use(bodyParser.urlencoded({extended: false}));
const accountSid = 'AC4c5669b65a5adfd26d1b2082a2b8baf4';
const authToken = '0c26716141831599965530caea4df03d';
var twilio = require('twilio');
var client = new twilio(accountSid, authToken);
const path = require('path');
var http = require('http').Server(app);
// ---------- HTTP Web Server Part ----------- // 
app.use(express.static(path.join(__dirname, 'public')));
app.set('view engine', 'ejs');
// // css link requirement
app.use(express.static("."));

app.get('/', function (req, res) {
    res.sendFile(__dirname + '/home.html');
});
app.get('/', function (req, res) {
    res.sendFile(__dirname + '/home.html');
});


const admin = require('firebase-admin');
const serviceAccount = require('./running-safety-firebase-adminsdk-incdh-1536c1ed15.json');
admin.initializeApp({
    credential: admin.credential.cert(serviceAccount)
});
const db = admin.firestore();

http.listen(8080, function () {
    console.log('web server listen on *: 8080');
})
app.post('/sms',function(req, res){
	console.log(req.body);
	var msgFrom = req.body.From;
    var msgBody = req.body.Body;
    var ID = msgBody[15];
    var location = msgBody.slice(70,89);//get the gps location
    console.log("location: ", location);
    console.log("ID = ", ID);
    var mynumber = '';
    var citiesRef = db.collection('users');
    var allCities = citiesRef.get()
        .then(snapshot => {
            snapshot.forEach(doc => {
                if (doc.data().device == ID) {
                //if (doc.data().econtact == '6304926259') {
                    mynumber = doc.data().econtact;
                    console.log(mynumber);  
                    client.messages
                        .create({
                            body: msgBody + " from " + msgFrom,
                            from: '+18572148417',
                            to: mynumber
                        })
                        .then(message => console.log(message.sid))
                        .done();
                }

            });
        })
        .catch(err => {
            console.log('Error getting documents', err);
        });
    
});

app.listen(1337);
    //get number
    /*
    var mynumber = '';
    var citiesRef = db.collection('users');
    var allCities = citiesRef.get()
        .then(snapshot => {
            snapshot.forEach(doc => {
                if (doc.data().econtact == "6304926259") {

                    mynumber = doc.data().econtact;

                }

            });
        })
        .catch(err => {
            console.log('Error getting documents', err);
        });
    client.messages
      .create({
          body: msgBody +" from "+ msgFrom,
          from: '+18572148417',
  		    to: mystring
  		    })
	     .then(message => console.log(message.sid))
	     .done();
    });

app.listen(1337);

*/
