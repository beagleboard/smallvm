var verbose = true,
    port = 9999,
    httpServer,
    server,
    socket,
    connectionId,
    listenerId;

// Utility functions

function log (string) {
    var element = document.querySelector('#log');
    if (!verbose) { return; }
    element.append(string + '\n');
    element.scrollTop = element.scrollHeight;
    console.log(string);
};

// WebSocket server
if (http.Server && http.WebSocketServer) {
    httpServer = new http.Server();
    wsServer = new http.WebSocketServer(httpServer);
    httpServer.listen(port);

    wsServer.addEventListener('request', function (request) {
        log('websocket client connected');
        socket = request.accept();
        socket.binaryType = 'arraybuffer';

        // redirect anything we get on the socket to the serial port
        socket.addEventListener('message', function (event) {
            // event.data should contain an array of integers
            serialSend(event.data);
        });

        socket.addEventListener('close', function() {
            log('websocket client disconnected');
            socket = null;
        });
        return true;
    });
}

// Connection handling

function serialConnect (portName, callback) {
    log('connecting to ' + portName);
    chrome.serial.connect(
        portName, 
        { bitrate: 9600 },
        function (connectionInfo) {
            connectionId = connectionInfo.connectionId;
            listenerId = chrome.serial.onReceive.addListener(onSerialReceive);
            log('connected');
            callback();
        }
    );
};

function serialDisconnect (retries, callback, onErrorCallback) {
    var myself = this;
    
    if (retries === 0) {
        onErrorCallback();
        throw new Error('Could not disconnect from board');
    }

    chrome.serial.onReceive.removeListener(listenerId);
    chrome.serial.disconnect(
        connectionId,
        function (success) {
            if (success) {
                log('disconnected');
                callback();
                chrome.runtime.reload();
            } else {
                log('could not disconnect, retrying... (attempt #' + (3 - retries) + ')');
                myself.disconnect(retries ? retries - 1 : 2, callback);
            }
        }
    );
};

// Serial messaging

function onSerialReceive (info) {
    //var arrayBuffer = (new Uint8Array(info.data)).buffer;
    if (socket) {
        log('serial to ws: ' + info.data);
        socket.send(info.data);
    } else {
        log('Socket is not connected');
    }
};

function serialSend (arrayBuffer) {
    if (connectionId) {
        chrome.serial.send(
            connectionId,
            arrayBuffer,
            function (result) { 
                log('ws to serial: ' + result); 
            }
        );
    } else {
        log('Board is not connected');
    }
};
