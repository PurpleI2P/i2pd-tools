var l = document.getElementById("log");
var c = document.getElementById("main");
var nodes = {
  length: 0,
};
var in_traffic = 0;
var out_traffic = 0;
var tick = 0;

function nodeConnected(ident) {
  if (nodes[ident]) {
    return;
  }
  nodes[ident] = {
    send: 0,
    recv: 0 
  };
  nodes.length ++;
}

function nodeDisconnected(ident) {
  if (nodes[ident]) {
    delete nodes[ident];
    nodes.length --;
  }
}

function nodeSend(ident, n) {
  if(!nodes[ident]) {
    nodeConnected(ident);
  }
  nodes[ident].send += parseInt(n);
}

function nodeRecv(ident, n) {
  if(!nodes[ident]) {
    nodeConnected(ident);
  }
  nodes[ident].recv += parseInt(n);
}


var tpeers = {};

var tunnels = {
  length : 0
};

function ensureTunnel(tid) {
  if(!tunnels[tid]) {
    tunnels[tid] = {
      latency : -1,
      color: "#ddd",
    };
    tunnels.length ++;
  }
}

function tunnelBuild(peers, tid, inbound) {
  logit("Tunnel "+tid+" build started");
  ensureTunnel(tid);
  tunnels[tid].peers = peers.split(":");
  tunnels[tid].inbound = inbound;
  tunnels[tid].state = "building";
}

function tunnelLatency(tid, latency) {
  // logit("tunnel "+tid+" latency "+latency+"ms");
  if (!tunnels[tid]) return;
  tunnels[tid].latency = latency;
}

function tunnelEstablished(tid) {
  if(!tunnels[tid]) return;
  logit("Tunnel " + tid + " is healthy");
  tunnels[tid].state = "healthy";
  tunnels[tid].color = "#1d1";
}

function tunnelBuildFailed(tid) {
  if(!tunnels[tid]) return;
  logit("Tunnel " + tid + " failed to build");
  delete tunnels[tid];
  tunnels.length --;
}

function tunnelTestFailed(tid) {
  if(!tunnels[tid]) return;
  logit("Tunnel " + tid + " has failed tunnel test");
  tunnels[tid].state = "stuck";
  tunnels[tid].color = "orange";
}

function tunnelFailed(tid) {
  if(!tunnels[tid]) return;
  logit("Tunnel " + tid + " has failed");
  delete tunnels[tid];
  tunnels.length--;
}

function tunnelExpiring(tid) {
  if(!tunnels[tid]) return;
  logit("Tunnel " + tid + " is expired");
  delete tunnels[tid];
  tunnels.length--;
}


function tunnelState(tid, state) {
  if (state == "3") {
    tunnelEstablished(tid);
  } else if (state == "2" ) {
    tunnelBuildFailed(tid);
  } else if (state == "4" ) {
    tunnelTestFailed(tid);
  } else if (state == "5" ) {
    tunnelFailed(tid);
  } else if (state == "6" ) {
    tunnelExpiring(tid);
  }
}

function tunnelCreated(tid) {
  logit("Tunnel "+tid+" was created");
}

function logit(msg) {
  console.log(msg);
  var t = document.createTextNode(msg);
  var e = document.createElement("div");
  e.appendChild(t);
  l.appendChild(e);
  while(l.children.length > 50)
    l.removeChild(l.children[0]);
}

function socketClosed() {
  var b = document.getElementById("connect-button");
  b.onclick = startui
  b.value = "connect";
}

