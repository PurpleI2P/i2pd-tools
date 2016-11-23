def getaddress(info):
    """
    get ip address from router info dict
    """
    for addr in info.addrs:
        opts = addr.options
        if b'host' in opts:
            return opts[b'host']

def getcaps(info):
    """
    extract router caps
    """
    if b'caps' in info.options:
        return info.options[b'caps']
