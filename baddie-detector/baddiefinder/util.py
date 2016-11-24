def getaddress(info):
    """
    get ip address from router info dict
    """
    for addr in info.addrs:
        opts = addr.options
        if b'host' in opts:
            h = opts[b'host']
            if b':' not in h:
                return h

def getcaps(info):
    """
    extract router caps
    """
    if b'caps' in info.options:
        return info.options[b'caps']