function startui() {
  var el = document.getElementById("ws-url");
  var url;
  if(el)
    url = el.value;
  else
    url = "ws://127.0.0.1:7666";
  
  var ws = new WebSocket(url);
  ws.onclose = function (ev) {
    logit("failed to connect to "+url);
    nodes = { length: 0 };
    tpeers = { length: 0 };
    socketClosed();
  }
  ws.onopen = function(ev) {
    logit("connected to "+url);
  }
  ws.onmessage = function(ev) {
    var j = JSON.parse(ev.data);
    if (j) {
      console.log(j);
      if(j.type == "transport.connected") {
        nodeConnected(j.ident);
      } else if (j.type == "transport.disconnected") {
        nodeDisconnected(j.ident);
      } else if (j.type == "transport.sendmsg") {
        nodeSend(j.ident, j.number);
      } else if (j.type == "transport.recvmsg") {
        nodeRecv(j.ident, j.number);
      } else if (j.type == "tunnel.build") {
        tunnelBuild(j.value, j.tid, j.inbound);
      } else if (j.type == "tunnel.latency") {
        tunnelLatency(j.tid, j.value);
      } else if (j.type == "tunnel.state") {
        tunnelState(j.tid, j.value);
      } else if (j.type == "tunnels.created") {
        tunnelCreated(j.tid);
      } else if (j.type == "tunnels.expired") {
        tunnelExpiring(j.tid);
      } else {
        logit("message: "+j.type);
      }
    }
  };
  var b = document.getElementById("connect-button");
  b.onclick = function() {
    ws.close();
  }
  b.value = "disconnect";
}

function getPeer(h, us) {
  if (tpeers[h]) {
    return tpeers[h];
  }
  console.log("make peer "+h);
  
  var p = {
    x: Math.random(),
    y: Math.random(),
    name: h,
  };
  if (us) {
    p.x = 0.5;
    p.y = 0.5;
  } else {
    while (Math.abs(p.x - 0.5) <= 0.1 ) {
      p.x = Math.random();
    }
    while (Math.abs(p.y - 0.5) <= 0.1 ) {
      p.y = Math.random();
    }
  }
  tpeers[h] = p;
  return p;
}

function drawPeer(p) {
  draw.beginPath();
  draw.lineWidth = 1;
  draw.fillStyle = "white";
  draw.strokeStyle = "white";
  draw.arc(p.x * c.width, p.y * c.height, 20, 0, 2* Math.PI);
  draw.stroke();
  draw.fillText(p.name.substr(0,6), p.x * c.width, p.y * c.height);
}


