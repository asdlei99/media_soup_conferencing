'use strict';
const mediasoup = require('mediasoup-client');
const show_msg = require('./util');

let device = null;
let producerTransport = null;
let producer = null;


class ConsumerHandler{


  constructor(){
    this.transport_ = null;
    this.sender_ = null;
    this.callback_ = null;
  }

  async subsribe(sender, stream_observer){
    this.sender_ = sender;

    show_msg("calling subscribe");
    sender.register_callback("subscribe", evt=>{
  
      const jmsg = JSON.parse(evt.data);
      if(jmsg.type == 'responseCreateConsumerTransport'){
        sender.unregister_callback("subscribe");
        show_msg('responseCreateConsumerTransport');
        const transport = device.createRecvTransport(jmsg.m);
        transport.on('connect', ({ dtlsParameters }, callback, errback) => {
          sender.register_callback("one", evt=>{
            const jmsg = JSON.parse(evt.data);
            if(jmsg.type == 'responseConnectConsumerTransport' ){
              callback()
              sender.unregister_callback("one");
            }
          });
          sender.send(JSON.stringify({type:'connectConsumerTransport',
                'm':{transportId:transport.id, dtlsParameters}}));
          }
        );

      transport.on('connectionstatechange', (state) => {
        switch (state) {
          case 'connecting':
          show_msg("connecting subscription");
           // $txtSubscription.innerHTML = 'subscribing...';
           // $fsSubscribe.disabled = true;
            break;
    
          case 'connected':
          show_msg("remove video receved connected recevied");
          //document.querySelector('#shareVideo').srcObject = stream;
            // $txtSubscription.innerHTML = 'subscribed';
            // $fsSubscribe.disabled = true;
            break;
    
          case 'failed':
          show_msg("failed");
            transport.close();
            // $txtSubscription.innerHTML = 'failed';
            // $fsSubscribe.disabled = false;
            break;
    
          default: break;
        }
      });
      this.transport_ = transport;
      this.listen_peer_notification(stream_observer);
    }
  });
    sender.send(JSON.stringify({'type':'createConsumerTransport', 
                m:JSON.stringify({forceTcp:false})})
                );

  }

  async listen_peer_notification(callback){
    this.sender_.register_callback("peer_notification", evt=>{
      console.log("peer notification called");
      const jmsg = JSON.parse(evt.data);
      if(jmsg.type == "peer_add" ){
        console.log("add event receive");
        const producer_id = jmsg.m;
        this.consume(producer_id, stream=>{
          callback('receive', producer_id, 'video', stream);
        } )
      }

    });
  }

consume(remote_peer_id, callback) {
    const { rtpCapabilities } = device;
  
    this.sender_.register_callback("consume", evt=>{
      const jmsg = JSON.parse(evt.data);
      console.log(jmsg.type);
      if(jmsg.type == 'responseConsumer'){
        show_msg('responsconsume received ', jmsg);
        const data = jmsg.m;
        const {
          producerId,
          id,
          kind,
          rtpParameters,
        } = data;
        let codecOptions = {};
  
        this.transport_.consume({
            id,
            producerId,
            kind,
            rtpParameters,
          codecOptions,
        }).then(consumer=>{
          const stream = new MediaStream();
          stream.addTrack(consumer.track);
          show_msg("remove stream received");
          callback(stream);
        });
       this.sender_.unregister_callback("consume");
      }
    })
  
    this.sender_.send(JSON.stringify({type:'consume', id:remote_peer_id, m:JSON.stringify({'rtpCapabilities':rtpCapabilities})}));
  }

}

let consumerHandler = new ConsumerHandler();

