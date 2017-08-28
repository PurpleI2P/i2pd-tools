WEBSOCKS_SRC = websocks.js
BT_OUT = websocks-bt.js
BT_SRC = bittorrent.js

IRC_OUT = websocks-irc.js
IRC_SRC = irc-client.js

all: $(BT_OUT) $(IRC_OUT)

$(IRC_OUT):
	browserify $(WEBSOCKS_SRC) $(IRC_SRC) -o $(IRC_OUT)

$(BT_OUT):
	browserify $(WEBSOCKS_SRC) $(BT_SRC) -o $(BT_OUT)

clean:
	rm -f $(BT_OUT) $(IRC_OUT)
