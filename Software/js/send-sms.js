
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


