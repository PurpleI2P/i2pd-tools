# test_netdb.py - Test netdb.py
# Author: Chris Barry <chris@barry.im>
# License: MIT

# Note: this uses py.test.

import netdb,os,random

'''
def test_inspect():
	netdb.inspect()
'''

def test_sha256():
	assert('d2f4e10adac32aeb600c2f57ba2bac1019a5c76baa65042714ed2678844320d0' == netdb.netdb.sha256('i2p is cool', raw=False))

def test_address_valid():
	invalid = netdb.netdb.Address()
	valid = netdb.netdb.Address()
	valid.cost = 10
	valid.transport = 'SSU'
	valid.options = {'host': '0.0.0.0', 'port': '1234', 'key': '', 'caps': ''}
	valid.expire = 0
	assert(valid.valid() and not invalid.valid())

def test_address_repr():
	valid = netdb.netdb.Address()
	valid.cost = 10
	valid.transport = 'SSU'
	valid.options = {'host': '0.0.0.0', 'port': '1234', 'key': '', 'caps': ''}
	valid.expire = 0
	assert(repr(valid) == 'Address: transport=SSU cost=10 expire=0 options={\'host\': \'0.0.0.0\', \'port\': \'1234\', \'key\': \'\', \'caps\': \'\'} location=None')

# TODO: test_entry*

def test_entry_read_short():
	assert(True)
def test_entry_read_mapping():
	assert(True)
def test_entry_read():
	assert(True)
def test_entry_read_short():
	assert(True)
def test_entry_read_byte():
	assert(True)
def test_entry_read_string():
	assert(True)
def test_entry_init():
	assert(True)
def test_entry_load():
	assert(True)
def test_entry_verify():
	assert(True)
def test_entry_repr():
	assert(True)
def test_entry_dict():
	assert(True)

# Make some garbage files and hope they break things.
def test_fuzz():
    pwd = os.environ['PWD']
    for i in range(1,100):
        with open('{}/fuzzdb/{}.dat'.format(pwd, i), 'wb') as fout:
            fout.write(os.urandom(random.randint(2,400))) # replace 1024 with size_kb if not unreasonably large
    # Now let's inspect the garbage.
    netdb.inspect(netdb_dir='{}/fuzzdb/'.format(pwd))
