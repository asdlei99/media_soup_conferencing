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
        if(response.type == "request_room_join"){
           media_soup_server.handle_room_join_request(peer);
        }
        else if(response.type == 'mediasoup-request'){
         
           
            let request = response.request;//todo: handle this
            let what = request.method; //todo: handle this
            let id = response.id;
            console.log(request, what);
            media_soup_server.handle_media_soup_request(peer, request, what, id);
        }
        else if(response.type == 'request_close'){
          media_soup_server.handle_close(peer);
        }
        else if(response.type == 'mediasoup-notification'){
          media_soup_server.on_notification(response.m, peer);
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
