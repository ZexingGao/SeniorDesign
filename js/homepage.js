var db = firebase.firestore();

//Add Contacts Button
function register()   {
    document.getElementById('add').hidden = true;
    document.getElementById('devicenum').hidden = false;
    document.getElementById('econtact').hidden = false;
    document.getElementById('submit').hidden = false;
}

//Write DB Data
function submit() {
    var contact = document.getElementById('econtact').value;
    var devicenum = document.getElementById('devicenum').value;
    var userId = firebase.auth().currentUser.uid;
    db.collection("users").doc(userId.toString()).set({
        econtact: contact ,
        device: devicenum
    })
        .then(function() {
            alert("You have successfully registered your device and Contact!");
        })
        .catch(function(error) {
            alert(error);
        });
}



function JumpBackToLogin() {
    window.location = 'login.html';
}
//SignOut function
function signOut() {
    document.getElementById('outbutt').addEventListener('click', JumpBackToLogin, false);
    firebase.auth().signOut().then(function() {
        // Sign-out successful.
    }).catch(function(error) {
        // An error happened.
    });

}

//Every page Load
function nextInit() {
    firebase.auth().onAuthStateChanged(function(user) {
        if (user) {
            document.getElementById('message').textContent = "Welcome!";
        }
        else {
            document.getElementById('message').textContent = "Signed Out, thank you!";
        }
    });

    document.getElementById('outbutt').addEventListener('click', signOut, false);
}

window.onload = function() {
    nextInit();
};


//Separate Read DB Data Snippet for Twilio
/*
var userId = firebase.auth().currentUser.uid;
var docRef = db.collection("users").doc(userId.toString());
docRef.get().then(function(doc) {
    if (doc.exists) {
        var mystring = JSON.stringify(doc.data().econtact, null, '  ')
        mystring = mystring.replace(/^"(.*)"$/, '$1');
        //apply mystring to the contact of twilio
    } else {
        // doc.data() will be undefined in this case
        alert("No data");
    }
}).catch(function(error) {
    alert(error);
});*/