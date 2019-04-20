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


  module.exports.create_room = codecs=>{
    return mediaServer.Room(codecs);
  };

  module.exports.handleMediaPeer = (mediaPeer,con) => {
    mediaPeer.on('notify', (notification) => {
      console.log('New notification for mediaPeer received:', notification);
      con.send(JSON.stringify({type:'mediasoup-notification', m:notification}));
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
  }