'use strict';
const conference_signalling = require('./conference_signaler');
const show_msg = require('./util');
const media_soup_conference = require('./media_soup_conference');

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

function start_streaming(peer_name, signaller){
    media_soup_conference_ = new media_soup_conference(stream_observer);
    media_soup_conference_.start(peer_name, signaller);
}

function start_subsribe(peer_name, signaller){
    media_soup_conference_ = new media_soup_conference(stream_observer);
    media_soup_conference_.start_consumer(peer_name, signaller);
}

function signIn(callback) {
    let server_addr = get_server_url();
    let room_id = get_room_id();
    let peer_name = get_peer_name();
    let signaller = new conference_signalling(server_addr, room_id, peer_name);
    signaller.start(peer_name=>{
        callback(peer_name, signaller);
    });
}

function connect(callback) {
    console.log("connect called");

    var server = document.getElementById("server").value;

    if (server.length == 0) {
        console.log("I need server address.");
        document.getElementById("server").focus();
    }
    else {
        signIn(callback);
    }
}

function parse_room_manager_msg(jmsg){
    if(jmsg.type == 'room_open_response'){
        if(jmsg.status == "success"){
            show_msg("room creation success");
            document.getElementById("roomId").value = jmsg.id;
        }
    }
    else if(jmsg.type == 'room_close_response'){
        show_msg("room close response = " + jmsg.status);
    
    }
    else if(jmsg.type == "response_room_info"){

        show_msg("room info rooms count = " + jmsg.count);
        show_msg("rooms id ");
        jmsg.ids.forEach(show_msg);
        jmsg.names.forEach(show_msg);
    }
    else{
        show_msg("this is not expeced "+ JSON.stringify(jmsg));
    }
}

function send_room_manager_msg(serverIp, msg){
    let socket = new WebSocket("ws://" + serverIp);

    socket.onopen = () => {
        show_msg("connect to room server success");
        socket.send(msg);
    };

    socket.onerror = err => {
        show_msg("server connection error " + err);
    };
    socket.onclose = () => {
        show_msg("sever connection closed");
    };
    socket.onmessage = evt=>{
        let jmsg = JSON.parse(evt.data);
        parse_room_manager_msg(jmsg);
        socket.close();
        socket = null;
    }
}

function create_room(serverIp, roomName){
    send_room_manager_msg(serverIp, JSON.stringify({type:"request_room_open", name:roomName}));
}

function delete_room(serverIP, roomId){
    send_room_manager_msg(serverIP, JSON.stringify({type:'request_room_close', id:roomId}));
}

document.querySelector('#ServerConnect').addEventListener('click', function (e) {

    connect(start_streaming);
});

document.querySelector('#Subscribe').addEventListener('click', function (e) {

    connect(start_subsribe);
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



document.querySelector('#roomCreate').addEventListener('click', function (e) {
    const roomManagerServer = document.getElementById('roomManager').value;
    if(roomManagerServer.length == 0){
        console.error("put server address");
        document.getElementById('roomManager').focus();
        return;
    }
    const roomName = document.getElementById("roomName").value;
    if(roomName.length == 0) {
        console.error("put room name");
        document.getElementById("roomName").focus();
    }
    else{
        create_room(roomManagerServer, roomName);
    }
   
});

document.querySelector('#deleteRoom').addEventListener('click', function (e) {
    const roomManagerServer = document.getElementById('roomManager').value;
    if(roomManagerServer.length == 0){
        console.error("put server address");
        document.getElementById('roomManager').focus();
        return;
    }
    const roomId = document.getElementById("roomId").value;
    if(roomId.length == 0) {
        console.error("no room id");
        document.getElementById("roomId").focus();
        return;
    }
    else{
        delete_room(roomManagerServer, roomId);
    }
   
});

document.querySelector('#getRoomsInfo').addEventListener('click', function (e) {
    const roomManagerServer = document.getElementById('roomManager').value;
    if(roomManagerServer.length == 0){
        console.error("put server address");
        document.getElementById('roomManager').focus();
        return;
    }
    send_room_manager_msg(roomManagerServer, JSON.stringify({type:"request_rooms_info"}));
});

