import flask
import io
import netdb

from . import settings
from . import filter
from . import processor

app = flask.Flask(__name__)

@app.route("/baddies.txt")
def baddies():
    s = settings.load('baddies.ini')
    fmax = s.get("thresholds", "max_floodfills_per_ip", fallback=3)
    f = filter.FloodfillFilter(fmax)
    p = processor.BaddieProcessor([f])
    path = s.get("netdb", "dir", fallback=None)
    if path:
        netdb.inspect(p.hook, path)
    else:
        netdb.inspect(p.hook)
    body = io.StringIO()
    p.write_blocklist(body)
    v = body.getvalue()
    body.close()
    r = flask.Response(v)
    r.headers["Content-Type"] = 'text/plain'
    return r

@app.route("/")
def index():
    return """this server serves a router info blocklist of ip addresses with unreasonably high desnity of i2p routers (right now just floodfills)
"""


if __name__ == '__main__':
    app.run()
