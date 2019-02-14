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

app.post('/sms',function(req, res){
	console.log(req.body);
	var msgFrom = req.body.From;
	var msgBody = req.body.Body;
	
client.messages
  .create({
  	body: msgBody,
  		from: '+18572148417',
  		to: '+18572723466'
  		})
	 .then(message => console.log(message.sid))
	 .done();
});

app.listen(1337);


