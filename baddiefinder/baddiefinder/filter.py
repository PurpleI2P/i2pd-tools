
from . import util

class Filter:

    name = "unnamed filter"
    
    def process(self, info):
        """
        process an info and return a string representation of a reason to add to blocklist
        any other return value will cause this info to NOT be added to blocklist
        """

class FloodfillFilter(Filter):

    name = "floodfill sybil detector"
    
    def __init__(self, fmax):
        self._floodfills = dict()
        self.fmax = int(fmax)

    def process(self, info):
        caps = util.getcaps(info)
        if not caps:
            return 
        if b'f' not in caps:
            return 
        h = util.getaddress(info)
        if h not in self._floodfills:
            self._floodfills[h] = 0
        self._floodfills[h] += 1
        if self._floodfills[h] > self.fmax:
            return '{} > {} floodfills per ip'.format(self._floodfills[h], self.fmax)
