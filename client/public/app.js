'use strict';

function get_user_name() {
    let user = document.getElementById("local").value;
    console.log(user);
    return user;
}

function trace(txt, isError) {

    if (isError != undefined && isError == true)
        console.error(txt);
    else
        console.log(txt);
}

function show_msg(msg) {
    const NEW_LINE = '</br>';
    let time = new Date();
    msg = time.getMinutes() + ':' + time.getSeconds() + ':' + time.getMilliseconds() + '=' + msg;
    document.getElementById("debug").innerHTML += msg;
    document.getElementById("debug").innerHTML += NEW_LINE;
}

function clearMsg() {
    document.getElementById("debug").innerHTML = '';
}

function get_server_url() {
    let server = document.getElementById("server").value;
    var serverString = 'ws://' + server;
    return serverString;
}

function get_peer_name(){
   return  document.getElementById("user").value;
}

function get_room_id(){
    return document.getElementById("roomId").value;
}

let media_soup_conference_ = null;

function stream_observer(type, id, kind, stream){
    if ('receive' == type) {
        if (kind == 'video') {
            show_msg("video recive "+ id);
            const video = document.createElement('video');
            video.setAttribute('style', 'max-width: 400px;');
            video.setAttribute('playsinline', '');
            video.srcObject = stream;
            document.getElementById('container').appendChild(video);
            video.play();
        }
        else if (kind == 'audio') {
            show_msg("audio recive "+id);
            const audio = document.createElement('audio');
            audio.srcObject = stream;
            document.getElementById('container').appendChild(audio);
            audio.play();
        }
        else {
            //todo: it should never come here
            
        }
    }
    else if('close' == type){
        show_msg("handel close message for consumer id " + id + "kind = "+kind);
    }
}

function signIn() {
    let server_addr = get_server_url();
    let room_id = get_room_id();
    let peer_name = get_peer_name();
    let signaller = new conference_signalling(server_addr, room_id, peer_name);
    signaller.start(peer_name=>{
        media_soup_conference_ = new media_soup_conference(stream_observer);
        media_soup_conference_.start(peer_name, signaller);
    });
}

function connect() {
    console.log("connect called");

    var server = document.getElementById("server").value;

    if (server.length == 0) {
        console.log("I need server address.");
        document.getElementById("server").focus();
    }
    else {
        signIn();
    }
}

document.querySelector('#ServerConnect').addEventListener('click', function (e) {

    connect();
});



document.querySelector('#clearLog').addEventListener('click', function (e) {
    clearMsg();
});

document.querySelector('#stop').addEventListener('click', function (e) {
        show_msg("sending stop to ");
        if(media_soup_conference_ != null){
            media_soup_conference_.stop();
        }
        
        media_soup_conference_ = null;
    //audio_conf_handler = null;
});

