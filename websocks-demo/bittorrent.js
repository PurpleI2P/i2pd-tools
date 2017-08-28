var Protocol = require('bittorrent-protocol');
var stream = require('stream');
var WebSocks = require("./websocks.js").WebSocks;


function start_torrent(torrent_file_blob)
{
    var torrent = new WebsocksTorrent();
    var elem = document.getElementById("ws-server");
    torrent.start(elem.value);
    torrent.addTorrent(torrent_file_blob);
}

function start_downloading()
{
    var elem = document.getElementById("torrent-file");
    var files = elem.files;
    for (var f of files) {
        if(f.slice) {
            start_torrent(f.slice());
        }
    }
}
