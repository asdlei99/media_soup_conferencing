const config = require('./config');
const wsServer = require('./websocket_server'); 
const media_soup_server = require('./media_soup_server');

 
function handle_conference(request){
    let ext_connection = request.accept(null, request.origin);
    let key = request.key;
    let roomId = request.resourceURL.query.roomId;
    let peerName = request.resourceURL.query.peerName;
    let peer = media_soup_server.save_peer(key, peerName, roomId, ext_connection);
    ext_connection.on('message', function (data) {
        let response = JSON.parse(data.utf8Data);
        console.log(response.type);
        if(response.type == "request_room_join"){
           media_soup_server.handle_room_join_request(peer);
        }
        else if(response.type == 'request_close'){
          media_soup_server.handle_close(peer);
        }
        else if(response.type == "get_router_capability"){
          const capablity = media_soup_server.get_router_capablity(roomId);
          const m = JSON.stringify({
            type:'response_router_capablity',
            'm':capablity
          });
          ext_connection.send(m);
        }
        else if(response.type == 'createProducerTransport' ){
          media_soup_server.createproducer(JSON.parse(response.m), peer)
          .then(param=>{
            ext_connection.send(JSON.stringify({
              type:'responseCreateProducerTransport',
              m:param
            }));
          });
          
        }
        else if('connectProducerTransport' == response.type){
          media_soup_server.connectProducerTransport(JSON.parse(response.m), peer);
        }
        else if(response.type =='produce'){
         media_soup_server.produce(response.m, peer)
         .then(ret=>{ext_connection.send(JSON.stringify({
                          'type':'responseProduce',
                       m:ret
                }));
                return;}
          )
          .then(()=>{
            media_soup_server.getPeersConnection(peer)
            .then(peers=>{
              console.log(peers);
              if(peers == undefined) return;
              peers.forEach(element => {
                element.con.send(JSON.stringify({type:"peer_add", m:element.id}));
              });
            })
           
          });
         
        }
        else if('createConsumerTransport' == response.type){
          console.log("going to call creat consumer tranport");
          media_soup_server.createConsumerTransport(JSON.parse(response.m), peer)
          .then(data=>{
            console.log('sending responsecreateconsumertransport');
            ext_connection.send(JSON.stringify({type:'responseCreateConsumerTransport', m:data}))});
        }
        else if(response.type == 'connectConsumerTransport'){
          media_soup_server.connectConsumerTransport(response.m, peer)
          .then( ()=>{ ext_connection.send(JSON.stringify({
            type:'responseConnectConsumerTransport'
          }))});
        }
        else if(response.type == 'consume'){
          console.log("going to create consumer");
          media_soup_server.createConsumer(JSON.parse(response.m), peer, response.id)
          .then(ret=>{
            console.log("sending back consum response");
            ext_connection.send(
              JSON.stringify({ 
                type:'responseConsumer',
                m:ret
              })
            );
          });
        }
        else if(response.type == "resume"){
          media_soup_server.resume(peer);
        }
        else{
          console.error("type is not ", response);
        }
     }
    );

    ext_connection.on('close', ()=>{
      console.error(" on close ", peerName, roomId);
      media_soup_server.handle_close(peer);

    });
}

wsServer.start(config.server.port, handle_conference);

//currently room create and delete request will come on port 8081
const ROOM_MANG_PORT = 8081;

function handle_room_managing_request(request){
  let ext_connection = request.accept(null, request.origin);
  const key = request.key;

  ext_connection.on('message', (data)=>{
    const response = JSON.parse(data.utf8Data);
    if(response.type == "request_room_open"){
        const room_name = response.name;
        media_soup_server.create_room(room_name)
        .then(id=> {
          console.log("sending back room id", id);
          ext_connection.send(
          JSON.stringify({type:"room_open_response",
          status:'success', 'id':id})
        );}
        );
       
    }
    else if(response.type == "request_room_close"){
      const roomId  = response.id;
      const status = media_soup_server.delete_room(roomId) == true?"success":"fail";
      console.log('room creation request' + status);
      console.log()
      ext_connection.send(
        JSON.stringify(
          {
            type:"room_close_response",
            'status':status
          }
        )
      );
    }
    else if(response.type == "request_rooms_info"){
      const rooms = media_soup_server.get_rooms_info();
      console.log("request room info received ", rooms.length);
      let ids = new Array();
      let names = new Array();

      rooms.every(obj=>{
        ids.push(obj.id);
        names.push(obj.name);
        console.log(obj.id, obj.name);
      });
      let m = JSON.stringify({
        type:"response_room_info",
        count:rooms.length,
        'ids':ids,
        'names':names
        
      });
      ext_connection.send(m);
      //send back this info to user
      ext_connection.close();
      ext_connection = null;
    }
    
    else{
      //todo: should not come here
      console.error("should not come here, it is an error");
    }
  }
  );

}

wsServer.start(ROOM_MANG_PORT, handle_room_managing_request);