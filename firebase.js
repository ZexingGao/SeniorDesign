
//Sign In button Function
function SignIn() {
    if (firebase.auth().currentUser) {
        firebase.auth().signOut();
    }
    else{
        var email = document.getElementById('email').value;
        var password = document.getElementById('password').value;

        //need to error check

        firebase.auth().signInWithEmailAndPassword(email, password).catch(function(error) {
            //Catch all errors with sign-in
            var errorCode = error.code;
            var errorMessage = error.message;
            if (errorCode == 'auth/wrong-password') {
                alert('Wrong Password.');
            }
            else {
                alert(errorMessage);
            }
            console.log(error);
            document.getElementById('sign-in').disabled = false;
        });

    }
    document.getElementById('sign-in').disabled =true;
}

//SignUp button function
function SignUp() {
    var email = document.getElementById('email').value;
    var password = document.getElementById('password').value;
    //error check?

    firebase.auth().createUserWithEmailAndPassword(email, password).catch(function(error) {
        //Catch all errors with sign-up
        var errorCode  = error.code;
        var errorMessage = error.message;

        if(errorCode == 'auth/weak-password'){
            alert('Password not valid');
        }
        else{
            alert(errorMessage);
        }
        console.log(error);
    });
}

/*function EmailVerify() {
    firebase.auth().currentUser.sendEmailVerification().then(function() {
        alert('Email Verification Sent!');
    });
}*/

//Initialize application every page load
function initApp() {
    //detects change in authentication state
    firebase.auth().onAuthStateChanged(function(user) {
        if (user) {
            var displayName = user.displayName;
            var email = user.email;
            var uid = user.uid;

            document.getElementById('sign-in').textContent = 'Sign out';
            document.getElementById('sign-up').hidden = true;
            document.getElementById('email').hidden = true;
            document.getElementById('password').hidden = true;
            document.getElementById('first-text').textContent = 'Welcome!';
            document.getElementById('name-text').textContent = JSON.stringify(user.displayName, null, '  ');
            document.getElementById('email-text').textContent = JSON.stringify(user.email, null, '  ');
            document.getElementById('uid-text').textContent = JSON.stringify(user.uid, null, '  ');
        }
        else {
            document.getElementById('sign-in').textContent = 'Sign in';
            document.getElementById('sign-up').hidden = false;
            document.getElementById('first-text').textContent = 'Enter your email and password to either sign into your account or sign up!';
            document.getElementById('name-text').textContent = ' ';
            document.getElementById('email-text').textContent = ' ';
            document.getElementById('uid-text').textContent = ' ';
        }
        document.getElementById('sign-in').disabled = false;
    });

    document.getElementById('sign-in').addEventListener('click', SignIn, false);
    document.getElementById('sign-up').addEventListener('click', SignUp, false);
}

window.onload = function() {
    initApp();
};
