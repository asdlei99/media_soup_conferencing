function get_user_name(){
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

function signIn() {
    let server_addr = get_server_url();
    let room_id = get_room_id();
    let peer_name = get_peer_name();
    media_soup_conference_ = new media_soup_conference()
    media_soup_conference_.start(server_addr, room_id, peer_name);
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

