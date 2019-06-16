const mediasoup = require('mediasoup');
const config = require('./config');

//global variable
let worker;
let mediasoupRouter;
let producerTransport;
let producer;
let consumerTransport;
let consumer;

  (async () => {
    try {
      await runMediasoupWorker();
    } catch (err) {
      console.error(err);
    }
  })();

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
  mediasoupRouter = await worker.createRouter({ mediaCodecs });
}

async function createWebRtcTransport() {
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

async function createConsumer(producer, rtpCapabilities) {
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
    consumer = await consumerTransport.consume({
      producerId: producer.id,
      rtpCapabilities,
      paused: producer.kind === 'video',
    });
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


class _room_handler{
    constructor(){
     this.rooms = new Map();
    }
 
    is_room_exists(roomId){
      return this.rooms.has(Number(roomId));
    }
 
    get_room_handle(roomId){
      return this.rooms.get(Number(roomId)).room;
    }

    save_room(roomId, room, name){
      this.rooms.set(Number(roomId), { 'room':room, 'name':name});
    }

    print_rooms(){
      console.log("following rooms id available");
      for (var [key, value] of this.rooms) {
        console.log(key);
      }
    }
    delete_room(roomId){
      const r =  this.rooms.delete(Number(roomId));
      return r;
    }
    
    //return {id:id, name:}
    get_rooms_info(){
      let arr = new Array();
      for(let [key, value] of this.rooms){
          arr.push({'id':key, 'name':value.name});
      }
      return arr;
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

 function create_room(roomName){
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

 module.exports.get_router_capablity = ()=>{
   return mediasoupRouter.rtpCapabilities;
 }

 module.exports.createproducer = async ({forceTcp, rtpCapabilities})=>{
  const { transport, params } = await createWebRtcTransport();
  producerTransport = transport;
  return params;
 };

 module.exports.connectProducerTransport = async (data)=>{
  await producerTransport.connect({ dtlsParameters: data.dtlsParameters });
 }
 
 module.exports.connectConsumerTransport = async (data)=>{
  await consumerTransport.connect({ dtlsParameters: data.dtlsParameters });
  return;
 };

 module.exports.produce = async (data)=>{
  const {kind, rtpParameters} = data;
  producer = await producerTransport.produce({ kind, rtpParameters });
  return { id: producer.id };
 };

 module.exports.createConsumerTransport = async (data)=>{
   console.log('going to create consumer tranport');
  const { transport, params } = await createWebRtcTransport();
  console.log("got consumer transport param ", params);
      consumerTransport = transport;
      return params;
 }

 module.exports.createConsumer  = async (data)=>{
  const ret = await createConsumer(producer, data.rtpCapabilities);
  return ret;
 }

 module.exports.resume = async ()=>{
   consumer.resume();
 }