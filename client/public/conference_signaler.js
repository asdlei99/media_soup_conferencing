'use strict';


function create_conferencing_url(server_url, room_id, peer_name) {
    //ip:port/?roomId=id?peerName=name;
    return server_url + "/?roomId=" + room_id + "&peerName=" + peer_name;
}

class conference_signalling {
    constructor(server_address, room_id, peer_name) {
        this.server_address = server_address;
        this.room_id = room_id;
        this.peer_name = peer_name;
    }
    start(succcess) {
        let url = create_conferencing_url(this.server_address,
            this.room_id, this.peer_name);
        show_msg("url = " + url);
        this.socket = new WebSocket(url);

        this.socket.onopen = () => {
            show_msg("connect to server success");
            succcess(this.peer_name);
        };

        this.socket.onerror = err => {
            show_msg("server connection error " + err);
        };
        this.socket.onclose = () => {
            show_msg("sever connection closed");
        };

    }

    send(msg) {
        this.socket.send(msg);
    }

    register_callback(callback) {
        this.socket.onmessage = callback;
    }
}