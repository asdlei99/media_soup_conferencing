const mediasoup = require('mediasoup');
const config = require('./config');

//global variable
let worker;

class ProducerHandler{
  constructor(){
    this.producers_ = [];
  }

  save_producer(producer){
    this.producers_.push(producer);
  }

  get_producers_except(producer){
    return  producer == null? this.producers_:this.producers_.filter(elem=>elem.id != producer.id);
  }

  remove_producer(producer){
    const new_arr = this.get_producers_except(producer);
    this.producers_ = new_arr;
  }
  
};

let producer_handler = new ProducerHandler();


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
  let mediasoupRouter = room_handler.get_room_handle(roomId);
  if (!mediasoupRouter.canConsume(
    {
      producerId: producer.id,
      rtpCapabilities,
    })
  ) {
    console.error('can not consume');
    return;
  }
  try {
    console.log("going to create consumer");
    let consumerTransport = peer.get_consumer_transport();
    let consumer = await consumerTransport.consume({
      producerId: producer.id,
      rtpCapabilities,
      paused: producer.kind === 'video',
    });

    peer.save_consumer(consumer);

  } catch (error) {
    console.error('consume failed', error);
    return;
  }

  // if (consumer.type === 'simulcast') {
  //   await consumer.setPreferredLayers({ spatialLayer: 2, temporalLayer: 2 });
  // }
  let consumer = peer.get_consumer();
  return {
    producerId: producer.id,
    id: consumer.id,
    kind: consumer.kind,
    rtpParameters: consumer.rtpParameters,
    type: consumer.type,
    producerPaused: consumer.producerPaused
  };
}


class _room_handler{
  /*
  each element will be object in format
  {
    id:routreid
    room:router_object,
    name:room_name
  }
  */
    constructor(){
     this.rooms = new Array();//{id:,room:,name: }
    }
 
    is_room_exists(roomId){
      const elm =  this.rooms.find(elem=>elem.id==roomId);
      return elm !== undefined;
    }
 
    get_room_handle(roomId){
      const elm =  this.rooms.find(elem=>elem.id==roomId);
      return elm !== undefined?elm.room:null; 
    }

    save_room(roomId, room, name){
      this.rooms.push({id:roomId, 'room':room, 'name':name});
    }

    print_rooms(){
      console.log("following rooms id available");
      this.rooms.forEach(elem=>{
        console.log(elem.id);
      });
    }

    delete_room(roomId){
      const new_rooms = this.rooms.filter(elem=>elem.id != roomId);
      const status = new_rooms.length < this.rooms;
      this.rooms = new_rooms;
      return status;
    }
    
    //return {id:id, name:}
    get_rooms_info(){
      const arr = this.rooms.map(elem =>{return {'id':elem.id, 'name':elem.name}});
      return arr;
    }
 
  };

  class _peer{
      constructor(key, name, roomId, con){
          this.id_ = key;
          this.name_ = name;
          this.roomId_ = roomId;
          this.con_ = con;
          this.producerTransport_ = null;
          this.producer_ = null;
          this.consumerTransport_ = null;//todo: FIX ME there could be more than one consumer transport 
          this.consumer_ = null; //todo: FIX ME there could be more than one consumer.
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

      save_consumer(consumer){ this.consumer_ = consumer;}

      get_consumer(){return this.consumer_;}

      get_name(){return this.name_;}
      get_roomId(){return this.roomId_;}
      get_conection(){return this.con_;}
      close(){
        this.id_ = null;
        this.peer_= null;
        this.name_ = null;
        this.roomId_ = null;
        producer_handler.remove_producer(this.producer_);
        //todo: need to do for producerTransport
      }
  }
 
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
  room_handler.save_room(id, mediasoupRouter, roomName);
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

    let mediaPeer = peer.get_peer();
    if(mediaPeer != null)
      mediaPeer.close();
    mediaPeer = null;
    peer.close();
    peer = null;
  }


  module.exports.save_peer = (key, peerName, roomId, con)=>{
    let peer = new _peer(key, peerName, roomId, con);
    return peer;
  }

  module.exports.handle_room_join_request = handle_room_join_request;

 //module.exports.handle_media_soup_request = handle_media_soup_request;

 module.exports.handle_close = handle_close;

 module.exports.create_room = create_room;

 module.exports.delete_room = delete_room;

 module.exports.get_rooms_info = ()=>{ return room_handler.get_rooms_info();};

 module.exports.get_router_capablity = (roomId)=>{
   let mediasoupRouter = room_handler.get_room_handle(roomId);
   return mediasoupRouter.rtpCapabilities;
 }

 module.exports.createproducer = async ({forceTcp, rtpCapabilities}, peer)=>{
  const roomId = peer.get_roomId();
  let router = room_handler.get_room_handle(roomId);
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
  producer_handler.save_producer(producer);
  return { id: producer.id };
 };

 module.exports.createConsumerTransport = async (data, peer)=>{
   console.log('going to create consumer tranport');
   const roomId = peer.get_roomId();
   let router = room_handler.get_room_handle(roomId);
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

 module.exports.createConsumer  = async (data, peer)=>{
   const roomId = peer.get_roomId();
   const producer = peer.get_producer();
   const producers = producer_handler.get_producers_except(producer);
  const ret = await createConsumer(producers[0], data.rtpCapabilities, peer);
  return ret;
 }

 module.exports.resume = async (peer)=>{
   let consumer = peer.get_consumer();
   consumer.resume();
 }