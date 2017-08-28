var WebSocks = require("./websocks.js").WebSocks;


function ws_try_connect()
{

    var elem = document.getElementById("irc-window");
    elem.remove();
    elem = document.createElement("div");
    elem.id ="irc-window";
    document.body.appendChild(elem);

    var input = document.createElement("input");
    input.setAttribute("class", "text-input");
    elem.appendChild(input);


    var irc = {
        connected: 0,
        line_in: "",
        target: "",
        nick: "ebin",
        panes: {}
    };

    function irc_ui_show_pane(pane)
    {
        irc_ui_ensure_pane(pane);
        // hide all other panes
        for (var k in irc.panes) {
            if(k == pane) {
                irc.panes[k].elem.style = "display: inline-block";
                irc.target = k;
            } else {
                irc.panes[k].elem.style = "display: none";
            }
        }
    }

    function irc_ui_ensure_pane(pane)
    {
        if(irc.panes[pane]) return;
        var e = document.createElement("pre");
        e.setAttribute("class", "text-window");
        e.style = "display: none";
        e.setAttribute("panename", pane);
        irc.panes[pane] = {
            elem: e,
            name: pane
        };
        var root = document.getElementById("irc-window");
        root.appendChild(e);
    }

    function irc_ui_println(line, pane)
    {
        if(line == "") return;
        var node = document.createTextNode(line);
        var e = document.createElement("div");
        e.appendChild(node);
        if(!pane) {
            pane = " ";
        }
        irc_ui_ensure_pane(pane);
        var p = irc.panes[pane];
        if(p) {
            p.elem.appendChild(e);
            if (pane == irc.target) {
                window.scroll(0, p.elem.offsetTop + p.elem.offsetHeight);
            }
        } else {
            console.log("No pane called "+pane);
        }

    }

    function irc_on_privmsg(src, target, msg)
    {
        var parts = src.split("!");
        src = parts[0].slice(1);
        irc_ui_println("<"+src+ "> "+msg, target);
    }

    function irc_on_greeted(conn)
    {
        irc_ui_println("successfully joined irc");
    }

    function irc_on_join(src, target)
    {
        irc_ui_println("--> "+src, target);
    }

    function irc_on_part(src, target)
    {
        irc_ui_println("<-- "+src, target);
    }

    function irc_on_other(src, cmd, target, msg)
    {
        irc_ui_println("<"+src+"> "+msg, src);
    }

    function irc_process_in(conn)
    {
        var line = irc.line_in.trim();
        console.log("--> "+line);
        if(line.startsWith("PING ")) {
            // handle ping
            irc_sendline(conn, "PONG "+line.slice(5));
            return;
        }
        var parts = line.split(" ");
        if (parts.length > 2) {
            var src = parts[0];
            var cmd = parts[1];
            var target = parts[2];
            var idx = line.indexOf(target);
            var msg = line.slice(idx+target.length+2);
            if (cmd == "PRIVMSG") {
                irc_on_privmsg(src, target, msg);
                return;
            }
            if (cmd == "JOIN" ) {
                irc_on_join(src, target);
                return;
            }
            if (cmd == "PART") {
                irc_on_part(src, target);
                return;
            }

            if(cmd == "PONG") return;
            if(cmd == "376") {
                // we have been greeted fully
                irc_on_greeted(conn);
                return;
            }
            if (cmd == "433") {
                irc.nick = irc.nick + "_";
                irc_sendline(conn, "NICK "+irc.nick);
                irc_sendline(conn, "USER "+irc.nick+" "+irc.nick+" "+irc.nick+" :"+irc.nick);
            }
            irc_on_other(src.slice(1), cmd, target, msg);
        }
        irc_ui_println(line);
    }

    function irc_data(conn, data)
    {
        data = irc.line_in + data;
        var lines = data.split("\n");
        for(var idx = 0; idx < lines.length; idx++) {
            irc.line_in = lines [idx] + "\n";
            irc_process_in(conn);
        }
    }

    function irc_sendline(conn, line)
    {
        console.log("<-- "+ line);
        conn.write(line + "\n");
    }

    function irc_privmsg(conn, target, msg)
    {
        irc_ui_println("<"+irc.nick+"> "+msg, target);
        irc_sendline(conn, "PRIVMSG "+target+" :"+msg);
    }

    function irc_join_channel(conn, chnl)
    {
        irc_sendline(conn, "JOIN "+chnl);
        irc_ui_ensure_pane(chnl);
    }

    function handle_input_command(conn, arg, params)
    {
        arg = arg.toLowerCase();
        if (arg == "j" || arg == "join") {
            for (var idx = 0 ; idx < params.length; idx ++) {
                irc_join_channel(conn, params[idx]);
            }
            return;
        }

        if(arg == "lp" || arg == "listpanes") {
            irc_ui_println("--- begin list of panes", irc.target);
            for (var k in irc.panes) {
                if(k == irc.target) {
                    irc_ui_println("(active) : "+k, irc.target);
                } else {
                    irc_ui_println("         : "+k, irc.target);
                }
            }
            irc_ui_println("--- end list of panes", irc.target);
            return;
        }


        if(arg == "m" || arg == "msg") {
            irc_privmsg(conn, params[0], params.slice(1).join(" "));
            return;
        }

        if(arg == "n" || arg == "nick") {
            irc_sendline(conn, "NICK "+params[0]);
            irc.nick = params[0];
            return;
        }
        if(arg == "r" || arg == "raw") {
            irc_sendline(conn, params.join(" "));
            return;
        }
        if(arg == "q" || arg == "quit") {
            irc_sendline(conn, "QUIT");
            return;
        }
        if(arg == "w" || arg == "window") {
            irc.target = params[0];
            irc_ui_show_pane(irc.target);
            return;
        }
    }

    function handle_input_line(conn, line)
    {
        if(line[0] == "/") {
            var parts = line.split(" ");
            handle_input_command(conn, parts[0].slice(1), parts.slice(1));
        } else {
            irc_privmsg(conn, irc.target, line);
        }
    };

    function irc_connected(conn, url)
    {
        irc.connected = 1;
        console.log("connected to irc");
        irc_ui_println("connecting to "+url+"...");
        // send user command
        irc_sendline(conn, "NICK "+irc.nick);
        irc_sendline(conn, "USER "+irc.nick+" "+irc.nick+" "+irc.nick+" :"+irc.nick);
        irc.pinger = setInterval(function(){
            if(irc.connected) {
                irc_sendline(conn, "PING :i-hate-tcp-lol-"+new Date().getTime());
            }
        }, 10000);
        input.addEventListener("keypress", function(ev){
            // handle enter key
            switch(ev.key) {
            case "Enter":
                handle_input_line(conn, input.value);
                ev.preventDefault();
                input.value = "";
            }
            return;
        });
    }

    var e = document.getElementById("ws-server");
    var wsurl = e.value;
    e = document.getElementById("irc-server");
    var irc_url = e.value;

    var parts = irc_url.split(":");
    var host = parts[0];
    var port = 6667;
    if(parts.length != 1) {
        port = parts[1];
    }

    irc_ui_show_pane(" ");

    var sock = new WebSocks(wsurl);

    sock.connect(host, port, function(err, writer, reader) {
        if(err) {
            console.log(err);
        } else {
            irc_connected(writer);
            reader.on("data", function(data) {
                irc_data(writer, data);
            });
        }
    });
};

document.getElementById("connect").onclick = function() {
    console.log("try connect");
    ws_try_connect();
};
