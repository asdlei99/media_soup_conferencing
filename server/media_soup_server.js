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
/*
    dtlsCertificateFile:config.mediasoup.dtlsCertFile,
    dtlsPrivateKeyFile:config.mediasoup.dtlsKeyFile
    */
  });

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
      close(){
        this.id_ = null;
        this.peer_= null;
        this.name_ = null;
        this.roomId_ = null;
      }
  }
 
 let room_handler = new _room_handler();

 function handle_room_join_request(peer){
   let roomId = peer.get_roomId();


    if (room_handler.is_room_exists(roomId)) {
      console.log("existing room found for roomId " + roomId);
      //room_handler.get_room_handle(roomId);
    } else {
      console.log("creating new room for "+ roomId)
      //room = mediaServer.Room(config.mediasoup.mediaCodecs);
      let room = mediaServer.Room(config.mediasoup.mediaCodecs);
      room_handler.save_room(roomId, room);
      room.on('close', () => {
        room_handler.delete_room(roomId);
      });
      
    }
   
    let signaller = peer.get_conection();
    signaller.send(JSON.stringify({'type':"room_join_response", 
                            status:'okay'}));
}

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
 
  function handle_media_soup_request(peer, request, what, id){
    let roomId =  peer.get_roomId();
    let signaller = peer.get_conection();
   let room = room_handler.get_room_handle(roomId);
   let peerName = peer.get_name();
   let target = request.target;
    console.log(request, what);
    if(target == "room"){
      room.receiveRequest(request)
            .then((response) => {
              if(what =='join'){
                let mediaPeer = process_room_join(roomId, peerName,signaller);
                peer.save_peer(mediaPeer);
              }
                signaller.send(JSON.stringify(
                    {'type':"response",
                      'id':id,
                      'm':response
                    }));
            })
            .catch((error) =>{
                console.log("handle this error", error);
                signaller.send(JSON.stringify(
                    {'type':"response_error",
                      'id':id,
                      'm':error.toString()
                    }));
            });
          
    }
    else if(target == "peer"){
      let mediaPeer = peer.get_peer();
      if(mediaPeer){
        mediaPeer.receiveRequest(request)
          .then((response) =>{ 
            signaller.send(JSON.stringify({
                'type':'response',
                'id':id,
                'm':response
            }));
          })
          .catch((error) => {
              console.log("error", error);
              signaller.send(JSON.stringify({
                'type':"response_error",
                'id':id,
                m:error.toString()
            }));
          });
      }
      else{
        console.error("why comming here = ", request);
      }
    }
    else{
      console.error("it should not come here", target);
    }
  }

 

  function handle_close(peer){
    console.log('request_close received...');

    let mediaPeer = peer.get_peer();
    if(mediaPeer != null)
      mediaPeer.close();
    mediaPeer = null;
    peer.close();
    peer = null;
  }


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

  module.exports.handle_room_join_request = handle_room_join_request;

 module.exports.handle_media_soup_request = handle_media_soup_request;

 module.exports.handle_close = handle_close;