async function loadDevice(routerRtpCapabilities) {
    try {
        device = new mediasoup.Device();
    } catch (error) {
      if (error.name === 'UnsupportedError') {
        console.error('browser not supported');
      }
    }
   await device.load({ routerRtpCapabilities });
  }

  async function subscribe(sender){
    show_msg("calling subscribe");
    sender.register_callback("subscribe", evt=>{
      
      const jmsg = JSON.parse(evt.data);
      if(jmsg.type == 'responseCreateConsumerTransport'){
        sender.unregister_callback("subscribe");
        show_msg('responseCreateConsumerTransport');
        const transport = device.createRecvTransport(jmsg.m);
        transport.on('connect', ({ dtlsParameters }, callback, errback) => {
          sender.register_callback("one", evt=>{
            const jmsg = JSON.parse(evt.data);
            if(jmsg.type == 'responseConnectConsumerTransport' ){
              callback()
              sender.unregister_callback("one");
            }
          });
          sender.send(JSON.stringify({type:'connectConsumerTransport',
                'm':{transportId:transport.id, dtlsParameters}}));
          }
        );

      transport.on('connectionstatechange', (state) => {
        switch (state) {
          case 'connecting':
          show_msg("connecting subscription");
           // $txtSubscription.innerHTML = 'subscribing...';
           // $fsSubscribe.disabled = true;
            break;
    
          case 'connected':
          show_msg("remove video receved connected recevied");
          //document.querySelector('#shareVideo').srcObject = stream;
            // $txtSubscription.innerHTML = 'subscribed';
            // $fsSubscribe.disabled = true;
            break;
    
          case 'failed':
          show_msg("failed");
            transport.close();
            // $txtSubscription.innerHTML = 'failed';
            // $fsSubscribe.disabled = false;
            break;
    
          default: break;
        }
      });
      
     // let stream;
    consume(transport, sender, (stream_out)=>{
      stream = stream_out;
      sender.send(JSON.stringify({'type':'resume'}));
     
    });
    }
  });
    sender.send(JSON.stringify({'type':'createConsumerTransport', 
                m:JSON.stringify({forceTcp:false})})
                );
  
  }

  

 async function publish(sender){
    
  sender.register_callback("publish", evt=>{
      const jmsg = JSON.parse(evt.data);
      if(jmsg.type == 'responseCreateProducerTransport'){
          sender.unregister_callback("publish");
         const data = jmsg.m;
         producerTransport =  device.createSendTransport(data);
         producerTransport.on('connect', 
          async ({ dtlsParameters }, callback, errback) => {

              sender.send(JSON.stringify({
                  'type':'connectProducerTransport', 
                  m:JSON.stringify({'dtlsParameters':dtlsParameters })
              }));
              callback();
            });

            producerTransport.on('produce', 
          async ({ kind, rtpParameters }, callback, errback) => {
          try {
            sender.register_callback( "respnoseProduce", evt=>{
                  const jmsg = JSON.parse(evt.data);
                  if(jmsg.type == 'responseProduce'){
                    sender.unregister_callback("responseProduce");
                      const {id} = jmsg.m;
                      callback(id);
                  }
              });
              sender.send(JSON.stringify({
                  type:'produce',
                  m:{ transportId: producerTransport.id,
                      kind,
                      rtpParameters,}
              }));
          } catch (err) {
              errback(err);
          }
          });

          producerTransport.on('connectionstatechange', (state) => {
              switch (state) {
                case 'connecting':
                show_msg('conneccting transport');
                  // $txtPublish.innerHTML = 'publishing...';
                  // $fsPublish.disabled = true;
                  // $fsSubscribe.disabled = true;
                break;
          
                case 'connected':
                show_msg('connected');
                  
                  // $txtPublish.innerHTML = 'published';
                  // $fsPublish.disabled = true;
                  // $fsSubscribe.disabled = false;
                break;
          
                case 'failed':

                  show_msg('failed handle it fast');
                  producerTransport.close();
                  // $txtPublish.innerHTML = 'failed';
                  // $fsPublish.disabled = false;
                  // $fsSubscribe.disabled = true;
                break;
          
                default: break;
              }
            });
          
          // let stream;
         try {
              getUserMedia(producerTransport).then(stream=>{
                document.querySelector('#shareVideo').srcObject = stream;
               // document.querySelector('#shareVideo').play();
                show_msg('got stream from user media');
              });
         } catch (err) {
          show_msg("error ", err);
             //$txtPublish.innerHTML = 'failed';
         }
      }
    });

    sender.send(JSON.stringify({'type':'createProducerTransport',
          m:JSON.stringify({forceTcp: false,
              rtpCapabilities: device.rtpCapabilities})
            })
            );
 }

async function getUserMedia(transport) {
    if (!device.canProduce('video')) {
      show_msg('cannot produce video');
      return;
    }
  
    let stream;
    try {
        stream = await navigator.mediaDevices.getUserMedia({ video: true });
    } catch (err) {
      show_msg('starting webcam failed,', err.message);
      throw err;
    }
    const track = stream.getVideoTracks()[0];
    const params = { track };
    
    producer = await transport.produce(params);
    return stream;
  }

class media_soup_conference {

    
    constructor(observer){
        this.stream_observer_ = observer;
        this.consumerHandler_ = new ConsumerHandler();
    }

    start_consumer(peer_name, signaller){
      this.start(peer_name, signaller, subscribe )
    }
  
    start(peer_name, signaller, functionality = null) {
        this.signaller = signaller;
        if(functionality == null) functionality = publish;
        this.request_room_join(peer_name, signaller, functionality);
        
    
    }

    stop(){
        show_msg("going to send close request");
        
        this.signaller.send(
        JSON.stringify({
            'type': 'request_close'
        }));
    }

    request_room_join(peerName, sender, functionality){
        let self = this;
        sender.register_callback("room_join", evt=>{
            let jmsg = JSON.parse(evt.data);
            if(jmsg.type == 'room_join_response'){
              sender.unregister_callback("room_join");
                if(jmsg.status == 'okay'){
                    self._join_room(peerName, sender, functionality);
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


    _join_room(peer_name, sender, functionality) {
        //let stream_observer_ = this.stream_observer_;
       // let req_res = new response_caller();
       let self = this;
        sender.register_callback("router_capacity", evt=>{
            const jmsg = JSON.parse(evt.data);
            if(jmsg.type == 'response_router_capablity'){
              sender.unregister_callback("router_capacity");
               show_msg("response_router_capablity got");
                loadDevice(jmsg.m).then(
                    ()=>{
                      functionality(sender);
                      this.consumerHandler_.subsribe(sender, this.stream_observer_);
                    }
                );
               // await publish(sender);
            }
        });
        sender.send(JSON.stringify({
            'type':'get_router_capability'
        }));
      }



    }

    module.exports = media_soup_conference;