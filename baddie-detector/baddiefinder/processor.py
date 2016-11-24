from . import util

import datetime

class BaddieProcessor:

    def __init__(self, filters):
        self._filters = filters
        self._baddies = dict()

        
    def hook(self, entry):
        now = datetime.datetime.now()
        for f in self._filters:
            if f.process(entry) is True:
                self.add_baddie(entry, 'detected by {} on {}'.format(f.name, now.strftime("%c").replace(":",'-')))

    def add_baddie(self, entry, reason):
        addr = util.getaddress(entry).decode('ascii')
        self._baddies[addr] = reason 

    def write_blocklist(self, f):
        f.write('# baddies blocklist generated on {}\n'.format(datetime.datetime.now()))
        for k in self._baddies:
            f.write('{}:{}\n'.format(self._baddies[k], k))
