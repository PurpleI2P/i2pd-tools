


var c = document.getElementById("main");
var nodes = {
  length: 0,
};
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


var ws = new WebSocket("ws://127.0.0.1:7665/");
ws.onmessage = function(ev) {
  var j = JSON.parse(ev.data);
  if (j) {
    if(j.type == "transport.connected") {
      nodeConnected(j.ident);
    } else if (j.type == "transport.disconnected") {
      nodeDisconnected(j.ident);
    } else if (j.type == "transport.sendmsg") {
      nodeSend(j.ident, j.number);
    } else if (j.type == "transport.recvmsg") {
      nodeRecv(j.ident, j.number);
    }
  }
};

var draw = c.getContext("2d");

draw.font = "10px monospace";

setInterval(function() {
  draw.clearRect(0, 0, c.width, c.height);
  draw.fillStyle = "white";
  draw.fillText("Active Peers: " + nodes.length, 500, 25);

  
  var n = nodes.length;
  
  var centerx = c.width / 2;
  var centery = c.height / 2;

  var outer_r = (n * 5);
  
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
    var send = nodes[ident].send * 5;
    var recv = nodes[ident].recv * 5;
    
    var x0 = (Math.cos(rad) * inner_r) + centerx;
    var y0 = (Math.sin(rad) * inner_r) + centery;
    var x1 = (Math.cos(rad) * (inner_r + send )) + centerx;
    var y1 = (Math.sin(rad) * (inner_r + send )) + centery;
    var bigger = send;
    if (recv > bigger) bigger = recv;
    var x2 = (Math.cos(rad) * (inner_r + bigger )) + centerx;
    var y2 = (Math.sin(rad) * (inner_r + bigger )) + centery;
    
    draw.fillStyle = "white";
    draw.beginPath();
    var txt = ident.substr(0, 6);
    draw.fillText(txt, x2-5, y2-5);
    txt += "| "+leftpad(nodes[ident].recv+" msg/s in", 15)+ " | "+leftpad(nodes[ident].send+" msg/s out", 15)+" |";
    draw.fillText(txt, 100, 20 + (i*10));
    draw.moveTo(x0, y0);

    draw.strokeStyle = "#dfa";

    draw.moveTo(x0, y0);
    draw.lineTo(x1, y1);
    draw.lineWidth = 8;
    draw.stroke();
    
    x1 = (Math.cos(rad) * (inner_r + recv )) + centerx;
    y1 = (Math.sin(rad) * (inner_r + recv )) + centery;
    
    draw.beginPath();
    draw.strokeStyle = "#fda";
    draw.moveTo(x0, y0);
    draw.lineTo(x1, y1);
    draw.lineWidth = 10;
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
    nodes[ident].send = 0;
    nodes[ident].recv = 0;
  }
}, 1000);
