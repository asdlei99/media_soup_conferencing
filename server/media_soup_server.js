const mediasoup = require('mediasoup');
const config = require('./config');

//global variable
let worker;

async function runMediasoupWorker() {

  worker = await mediasoup.createWorker({
    logLevel: config.mediasoup.worker.logLevel,
    logTags: config.mediasoup.worker.logTags,
    rtcMinPort: config.mediasoup.worker.rtcMinPort,
    rtcMaxPort: config.mediasoup.worker.rtcMaxPort,
  });

  worker.on('died', () => {
    console.error('mediasoup Worker died, exiting in 2 seconds... [pid:%d]', worker.pid);
    setTimeout(() => process.exit(1), 2000);
  });
  const mediaCodecs = config.mediasoup.router.mediaCodecs;
  let mediasoupRouter = await worker.createRouter({ mediaCodecs });
  return mediasoupRouter;
}

async function createWebRtcTransport(mediasoupRouter) {
  const {
    maxIncomingBitrate,
    initialAvailableOutgoingBitrate
  } = config.mediasoup.webRtcTransport;

  const transport = await mediasoupRouter.createWebRtcTransport({
    listenIps: config.mediasoup.webRtcTransport.listenIps,
    enableUdp: true,
    enableTcp: true,
    preferUdp: true,
    initialAvailableOutgoingBitrate,
  });
  if (maxIncomingBitrate) {
    try {
      await transport.setMaxIncomingBitrate(maxIncomingBitrate);
    } catch (error) {
    }
  }
  return {
    transport,
    params: {
      id: transport.id,
      iceParameters: transport.iceParameters,
      iceCandidates: transport.iceCandidates,
      dtlsParameters: transport.dtlsParameters
    },
  };
}

async function createConsumer(producer, rtpCapabilities, peer) {
  const roomId = peer.get_roomId();
  let room = room_handler.get_room_handle(roomId);
  let mediasoupRouter = room.get_router();
  if (!mediasoupRouter.canConsume(
    {
      producerId: producer.id,
      rtpCapabilities,
    })
  ) {
    console.error('can not consume');
    return;
  }
  let consumer = null;
  try {
    console.log("going to create consumer");
    let consumerTransport = peer.get_consumer_transport();
   consumer = await consumerTransport.consume({
      producerId: producer.id,
      rtpCapabilities,
      paused: producer.kind === 'video',
    });
    peer.save_consumer(consumer.id, consumer);

  } catch (error) {
    console.error('consume failed', error);
    return;
  }

  // if (consumer.type === 'simulcast') {
  //   await consumer.setPreferredLayers({ spatialLayer: 2, temporalLayer: 2 });
  // }

  return {
    producerId: producer.id,
    id: consumer.id,
    kind: consumer.kind,
    rtpParameters: consumer.rtpParameters,
    type: consumer.type,
    producerPaused: consumer.producerPaused
  };
}


class _peer{
  constructor(key, name, roomId, con){
      this.id_ = key;
      this.name_ = name;
      this.roomId_ = roomId;
      this.con_ = con;
      this.producerTransport_ = null;
      this.producer_ = null;
      this.consumerTransport_ = null;
      this.consumers_ = new Array();
  }

  save_producer_transport(transport){
    this.producerTransport_ = transport;
  }

  get_producer_transport(){ return this.producerTransport_;}

  save_producer(producer){
    this.producer_ = producer
  }

  get_producer(){return this.producer_;}

  save_consumer_transport(transport){
    this.consumerTransport_ = transport;
  }

  get_consumer_transport(){return this.consumerTransport_;}

  save_consumer(id, consumer){
     this.consumers_.push({id,consumer });
  }

  get_consumer(id){
      const elem = this.consumers_.find((value, index, arra)=>{ return value.id == id});
      if(elem == undefined) return null;
      return elem.consumer;
  }

  get_name(){return this.name_;}
  get_roomId(){return this.roomId_;}
  get_conection(){return this.con_;}

  get_id(){return this.id_;}

  close(){
    this.id_ = null;
    this.peer_= null;
    this.name_ = null;
    this.roomId_ = null;
    //todo: need to do for producerTransport
  }
}

class Room{
  /*
  id - routerid;
  room - router
  name- room_name
  peers --> peer[]
  */
  constructor(id, router, name){
    this.id_ = id;
    this.room_ = router;
    this.name_ = name;
    this.peers = [];
  }

  save_peer(peer){
    this.peers.push(peer);
  }

  remove_peer(peer){
    const id = peer.get_id();
    const new_peers = this.peers.filter((peer_)=>{peer_.get_id() != id});
    const status = new_peers.length < this.peers.length;
    this.peers = new_peers;
    return status;//whether remove success or not.
  }

  get_peer(id){
    const peer =  this.peers.find((value, index, arra)=>{
      return value.get_id() == id; 
    });
    if(peer == undefined) return null;
    return peer;
  }

  get_peers(){
    return this.peers;
  }

  get_id(){
    return this.id_;
  }

  get_router(){
    return this.room_;
  }

  get_name(){return this.name_;}
  

}

class _room_handler{
 
    constructor(){
     this.rooms = []
    }
 
    is_room_exists(roomId){
      const elm =  this.rooms.find(elem=>elem.get_id()==roomId);
      return elm !== undefined;
    }
 
    get_room_handle(roomId){
      const elm =  this.rooms.find(elem=>elem.get_id()==roomId);
      return elm !== undefined?elm:null; 
    }

    save_room( room){
      this.rooms.push(room);
    }

