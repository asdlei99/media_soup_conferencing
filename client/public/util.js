'use strict'

function show_msg(msg) {
    const NEW_LINE = '</br>';
    let time = new Date();
    msg = time.getMinutes() + ':' + time.getSeconds() + ':' + time.getMilliseconds() + '=' + msg;
    document.getElementById("debug").innerHTML += msg;
    document.getElementById("debug").innerHTML += NEW_LINE;
}

module.exports = show_msg;