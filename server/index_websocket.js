const config = require('./config');
const wsServer = require('./websocket_server'); 
const media_soup_server = require('./media_soup_server');

 class _room_handler{
   constructor(){
    this.rooms = new Map();
   }

   is_room_exists(roomId){
     return this.rooms.has(roomId);
   }

   get_room_handle(roomId){
     return this.rooms.get(roomId);
   }

   save_room(roomId, room){
     this.rooms.set(roomId, room);
   }
  
   delete_room(roomId){
     this.rooms.delete(roomId);
   }

 };

let room_handler = new _room_handler();

function handle_room_join_request(peerName, roomId){
  
      if (room_handler.is_room_exists(roomId)) {
        console.log("existing room found for request from "+ peerName);
        return room_handler.get_room_handle(roomId);
      } else {
        console.log("creating new room for "+ peerName)
        //room = mediaServer.Room(config.mediasoup.mediaCodecs);
        let room = media_soup_server.create_room(config.mediasoup.mediaCodecs);
        room_handler.save_room(roomId, room);
        room.on('close', () => {
          room_handler.delete_room(roomId);
        });
        return room;
      }
}


function handle_conference(request){
    let ext_connection = request.accept(null, request.origin);
    let roomId = request.resourceURL.query.roomId;
    let peerName = request.resourceURL.query.peerName;
    let room = null;
    let mediaPeer = null;
    ext_connection.on('message', function (data) {
        let response = JSON.parse(data.utf8Data);
        if(response.type == "request_room_join"){
          room = handle_room_join_request(peerName, roomId);

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
          
                      media_soup_server.handleMediaPeer(mediaPeer, ext_connection);
          
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


wsServer.start(config.server.port, handle_conference);