    print_rooms(){
      console.log("following rooms id available");
      this.rooms.forEach(elem=>{
        console.log(elem.get_id());
      });
    }

    delete_room(roomId){
      const new_rooms = this.rooms.filter(elem=>elem.get_id() != roomId);
      const status = new_rooms.length < this.rooms;
      this.rooms = new_rooms;
      return status;
    }
    
    //return {id:id, name:}
    get_rooms_info(){
      const arr = this.rooms.map(elem =>{return {'id':elem.get_id(), 'name':elem.get_name()}});
      return arr;
    }
 
  };

 
 let room_handler = new _room_handler();

 async function create_room(roomName){
  const roomInfos = room_handler.get_rooms_info();//{id:, name:}
  const rooms = roomInfos.filter(info=>info.name == roomName);
  console.log("total room with name ", roomName, rooms.length);
  if(rooms.length > 0)
  {
    console.log("found existing room for name ", roomName, rooms[0]);

    return rooms[0].id;
  } 

  //create new one.
  let mediasoupRouter = await runMediasoupWorker();
  const id = mediasoupRouter.id;
  const room = new Room(id, mediasoupRouter, roomName);
  room_handler.save_room(room);
  mediasoupRouter.observer.on('close', ()=>{
    room_handler.delete_room(id);
  });
  return id;
 }

 function delete_room(roomId){
  return room_handler.delete_room(roomId);
 }

 function handle_room_join_request(peer){
   let roomId = peer.get_roomId();

   let signaller = peer.get_conection();
    if (room_handler.is_room_exists(roomId)) {
      
      signaller.send(JSON.stringify({'type':"room_join_response", 
                            status:'okay'}));
    } else {
      console.error("room not found with id ", roomId);
      room_handler.print_rooms();
      signaller.send(JSON.stringify({'type':"room_join_response", 
              status:'error', m:"room not exists"}));
    }
}

function handle_close(peer){
    console.log('request_close received...');
  
    const id = peer.get_roomId();
    let room =  room_handler.get_room_handle(id);
    if(room == null){
      console.error("room id is not found ", id);
      return;
    }
    const r = room.remove_peer(peer);
    peer.close();
   
  }


  module.exports.save_peer = (key, peerName, roomId, con)=>{
    let peer = new _peer(key, peerName, roomId, con);
    let room =  room_handler.get_room_handle(roomId);
    if(room == null){console.error("room not found id ", roomId)}
    else{
      room.save_peer(peer);
    }
    return peer;
  }

  module.exports.handle_room_join_request = handle_room_join_request;

 //module.exports.handle_media_soup_request = handle_media_soup_request;

 module.exports.handle_close = handle_close;

 module.exports.create_room = create_room;

 module.exports.delete_room = delete_room;

 module.exports.get_rooms_info = ()=>{ return room_handler.get_rooms_info();};

 module.exports.get_router_capablity = (roomId)=>{
   let room =  room_handler.get_room_handle(roomId);
   if(room == null){console.error("room not found ", roomId); return null;}
   const mediasoupRouter = room.get_router();
   return mediasoupRouter.rtpCapabilities;
 }

 module.exports.createproducer = async ({forceTcp, rtpCapabilities}, peer)=>{
  const roomId = peer.get_roomId();
  let room = room_handler.get_room_handle(roomId);
  if(room == null){console.error("room not found ", roomId); return null;}
  let router = room.get_router();
  const { transport, params } = await createWebRtcTransport(router);
  //producerTransport = transport;
  peer.save_producer_transport(transport);
  return params;
 };

 module.exports.connectProducerTransport = async (data, peer)=>{
   let producerTransport = peer.get_producer_transport();
  await producerTransport.connect({ dtlsParameters: data.dtlsParameters });
 }
 


 module.exports.produce = async (data, peer)=>{
  const {kind, rtpParameters} = data;
  let producerTransport = peer.get_producer_transport();
  let producer = await producerTransport.produce({ kind, rtpParameters });
  peer.save_producer(producer);
  return { id: producer.id };
 };

 module.exports.createConsumerTransport = async (data, peer)=>{
   console.log('going to create consumer tranport');
   const roomId = peer.get_roomId();
   let room = room_handler.get_room_handle(roomId);
   if(room == null){console.error("room not found ", roomId); return null;}
    let router = room.get_router();
  const { transport, params } = await createWebRtcTransport(router);
  console.log("got consumer transport param ", params);
     // consumerTransport = transport;
     peer.save_consumer_transport(transport);
      return params;
 }

 module.exports.connectConsumerTransport = async (data, peer)=>{
  let consumerTransport = peer.get_consumer_transport();
  await consumerTransport.connect({ dtlsParameters: data.dtlsParameters });
  return;
 };

 module.exports.createConsumer  = async (data, peer, peerId)=>{
   
   const roomId = peer.get_roomId();
   let room = room_handler.get_room_handle(roomId);
   let producer_peer = room.get_peer(peerId);
   if(producer_peer == null){console.error("peer not found ", peerId); return false;}

   const producer = producer_peer.get_producer();
  const ret = await createConsumer(producer, data.rtpCapabilities, peer);
  return ret;
 }

 module.exports.resume = async (peer, consumerId)=>{
   let consumer = peer.get_consumer(consumerId);
   if(consumer == null){console.error("consumer not found ");}
   else consumer.resume();
 }

 module.exports.getPeersConnection = async (roomId)=>{
        const room = room_handler.get_room_handle(roomId);
        const peers = room.get_peers();
        return peers.map(peer_=>{return {'id':peer_.get_id(), 'con':peer_.get_conection()}});
 }