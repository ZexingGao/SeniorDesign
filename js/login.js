


//Sign In button Function
function SignIn() {
    var email = document.getElementById('user').value;
    var password = document.getElementById('pass').value;

    //need to error check

    firebase.auth().signInWithEmailAndPassword(email, password).catch(function(error) {
        var errorCode = error.code;
        var errorMessage = error.message;
        if (errorCode == 'auth/wrong-password') {
            alert('Wrong Password.');
        }
        else {
            alert(errorMessage);
        }
        console.log(error);
    });

}

function NewMem() {
    window.location = 'signup.html';
}


//Initialize application every page load
function initApp() {
    firebase.auth().onAuthStateChanged(function(user) {
        if (user) {
            window.location ='home.html';
        }
        else {
            //nothing for now?
        }
    });

    document.getElementById('logbutt').addEventListener('click', SignIn, false);
    document.getElementById('newbutt').addEventListener('click', NewMem, false);
}

window.onload = function() {
    initApp();
};



/*function hasClass(elem, cls) {
    cls = cls || '';
    if (cls.replace(/\s/g, '').length == 0) return false; //If there is no func in cls, then return false
    return new RegExp(' ' + cls + ' ').test(' ' + elem.className + ' ');
}

function addClass(ele, cls) {
    if (!hasClass(ele, cls)) {
        ele.className = ele.className == '' ? cls : ele.className + ' ' + cls;
    }
}

function removeClass(ele, cls) {
    if (hasClass(ele, cls)) {
        var newClass = ' ' + ele.className.replace(/[\t\r\n]/g, '') + ' ';
        while (newClass.indexOf(' ' + cls + ' ') >= 0) {
            newClass = newClass.replace(' ' + cls + ' ', ' ');
        }
        ele.className = newClass.replace(/^\s+|\s+$/g, '');
    }
}
*/
