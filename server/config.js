module.exports = {
  server: {
    port: 8888
  },
  mediasoup: {
    // mediasoup Server settings.
    rtcIPv4: true,
    rtcIPv6: false,
   // rtcAnnouncedIPv4: "52.14.119.40", todo: enable this for aws machine
    rtcAnnouncedIPv4: null,
    rtcAnnouncedIPv6: null,
    dtlsCertFile:null,
    dtlsKeyFile:null,
    // Worker settings
    worker: {
      rtcMinPort: 10000,
      rtcMaxPort: 10100,
      logLevel: 'warn',
      logTags: [
        'info',
        'ice',
        'dtls',
        'rtp',
        'srtp',
        'rtcp',
        // 'rtx',
        // 'bwe',
        // 'score',
        // 'simulcast',
        // 'svc'
      ],
    },
     // Router settings
     router: {
      mediaCodecs:
        [
          {
            kind: 'audio',
            mimeType: 'audio/opus',
            clockRate: 48000,
            channels: 2
          },
          {
            kind: 'video',
            mimeType: 'video/VP8',
            clockRate: 90000,
            parameters:
              {
                'x-google-start-bitrate': 1000
              }
          },
        ]
    },
      // WebRtcTransport settings
       // rtcAnnouncedIPv4: "52.14.119.40", todo: enable this for aws machine
      webRtcTransport: {
        listenIps: [
         // {ip:'localhost', announcedIp:'52.14.119.40'}todo: enable this for aws machine
          {ip: '192.168.0.101', announcedIp: null}
        ],
        maxIncomingBitrate: 1500000,
        initialAvailableOutgoingBitrate: 1000000,
      },
    // mediasoup per Peer max sending bitrate (in bps).
    maxBitrate: 500000
  }
};
