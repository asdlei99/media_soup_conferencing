'use strict';


function show_msg(msg) {
    const NEW_LINE = '</br>';
    let time = new Date();
    msg = time.getMinutes() + ':' + time.getSeconds() + ':' + time.getMilliseconds() + '=' + msg;
    document.getElementById("debug").innerHTML += msg;
    document.getElementById("debug").innerHTML += NEW_LINE;
}

function clearMsg(){
    document.getElementById("debug").innerHTML = '';
}

var _id = 0;
function get_id() {
    _id++;
    return _id;
}

var req_res_map = new Map();

function save_req_res(id, req, error) {
    if (req_res_map.has(id)) {
        show_msg("error : id is afeady saved " + id);
    }
    else {
        req_res_map.set(id, [req, error]);
    }
}

function get_req_res(id) {
    var arr = req_res_map.get(id);
    if (req_res_map.delete(id) == false) {
        show_msg("error: id not found " + id);
    }
    return arr;
}


function create_conferencing_url(server_url, room_id, peer_name){
    //ip:port/?roomId=id?peerName=name;
    return server_url + "/?roomId="+room_id+"&peerName="+peer_name;
}

class conference_signalling {
    constructor(server_address, room_id, peer_name){
        this.server_address = server_address;
        this.room_id = room_id;
        this.peer_name = peer_name;
    }
     start(succcess) {
        let url = create_conferencing_url(this.server_address, 
            this.room_id, this.peer_name);
        show_msg("url = "+ url);
        this.socket = new WebSocket(url);

        this.socket.onopen =  ()=>{
            show_msg("connect to server success");
            succcess(this.peer_name);
        };

        this.socket.onerror = err =>{
            show_msg("server connection error " + err);
        };
        this.socket.onclose = ()=>{
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



class media_soup_conference {
    start(server_address, room_id, peer_name) {
        this.peer_name = peer_name;
        this.signaller = new conference_signalling(server_address, room_id, peer_name);
       
        this.signaller.start(()=>{
            this._join_room(peer_name);
        });
    }
    stop(){
        show_msg("going to send close request");
        this.signaller.send(
        JSON.stringify({
            'type': 'request_close'
        }));
    }

    _join_room(peer_name) {
        let sender = this.signaller;
        sender.send(JSON.stringify({
            'type': "request_room_join"
        }));

        // Create a local Room instance associated to the remote Room.
        const room = new mediasoupClient.Room();
        // Transport for sending our media.
        let sendTransport;
        // Transport for receiving media from remote Peers.
        let recvTransport;

        show_msg('ROOM:' + room);

        room.join(peer_name)
            .then((peers) => {
                show_msg('PEERS:' + peers);

                // Create the Transport for sending our media.
                sendTransport = room.createTransport('send');
                // Create the Transport for receiving media from remote Peers.
                recvTransport = room.createTransport('recv');

                peers.forEach(peer => handlePeer(peer));
            })
            .then(() => {
                // Get our mic and camera
                show_msg("getting mic and camera");
                return navigator.mediaDevices.getUserMedia({
                    audio: true,
                    video: false
                });
            })
            .then((stream) => {
                const audioTrack = stream.getAudioTracks()[0];
               // const videoTrack = stream.getVideoTracks()[0];

                // Create Producers for audio and video.
                const audioProducer = room.createProducer(audioTrack);
               // const videoProducer = room.createProducer(videoTrack);

                // Send our audio.
                audioProducer.send(sendTransport);
                // Send our video.
                //videoProducer.send(sendTransport);
            })
            .catch((err) => {
                show_msg("err" + err);
            });

        // Event fired by local room when a new remote Peer joins the Room
        room.on('newpeer', (peer) => {
            show_msg('A new Peer joined the Room:' + peer.name);
            // Handle the Peer.
            handlePeer(peer);
        });

        // Event fired by local room
        room.on('request', (request, callback, errback) => {
            show_msg('REQUEST:' + request);
            let id = get_id();
            save_req_res(id, callback, errback);
            sender.send(
                JSON.stringify({
                    'type': 'mediasoup-request',
                    "id": id,
                    "request": request
                }));
        }
        );

        // Be ready to send mediaSoup client notifications to our remote mediaSoup Peer
        room.on('notify', (notification) => {
            show_msg('New notification from local room:' + notification);
            sender.send(JSON.stringify({
                "type": 'mediasoup-notification',
                "m": notification
            }));
        });

        //message received from socket(websocket server)
        sender.register_callback(evt => {
            let jmsg = JSON.parse(evt.data);
            if (jmsg.type == 'mediasoup-notification') {
                show_msg('New notification came from server:' + jmsg.m);
                room.receiveNotification(jmsg.m);
            }
            else if (jmsg.type == "queryRoomResponse") {
                show_msg('queryRoomResponse: ' + jmsg.m);
                let req_resp = get_req_res(jmsg.id);
                if (req_resp != undefined) {
                    req_resp[0](jmsg.m);
                }
            }
            else if (jmsg.type == "queryRoomResponseError") {
                show_msg('query room response error: ' + jmsg.m);
                let req_resp = get_req_res(jmsg.id);
                if (req_resp != undefined) {
                    req_resp[1](jmsg.m);
                }
                //room_error_callback(jmsg.m);
            }
            else if (jmsg.type == "join_response") {
                show_msg('join response: ' + jmsg.m);
                let req_resp = get_req_res(jmsg.id);
                if (req_resp != undefined) {
                    req_resp[0](jmsg.m);
                }
            }
            else if (jmsg.type == "join_response_error") {
                show_msg('join response error: ' + jmsg.m);
                try {
                    let req_resp = get_req_res(jmsg.id);
                    if (req_resp != undefined) {
                        req_resp[1](jmsg.m);
                    }
                }
                catch (err) {
                    show_msg("join response error exception " + err);
                }

            }
            else if (jmsg.type == "response") {
                show_msg('response : ' + jmsg.m);
                let req_resp = get_req_res(jmsg.id);
                if (req_resp != undefined) {
                    req_resp[0](jmsg.m);
                }
            }
            else if (jmsg.type == "response_error") {
                show_msg('response error : ' + jmsg.m);

                try {
                    let req_resp = get_req_res(jmsg.id);
                    if (req_resp != undefined) {
                        req_resp[1](jmsg.m);
                    }
                }
                catch (err) {
                    show_msg('response  error exception : ' + err);
                }

            }
        });

        function handlePeer(peer) {
            // Handle all the Consumers in the Peer.
            peer.consumers.forEach(consumer => handleConsumer(consumer));

            // Event fired when the remote Room or Peer is closed.
            peer.on('close', () => {
                show_msg('Remote Peer closed')
            });

            // Event fired when the remote Peer sends a new media to mediasoup server.
            peer.on('newconsumer', (consumer) => {
                show_msg("got new consumer");
                // Handle the Consumer.
                handleConsumer(consumer);
            });
        }

        function handleConsumer(consumer) {
            // Receive the media over our receiving Transport.
            show_msg("handleConsumer called");
            consumer.receive(recvTransport)
                .then((track) => {
                    show_msg('Receiving a new remote MediaStreamTrack:' + consumer.kind);
                    // Attach the track to a MediaStream and play it.
                    const stream = new MediaStream();
                    stream.addTrack(track);

                    // if (consumer.kind === 'video') {
                    //     const video = document.createElement('video');
                    //     video.setAttribute('style', 'max-width: 400px;');
                    //     video.setAttribute('playsinline', '');
                    //     video.srcObject = stream;
                    //     document.getElementById('container').appendChild(video);
                    //     video.play();
                    // }
                    if (consumer.kind === 'audio') {
                        const audio = document.createElement('audio');
                        audio.srcObject = stream;
                        document.getElementById('container').appendChild(audio);
                        audio.play();
                    }
                });

            // Event fired when the Consumer is closed.
            consumer.on('close', () => {
                show_msg('Consumer closed');
            });
        }


    }
}