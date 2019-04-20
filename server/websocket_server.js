const WebSocketServer = require('websocket').server;
const http = require('http');


function start_server(port, connection_handler){
    let httpNodeSerer = http.createServer();

    let wsServer = new WebSocketServer(
        {
            httpServer:httpNodeSerer
        }
    );
    
    //const port = 8888;
    httpNodeSerer.listen(port);
    console.log("listening on port" + port);
    wsServer.on('request', function (request) {
        let source = request.resourceURL.query.source;
        console.log(request.resourceURL.query);
        console.log("on connect " + source);
    
        let room_id = request.resourceURL.query.roomId;
        let peer_name = request.resourceURL.query.peerName;
        console.log(room_id, peer_name);
    
        if (source == "audio_conference") {
            connection_handler(request);
        }
        else {
            console.log("unspported source received" + source);
            connection_handler(request);
            //assert(false, "handle unsupported source");
        }
    });
}

module.exports.start = start_server;



