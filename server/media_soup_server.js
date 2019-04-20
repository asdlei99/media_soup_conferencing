const mediasoup = require('mediasoup');
const config = require('./config');


// // MediaSoup server
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


  function create_room(codecs){
      return mediaServer.Room(codecs);
  }

  module.exports.create_room = create_room;

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

  class _peer{
      constructor(key, name, roomId, con){
          this.id_ = key;
          this.peer_ = null;
          this.name_ = name;
          this.roomId_ = roomId;
          this.con_ = con;
      }
      save_peer(peer){
          this.peer_ = peer;
      }

      get_peer(){ return this.peer_;}

      get_name(){return this.name_;}
      get_roomId(){return this.roomId_;}
      get_conection(){return this.con_;}
  }
 
 let room_handler = new _room_handler();

 function handle_room_join_request(peerName, roomId){

    if (room_handler.is_room_exists(roomId)) {
      console.log("existing room found for request from "+ peerName);
      return room_handler.get_room_handle(roomId);
    } else {
      console.log("creating new room for "+ peerName)
      //room = mediaServer.Room(config.mediasoup.mediaCodecs);
      let room = create_room(config.mediasoup.mediaCodecs);
      room_handler.save_room(roomId, room);
      room.on('close', () => {
        room_handler.delete_room(roomId);
      });
      return room;
    }
}

 module.exports.handle_room_join_request = handle_room_join_request;

function process_room_join(roomId, peerName, signaller){
    if(room_handler.is_room_exists(roomId)){
        let room = room_handler.get_room_handle(roomId);
        let mediaPeer = room.getPeerByName(peerName); //todo: correct this

        mediaPeer.on('notify', (notification) => {
            console.log('New notification for mediaPeer received:', notification);
            signaller.send(JSON.stringify({type:'mediasoup-notification', m:notification}));
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
          return mediaPeer;
    }
    else{
        //todo: it should not return hre.
    }
 };

 module.exports.process_room_join = process_room_join;

 
 function check_peer_media(mediaPeer, request, id, signaller){
    if(mediaPeer == null) return;
    if (mediaPeer) {
        mediaPeer.receiveRequest(request)
          .then((response) =>{ 
            signaller.send(JSON.stringify({
                'type':'response',
                'id':id,
                'm':response
            }));
              //cb(null, response)
          })
          .catch((error) => {
              console.log("error", error);
              signaller.send(JSON.stringify({
                'type':"response_error",
                'id':id,
                m:error.toString()
            }));
           //cb(error.toString())
          });
      }
 }
  
  function handle_media_soup_request(peer, request, what, id){
    let roomId =  peer.get_roomId();
    let signaller = peer.get_conection();
   let room = room_handler.get_room_handle(roomId);
   let peerName = peer.get_name();
    console.log(request, what);
    switch (what) {
  
        case 'queryRoom':
          room.receiveRequest(request)
            .then((response) => {
                signaller.send(JSON.stringify(
                    {'type':"queryRoomResponse",
                      'id':id,
                      'm':response
                    }));
            })
            .catch((error) =>{
                console.log("handle this error", error);
                signaller.send(JSON.stringify(
                    {'type':"queryRoomResponseError",
                      'id':id,
                      'm':error.toString()
                    }));
            });
          break;
  
        case 'join':
          room.receiveRequest(request)
            .then((response) => {
                let mediaPeer = process_room_join(roomId, peerName,signaller);
                peer.save_peer(mediaPeer);
                signaller.send(JSON.stringify({
                    'type':"join_response",
                    'id':id,
                    m:response
                }));

            })
            .catch((error) => {
                console.log("error occur ", error);//handle this case
                signaller.send(JSON.stringify(
                    {'type':"join_response_error",
                      'id':id,
                      'm':error.toString()
                    }));
            });
          break;
  
        default:
        check_peer_media(peer.get_peer(), request, id, signaller);
      }
  }

  module.exports.handle_media_soup_request = handle_media_soup_request;

  function handle_close(peer){
    console.log('request_close received...');

    let mediaPeer = peer.get_peer();
    if(mediaPeer != null)
      mediaPeer.close();
  }

  module.exports.handle_close = handle_close;

  function handle_notification(msg, peer){
    console.debug('Got notification from client peer', msg);
    let mediaPeer = peer.get_peer();
    if (!mediaPeer) {
      console.error('Cannot handle mediaSoup notification, no mediaSoup Peer');
      return;
    }
    
    mediaPeer.receiveNotification(msg);
  }

  module.exports.on_notification = handle_notification;

  module.exports.save_peer = (key, peerName, roomId, con)=>{
    let peer = new _peer(key, peerName, roomId, con);
    return peer;
  }
