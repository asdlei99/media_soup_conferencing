'use strict';
const mediasoup = require('mediasoup-client');
const show_msg = require('./util');

let device = null;
let transport = null;
let producer = null;

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
    sender.register_callback(evt=>{

      const jmsg = JSON.parse(evt.data);
      if(jmsg.type == 'responseCreateConsumerTransport'){
        show_msg('responseCreateConsumerTransport');
        const transport = device.createRecvTransport(jmsg.m);
        transport.on('connect', ({ dtlsParameters }, callback, errback) => {
          sender.register_callback(evt=>{
            const jmsg = JSON.parse(evt.data);
            if(jmsg.type == 'responseConnectConsumerTransport' ){
              callback()
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
          document.querySelector('#shareVideo').srcObject = stream;
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
      
      let stream;
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

  function consume(transport, sender, callback) {
    const { rtpCapabilities } = device;

    sender.register_callback(evt=>{
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

        transport.consume({
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
       
      }
    })

    sender.send(JSON.stringify({type:'consume', m:JSON.stringify({'rtpCapabilities':rtpCapabilities})}));
  }

 async function publish(sender){
    
  sender.register_callback(evt=>{
      const jmsg = JSON.parse(evt.data);
      if(jmsg.type == 'responseCreateProducerTransport'){
         const data = jmsg.m;
          transport =  device.createSendTransport(data);
          transport.on('connect', 
          async ({ dtlsParameters }, callback, errback) => {

              sender.send(JSON.stringify({
                  'type':'connectProducerTransport', 
                  m:JSON.stringify({'dtlsParameters':dtlsParameters })
              }));
              callback();
            });

          transport.on('produce', 
          async ({ kind, rtpParameters }, callback, errback) => {
          try {
            sender.register_callback( evt=>{
                  const jmsg = JSON.parse(evt.data);
                  if(jmsg.type == 'responseProduce'){
                      const {id} = jmsg.m;
                      callback(id);
                  }
              });
              sender.send(JSON.stringify({
                  type:'produce',
                  m:{ transportId: transport.id,
                      kind,
                      rtpParameters,}
              }));
          } catch (err) {
              errback(err);
          }
          });

          transport.on('connectionstatechange', (state) => {
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
                   transport.close();
                  // $txtPublish.innerHTML = 'failed';
                  // $fsPublish.disabled = false;
                  // $fsSubscribe.disabled = true;
                break;
          
                default: break;
              }
            });
          
          // let stream;
         try {
              getUserMedia(transport).then(stream=>{
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
        sender.register_callback(evt=>{
            let jmsg = JSON.parse(evt.data);
            if(jmsg.type == 'room_join_response'){
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
        sender.register_callback(evt=>{
            const jmsg = JSON.parse(evt.data);
            if(jmsg.type == 'response_router_capablity'){
               show_msg("response_router_capablity got");
                loadDevice(jmsg.m).then(
                    ()=>{
                      functionality(sender);
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