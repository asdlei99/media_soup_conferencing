'use strict';

class response_caller{
    //used to save id, and callback handler for success and response
//for the local room request, 
//those request are sent to remote and whatever reponse is received is 
//success or error callback is called by using id.

    constructor(){
        this.res_map = new Map();
        this.id_ = 0;
    }

    _get_id(){
        this.id_++;
        return this.id_;
    }

    save_response(success, error){
        let id = this._get_id();
        if (this.res_map.has(id)) {
            show_msg("error : id is afeady saved " + id);
        }
        else {
            this.res_map.set(id, [success, error]);
        }
        return id;
    }

    process(id, msg, is_success){
        let arr = this.res_map.get(id);
        if (this.res_map.delete(id) == false) {
            show_msg("error: id not found " + id);
        }
        let callback = (is_success?arr[0]:arr[1]);
        callback(msg);
    }
}

class media_soup_conference {

    constructor(observer){
        this.stream_observer_ = observer;
    }
  
    start(peer_name, signaller) {
        this.signaller = signaller;
        this.request_room_join(peer_name, signaller);
    
    }

    stop(){
        show_msg("going to send close request");
        
        this.signaller.send(
        JSON.stringify({
            'type': 'request_close'
        }));
    }

    request_room_join(peerName, sender){
        let self = this;
        sender.register_callback(evt=>{
            let jmsg = JSON.parse(evt.data);
            if(jmsg.type == 'room_join_response'){
                if(jmsg.status == 'okay'){
                    self._join_room(peerName, sender);
                }
                else{
                    show_msg("error in room join response");
                }
            }
            else{
                show_msg("unsupported message")
            }
        });
        sender.send(JSON.stringify({
            'type': "request_room_join"
        }));
    }

    _join_room(peer_name, sender) {
        let stream_observer_ = this.stream_observer_;
        let req_res = new response_caller();
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
                    video: true
                });
            })
            .then((stream) => {
                const audioTrack = stream.getAudioTracks()[0];
                const videoTrack = stream.getVideoTracks()[0];

                // Create Producers for audio and video.
                const audioProducer = room.createProducer(audioTrack);
                const videoProducer = room.createProducer(videoTrack);

                // Send our audio.
                audioProducer.send(sendTransport);
                // Send our video.
                videoProducer.send(sendTransport);
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
            let id = req_res.save_response(callback, errback);
           // save_req_res(id, callback, errback);
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
            else if (jmsg.type == "response") {
                show_msg(jmsg.type + ' ' + jmsg.m);
                req_res.process(jmsg.id, jmsg.m, true);
            }
            else if(jmsg.type == "response_error") {
                show_msg(jmsg.type + ' ' + jmsg.m);
                req_res.process(jmsg.id, jmsg.m, false);
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
                    if (stream_observer_)
                        stream_observer_(stream, consumer.kind, 'receive');
                    else {
                        show_msg("should not come here in receive");
                    }

                });

            // Event fired when the Consumer is closed.
            consumer.on('close', () => {
                show_msg('Consumer closed');
            });
        }


    }
}
