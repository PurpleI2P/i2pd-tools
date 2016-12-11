
function ws_try_connect()
{


    var elem = document.getElementById("irc-window");
    elem.remove();
    elem = document.createElement("div");
    elem.id ="irc-window";
    document.body.appendChild(elem);

    var text_e = document.createElement("pre");
    text_e.setAttribute("class", "text-window");
    elem.appendChild(text_e);
    var input = document.createElement("input");
    elem.appendChild(input);


    var irc = {
        connected: 0,
        line_in: "",
        target: "",
        nick: "ebin",
    };

    function irc_ui_println(line)
    {
        if(line == "") return;
        var node = document.createTextNode(line);
        var e = document.createElement("div");
        e.appendChild(node);
        text_e.appendChild(e);
        while(text_e.children.length > 20) {
            text_e.children[0].remove();
        }
    }

    function irc_on_privmsg(src, msg)
    {
        var parts = src.split("!");
        src = parts[0].slice(1);
        irc_ui_println("<"+src+ "> "+msg);
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
            if (cmd == "PRIVMSG") {
                var idx = line.indexOf(target);
                var msg = line.slice(idx+target.length+2);
                irc_on_privmsg(src, msg);
                return;
            }
            if(cmd == "PONG") return;
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
        conn.send(line + "\n");
    }

    function irc_privmsg(conn, target, msg)
    {
        irc_ui_println("<"+irc.nick+"> "+msg);
        irc_sendline(conn, "PRIVMSG "+target+" :"+msg);
    }

    function irc_join_channel(conn, chnl)
    {
        irc_sendline(conn, "JOIN "+chnl);
    }

    function handle_input_command(conn, arg, params)
    {
        arg = arg.toLowerCase();
        if (arg == "j" || arg == "join") {
            for (var idx = 0 ; idx < params.length; idx ++) {
                irc_join_channel(conn, params[idx]);
            }
        }

        if(arg == "n" || arg == "nick") {
            irc_sendline(conn, "NICK "+params[0]);
            return;
        }

        if(arg == "q" || arg == "quit") {
            irc_sendline(conn, "QUIT");
            clearInterval(irc.pinger);
            irc.connected = 0;
            return;
        }
        if(arg == "t" || arg == "target") {
            irc.target = params[0];
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
    }

    function irc_connected(conn, url)
    {
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
    var ws = new WebSocket(e.value);
    e = document.getElementById("irc-server");
    var irc_url = e.value;

    ws.onclose = function(err) {
        console.log("connection closed "+err);
        irc.connected = 0;
        clearInterval(irc.pinger);
        irc_ui_println("connection closed");
    }

    ws.onopen = function(ev) {
        console.log("connect to "+irc_url);
        irc_ui_println("trying to connect to "+url_url);
        ws.send(irc_url);
    }
    ws.onmessage = function(ev) {
        var data = ev.data;
        if(irc.connected) {
            irc_data(ws, data);
        } else {
            var j = JSON.parse(data);
            if(j.error) {
                console.log("WebSOCKS error: "+j.error);
                console.log("try again");
                setTimeout(function() {
                    ws.send(irc_url);
                }, 1000);
            }
            irc.connected = j.success == "1";
            if(irc.connected) {
                irc_connected(ws, irc_url);
            }
        }
    }


}