function getTraffic(node) {
  if (!node) return 1;
  return 1.0 + ( (node.send + node.recv + 1.0) / 1.5) ;
}
var draw = c.getContext("2d");
var ticks = 0;
// draw
setInterval(function() {
  draw.canvas.width  = window.innerWidth - 100;
  draw.canvas.height = window.innerHeight - 100;
  draw.font = "10px monospace";
  
  draw.clearRect(0, 0, c.width, c.height);

  // draw tunnels
  
  var e = [];

  for (var tid in tunnels) {
    if(tid == "length") continue;
    var t = tunnels[tid];
    var us = getPeer(t.peers[0], true);
    var traff = getTraffic(nodes[t.peers[1]]);
    e.push([us, getPeer(t.peers[1]), t.color, traff]);
    for (var idx = 1 ; idx + 1 < t.peers.length; idx ++ ) {
      var cur = getPeer(t.peers[idx]);
      var next = getPeer(t.peers[idx+1]);
      if(cur && next)
        e.push([cur, next, t.color, traff]);
      else
        console.log(cur, next);
    }
  }

  for ( var h in tpeers ) {
    if( h == "length") continue;
    drawPeer(getPeer(h));
  }

  var newPeers = {};
  var counter = 0;
  for ( var ed in e ) {
    var edge = e[ed];
    draw.beginPath();
    draw.strokeStyle = edge[2];
    draw.lineWidth = edge[3];
    draw.moveTo(edge[0].x * c.width, edge[0].y * c.height);
    draw.lineTo(edge[1].x * c.width, edge[1].y * c.height);
    if(!newPeers[edge[0].name]) {
      newPeers[edge[0].name] = edge[0];
      counter ++;
    }
    if(!newPeers[edge[1].name]) {
      newPeers[edge[1].name] = edge[1];
      counter ++;
    }
    draw.stroke();  
  }
  newPeers.length = counter;
  tpeers = newPeers;
  
  
  
  // draw nodes
  
  var n = nodes.length;
  
  var centerx = c.width / 2;
  var centery = c.height / 2;

  var mult = Math.log(10 + nodes.length) + 0.5;
  
  var outer_r = ((10 + n) * mult);
  if(outer_r > c.width || outer_r > c.height) {
    var smaller = c.height;
    if(c.width < smaller) smaller = c.width;
    outer_r = smaller - 20;
  }
  
  var inner_r = outer_r / 2;
  draw.beginPath();
  draw.lineWidth = 1;
  draw.strokeStyle = "white";
  draw.arc(centerx, centery, inner_r, 0, 2* Math.PI);
  draw.stroke();


  var idents = [];
  var rad = 0;
  for( var ident in nodes ) {
    if (ident == "length" ) {
      continue;
    }
    idents.push( ident );
  }

  idents = idents.sort();
  for (var i = 0; i < idents.length; i++) {
    var ident = idents[i];
    rad += ( Math.PI * 2 ) / nodes.length;

    in_traffic += nodes[ident].recv / 10.0 ;
    out_traffic += nodes[ident].send / 10.0;
    
    var send = (nodes[ident].send ) * 5;
    var recv = (nodes[ident].recv ) * 5;

    var traff = (getTraffic(nodes[ident]) - 1) * 5;
    
    var x0 = (Math.cos(rad) * inner_r) + centerx;
    var y0 = (Math.sin(rad) * inner_r) + centery;
    var x1 = (Math.cos(rad) * (inner_r + traff )) + centerx;
    var y1 = (Math.sin(rad) * (inner_r + traff )) + centery;

    draw.fillStyle = "white";
    draw.beginPath();
    var txt = ident.substr(0, 6);
    draw.fillText(txt, x1-5, y1-5);
    /**
    if(i * 10 < c.height) {
      txt += "| "+leftpad(nodes[ident].recv+" msg/s in", 15)+ " | "+leftpad(nodes[ident].send+" msg/s out", 15)+" |";
      draw.fillText(txt, 100, 20 + (i*10));
    } */
    nodes[ident].recv = 0;
    nodes[ident].send = 0;
    draw.moveTo(x0, y0);

    draw.strokeStyle = "#dfa";

    draw.moveTo(x0, y0);
    draw.lineTo(x1, y1);
    draw.lineWidth = 8;
    draw.stroke();
    /*
    if(( 40 + idx ) < c.height) {
      var send = nodes[ident].send * 10;
      var recv = nodes[ident].recv * 10;
      var t = send + recv;
      if (!t) continue;
      draw.fillStyle = "black";
      draw.fillText(ident.substr(0, 6), 10, idx);
      draw.beginPath();
      draw.rect(100, idx-20, send, 10);
      if(send >= c.width) draw.fillStyle = "red";
      else draw.fillStyle = "green";
      draw.fill();
      draw.beginPath();
      draw.rect(100, idx-10, recv, 10);
      if(recv >= c.width) draw.fillStyle = "red";
      else draw.fillStyle = "blue";
      draw.fill();

      idx += 40;
    }
    */

  }
  draw.fillStyle = "white";
  draw.fillText("Tracked Peers   | " +leftpad(" "+ nodes.length, 10), 500, 25);
  draw.fillText("Tracked Tunnels | " +leftpad(" "+ tunnels.length, 10), 500, 35);
  draw.fillText("In Traffic      | "+leftpad(" "+Math.ceil(in_traffic)+" msg/s", 10), 500, 45);
  draw.fillText("Out Traffic     | "+leftpad(" "+Math.ceil(out_traffic)+" msg/s", 10), 500, 55);
  
  if (tick % 10 == 0) {
    in_traffic = 0;
    out_traffic = 0;
  }
  tick ++;
}, 100);

logit("loaded");
