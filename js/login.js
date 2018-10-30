function hasClass(elem, cls) {
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
document.querySelector(".login-button").onclick = function(){
    addClass(document.querySelector(".login"), "active")
    setTimeout(function(){
        addClass(document.querySelector(".sk-rotating-plane"), "active")
        document.querySelector(".login").style.display = "none"
    },50)
    setTimeout(function(){
        removeClass(document.querySelector(".login"), "active")
        removeClass(document.querySelector(".sk-rotating-plane"), "active")
        document.querySelector(".login").style.display = "block"
        alert("Login Success")
        window.location.href='home.html';
    },1000)

}