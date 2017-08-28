var stream = require('stream');


function WebSocks(wsurl)
{
    this._wsurl = wsurl;
    this._ws = null;
    this._ouraddr = null;
    this._remoteAddr = null;
    this._recvbuffer = stream.PassThrough();
    this._sendbuffer = stream.PassThrough();
}

WebSocks.prototype.createListener = function(callback) {
    var self = this;
    self._doAccept(function(err) {
        if(err) callback(err, null);
        else {
            callback(null, self._sendbuffer, self._recvbuffer);
        }
    }, function(data) {
        self._recvbuffer.write(data);
    });
};

WebSocks.prototype._doAccept = function(accept_cb, recv_cb) {
    var self = this;
    self._socket(function (err, data) {
        if(!self._remoteAddr)
        {
            if(!data)
            {
                // send handshake
                self._ws.send("accept");
            }
            else
            {
                // this means handshake read
                var j = JSON.parse(data);
                if(j.error) {
                    connect_cb(j.error);
                } else if(j.success){
                    self._remoteAddr = host;
                    connect_cb(null);
                }
                // repeat
                setTimeout(function() {
                    self._doAccept(connect_cb);
                }, 10);
            }
        }
        else
        {
            recv_cb(err, data);
        }
    });
};

WebSocks.prototype._closed = function()
{
    var self = this;
    self._recvbuffer.close();
    self._sendbuffer.close();
    self._ws.close();
};

WebSocks.prototype.connect = function(host, port, connect_cb) {
    var self = this;
    self._socket(function (err, data) {
        if(!self._remoteAddr)
        {
            if(!data)
            {
                // send handshake
                self._ws.send("connect "+host+":"+port);
            }
            else
            {
                // this means handshake read
                var j = JSON.parse(data);
                if(j.error) {
                    connect_cb(j.error, null, null);
                } else if(j.success){
                    self._remoteAddr = host;
                    connect_cb(null, self._sendbuffer, self._recvbuffer);
                } else {
                    connect_cb("failed to connect", null, null);
                }
            }
        }
        else
        {
            self._recvbuffer.write(data);
        }
    });
};

WebSocks.prototype._socket = function(cb) {
    var self = this;
    self._ws = new WebSocket(self._wsurl);
    self._ws.onclose = function(ev) {
        self._closed();
    };

    self._ws.onopen = function(ev) {
        console.log("connected to websocks");
        self._ws.send("getaddr");
    };

    self._ws.onmessage = function(ev) {
        if(!self._ouraddr)
        {
            var j = JSON.parse(ev.data);
            if(j.error) {
                console.log("connect error: "+j.error);
                cb(j.error, null);
                return;
            }
            if(j.success && j.destination) {
                self._ouraddr = j.destination;
            } else {
                cb("failed to connect", null);
                return;
            }
            self._sendbuffer.on("data", function(data) {
                self._ws.send(data);
            });
            cb(null, null);
        }
        else
            cb(null, ev.data);
    };
};

module.exports = {
    "WebSocks": WebSocks,
};
