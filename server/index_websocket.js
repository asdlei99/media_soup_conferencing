var WebSocketServer = require('websocket').server;
var http = require('http');

const mediasoup = require('mediasoup');
const config = require('./config');

var httpNodeSerer = http.createServer();

var wsServer = new WebSocketServer(
    {
        httpServer:httpNodeSerer
    }
);

const port = 8888;
httpNodeSerer.listen(port);
// Map of Room instances indexed by roomId.
const rooms = new Map();
console.log("listening on port" + port);


wsServer.on('request', function (request) {
    let source = request.resourceURL.query.source;
    console.log(request.resourceURL.query);
    console.log("on connect " + source);

    let room_id = request.resourceURL.query.roomId;
    let peer_name = request.resourceURL.query.peerName;
    console.log(room_id, peer_name);

    if (source == "audio_conference") {
        handle_audio_conference(request);
    }
    else {
        console.log("unspported source received" + source);
        handle_audio_conference(request);
        //assert(false, "handle unsupported source");
    }
});

function handle_audio_conference(request){
    let ext_connection = request.accept(null, request.origin);
    let roomId = request.resourceURL.query.roomId;
    let peerName = request.resourceURL.query.peerName;
    let room = null;
    let mediaPeer = null;
    ext_connection.on('message', function (data) {
        // let room = null;
        // // Used for mediaSoup peer
        // let mediaPeer = null;
        // const { roomId, peerName } = socket.handshake.query;//todo handle this
        let response = JSON.parse(data.utf8Data);
        if(response.type == "request_room_join"){
           console.log("request room join received");
            if (rooms.has(roomId)) {
              console.log("existing room found for request from "+ peerName);
              room = rooms.get(roomId);
            } else {
              console.log("creating new room for "+ peerName)
              room = mediaServer.Room(config.mediasoup.mediaCodecs);
              rooms.set(roomId, room);
              room.on('close', () => {
                rooms.delete(roomId);
              });
            }
        }
        else if(response.type == 'mediasoup-request'){
           
            let request = response.request;//todo: handle this
            let what = request.method; //todo: handle this
            let id = response.id;
            console.log(request, what);
            switch (what) {

                case 'queryRoom':
                  room.receiveRequest(request)
                    .then((response) => {
                        ext_connection.send(JSON.stringify(
                        {'type':"queryRoomResponse",
                        'id':id,
                        'm':response
                        }));
                    })
                    .catch((error) =>{
                        console.log("handle this error", error);
                        ext_connection.send(JSON.stringify(
                          {'type':"queryRoomResponseError",
                          'id':id,
                          'm':error.toString()
                          }));
                     //cb(error.toString())
                    });
                  break;
          
                case 'join':
                  room.receiveRequest(request)
                    .then((response) => {
                      // Get the newly created mediasoup Peer
                      mediaPeer = room.getPeerByName(peerName);
          
                      handleMediaPeer(mediaPeer, ext_connection);
          
                      // Send response back
                      ext_connection.send(JSON.stringify({
                          'type':"join_response",
                          'id':id,
                          m:response
                      }));
                    })
                    .catch((error) => {
                        console.log("error occur ", error);//handle this case
                        ext_connection.send(JSON.stringify({
                          'type':"join_response_error",
                          'id':id,
                          m:error.toString()
                      }));
                        //cb(error.toString())
                    });
                  break;
          
                default:
                  if (mediaPeer) {
                    mediaPeer.receiveRequest(request)
                      .then((response) =>{ 
                        ext_connection.send(JSON.stringify({
                            'type':'response',
                            'id':id,
                            'm':response
                        }));
                          //cb(null, response)
                      })
                      .catch((error) => {
                          console.log("error", error);
                          ext_connection.send(JSON.stringify({
                            'type':"response_error",
                            'id':id,
                            m:error.toString()
                        }));
                       //cb(error.toString())
                      });
                  }
              }
        }
        else if (response.type == "request_sign_in") {
           // saveClientInfo(request.key, response.user, ext_connection);
        
            console.log("message received = " + response);
           // console.log("sign in request from extension for viewer id = " + respnose.gatewayUniqueId);
            //respnose.uniqueBoxId;
            ext_connection.send(JSON.stringify({
                "type": "response_sign_in",
                "m": 'joinedsucess',
                "client": "server",
            })); //todo: check viewer before sending response sucess
        }
        else if(response.type == 'request_close'){
          //mediaPeer = room.getPeerByName(peerName);
          console.log('request_close received...');
          if(mediaPeer != null)
            mediaPeer.close();
          mediaPeer = null;
        }
        else if(response.type == 'mediasoup-notification'){
          console.debug('Got notification from client peer', response.m);
          if (!mediaPeer) {
            console.error('Cannot handle mediaSoup notification, no mediaSoup Peer');
            return;
          }
          
          mediaPeer.receiveNotification(response.m);
        }
    }
    );
}



// MediaSoup server
const mediaServer = mediasoup.Server({
    numWorkers: null, // Use as many CPUs as available.
    logLevel: config.mediasoup.logLevel,
    logTags: config.mediasoup.logTags,
    rtcIPv4: config.mediasoup.rtcIPv4,
    rtcIPv6: config.mediasoup.rtcIPv6,
    rtcAnnouncedIPv4: config.mediasoup.rtcAnnouncedIPv4,
    rtcAnnouncedIPv6: config.mediasoup.rtcAnnouncedIPv6,
    rtcMinPort: config.mediasoup.rtcMinPort,
    rtcMaxPort: config.mediasoup.rtcMaxPort
  });


  const handleMediaPeer = (mediaPeer,con) => {
    mediaPeer.on('notify', (notification) => {
      console.log('New notification for mediaPeer received:', notification);
      con.send(JSON.stringify({type:'mediasoup-notification', m:notification}));
    });

    mediaPeer.on('newtransport', (transport) => {
      console.log('New mediaPeer transport:', transport.direction);
      transport.on('close', (originator) => {
        console.log('Transport closed from originator:', originator);
      });
    });

    mediaPeer.on('newproducer', (producer) => {
      console.log('New mediaPeer producer:', producer.kind);
      producer.on('close', (originator) => {
        console.log('Producer closed from originator:', originator);
      });
    });

    mediaPeer.on('newconsumer', (consumer) => {
      console.log('New mediaPeer consumer:', consumer.kind);
      consumer.on('close', (originator) => {
        console.log('Consumer closed from originator', originator);
      });
    });

    // Also handle already existing Consumers.
    mediaPeer.consumers.forEach((consumer) => {
      console.log('mediaPeer existing consumer:', consumer.kind);
      consumer.on('close', (originator) => {
        console.log('Existing consumer closed from originator', originator);
      });
    });
  }