#
# baddiefinder settings wrapper
#

from configparser import ConfigParser

def load(fname):
    c = ConfigParser()
    with open(fname) as f:
        c.read_file(f, fname)
    return c
