##
## i2p netdb parser
##
## Author: Jeff
## MIT Liecense 2014
##
import os,sys,struct,time,hashlib,fnmatch,io
import base64
import logging
import pygeoip

geo = pygeoip.GeoIP('/usr/share/GeoIP/GeoIPCity.dat')

b64encode = lambda x : base64.b64encode(x, b'~-').decode('ascii')

def sha256(data,raw=True):
    """
    compute sha256 of data
    """
    h = hashlib.new('sha256')
    h.update(data)
    if raw:
        return h.digest()
    else:
        return h.hexdigest()

class Inspector:

    _log = logging.getLogger('NDBInspector')
    
    def inspect(self, entry):
        pass
        
    def run(self, ndb):
        entry_counter = 0
        for root, dirnames, filenames in os.walk(ndb):
            for filename in fnmatch.filter(filenames, '*.dat'):
                fname = os.path.join(root, filename)
                e = Entry(fname)
                e.verify()
                if e.valid:
                    entry_counter += 1
                    self.inspect(e)
                else:
                    self._log.warn('invalid entry in file {}'.format(fname))
        self._log.info('read {} entries'.format(entry_counter))

class Address:
    """
    netdb address
    """
    cost = None
    transport = None
    options = None
    expire = None
    location = None

    def valid(self):
        return None not in (self.cost, self.transport, self.options, self.expire)
    
    def __repr__(self):
        return 'Address: transport={} cost={} expire={} options={} location={} firewalled={}' \
            .format(self.transport, self.cost, self.expire, self.options, self.location, self.firewalled)
    
class Entry:
    """
    netdb entry
    """
    _pubkey_size = 256
    _signkey_size = 128
    _min_cert_size = 3

    _log = logging.getLogger('NDBEntry')

    @staticmethod
    def _read_short(fd):
        Entry._log.debug('read_short')
        d = Entry._read(fd, 2)
        if d:
            return struct.unpack('!H',d)[0]

    @staticmethod
    def _read_mapping(fd):
        Entry._log.debug('read_mapping')
        mapping = dict()
        tsize = Entry._read_short(fd)
        if tsize is None:
            return
        data = Entry._read(fd, tsize)
        if data is None:
            return
        sfd = io.BytesIO(data)
        ind = 0
        while ind < tsize:
            Entry._log.debug(ind)
            key = Entry._read_string(sfd)
            if key is None:
                return
            Entry._log.debug(['key', key])
    
            ind += len(key) + 2
            Entry._read_byte(sfd)
            val = Entry._read_string(sfd)
            if val is None:
                return
            Entry._log.debug(['val',val])

            ind += len(val) + 2
            Entry._read_byte(sfd)
    
            #key = key[:-1]
            #val = val[:-1]
            if key in mapping:
                v = mapping[key]
                if isinstance(v,list):
                    mapping[key].append(val)
                else:
                    mapping[key] = [v,val]
            else:
                mapping[key] = val
        return mapping

    @staticmethod
    def _read(fd, amount):
        dat = fd.read(amount)
        Entry._log.debug('read %d of %d bytes' % (len(dat), amount))
        if len(dat) == amount:
            return dat
        

    @staticmethod 
    def _read_byte(fd):
        b = Entry._read(fd,1)
        if b:
            return struct.unpack('!B', b)[0]

    @staticmethod 
    def _read_two_bytes(fd):
        b = Entry._read(fd,2)
        if b:
            return struct.unpack('!H', b)[0]
    
    @staticmethod
    def _read_string(fd):
        Entry._log.debug('read_string')
        slen = Entry._read_byte(fd)
        if slen:
            return Entry._read(fd, slen)

    @staticmethod
    def _read_time(fd):
        d = Entry._read(fd, 8)
        if d:
            li = struct.unpack('!Q', d)[0]
            return li

    @staticmethod
    def geolookup(entry):
        return geo.record_by_addr(entry)
        
    @staticmethod
    def _read_addr(fd):
        """
        load next router address
        """
        Entry._log.debug('read_addr')
        addr = Address()
        addr.cost = Entry._read_byte(fd)
        addr.expire = Entry._read_time(fd)
        addr.transport = Entry._read_string(fd)
        addr.options = Entry._read_mapping(fd)
        addr.firewalled = False
        if addr.valid():
            # This is a try because sometimes hostnames show up.
            # TODO: Make it allow host names.
            try:
                addr.location = geolookup(addr.options.get('host', None))
            except:
                addr.location = None

            # If the router is firewalled (i.e. has no 'host' mapping), then use the first introducer (of 3).
            # In the future it might be worth it to do something else, but this helps for geopip information for now.
            # http://i2p-projekt.i2p/en/docs/transport/ssu#ra
            if not addr.location:
                # If there are introducers then it's probably firewalled.
                addr.firewalled = True
                try:
                    addr.location = geolookup(addr.options.get('ihost0', None))
                except:
                    addr.location = None

            return addr

    def __init__(self, filename):
        """
        construct a NetDB Entry from a file
        """
        self.addrs = list()
        self.options = dict()
        self.pubkey = None
        self.signkey = None
        self.cert = None
        self.published = None
        self.signature = None
        self.peer_size = None
        self.valid = False
        try:
            with open(filename, 'rb') as fr:
                self._log.debug('load from file {}'.format(filename))
                self._load(fr)
                #self.routerHash = 
        except (IOError, OSError) as e:
                self._log.debug('load from file {} failed'.format(filename))
            
    def _load(self, fd):
        """
        load from file descriptor
        More docs: http://i2p-projekt.i2p/en/docs/spec/common-structures#struct_RouterInfo
        """

        # router identity http://i2p-projekt.i2p/en/docs/spec/common-structures#struct_RouterIdentity
        # Do not assume that these are always 387 bytes!
        # There are 387 bytes plus the certificate length specified at bytes 385-386, which may be non-zero.

        # Subtract because read the Certificate on it's own.
        data = self._read(fd, 387-self._min_cert_size)
        if data is None:
            return
        ind = 0

        # public key
        self.pubkey = sha256(data[ind:ind+self._pubkey_size])
        ind += self._pubkey_size

        # signing key (we hash this later due to RI changes in 0.9.12)
        self.signkey = data[ind:ind+self._signkey_size]
        ind + self._signkey_size

        # certificate
        self.cert = dict()

        # If it's not null, follow what happens here: http://i2p-projekt.i2p/en/docs/spec/common-structures#type_Certificate
        cert_type = self._read_byte(fd)
        cert_len = self._read_two_bytes(fd)
        if cert_type == 5 and cert_len != 0: # New format where extra information is in the cert.
            spkt = self._read_two_bytes(fd)
            cpkt = self._read_two_bytes(fd)
            if spkt == 0:
                self.cert['signature_type'],cert_padding,cert_extra = 'DSA_SHA1',0,0
            elif spkt == 1:
                self.cert['signature_type'],cert_padding,cert_extra  = 'ECDSA_SHA256_P256',64,0
            elif spkt == 2:
                self.cert['signature_type'],cert_padding,cert_extra  = 'ECDSA_SHA384_P384',32,0
            elif spkt == 3:
                self.cert['signature_type'],cert_padding,cert_extra  = 'ECDSA_SHA512_P521',0,4
            elif spkt == 4:
                self.cert['signature_type'],cert_padding,cert_extra  = 'RSA_SHA256_2048',0,128
            elif spkt == 5:
                self.cert['signature_type'],cert_padding,cert_extra  = 'RSA_SHA384_3072',0,256
            elif spkt == 6:
                self.cert['signature_type'],cert_padding,cert_extra  = 'RSA_SHA512_4096',0,384
            elif spkt == 7:
                self.cert['signature_type'],cert_padding,cert_extra  = 'EdDSA_SHA512_Ed25519',96,0
            else:
                Entry._log.debug('Bad cert sign type.')
                return
            
            # This is always going to be 0 (as of 0.9.19), but future versions can add more crypto types.
            if cpkt == 0:
                self.cert['crypto_type'] = 'ElGamal'
            else:
                Entry._log.debug('Bad cert crypto type.')
                return
        else: # Old format where information is all in the main part.
            self.cert['signature_type'],cert_padding,cert_extra = 'DSA_SHA1',0,0
            self.cert['crypto_type'] = 'ElGamal'

        # Parse public key properly (http://i2p-projekt.i2p/en/docs/spec/common-structures#type_Certificate)
        if cert_padding > 0:
            self.signkey = self.signkey[cert_padding:]
        if cert_extra > 0:
            self.signkey += self._read(fd,cert_extra)

        Entry._log.debug('parsed cert, sig type {}, crypto type {}.'.format(self.cert['signature_type'], self.cert['crypto_type']))
    
        self.signkey = sha256(self.signkey)

        # date published
        self.published  = self._read_time(fd)
        if self.published is None:
            return

        # reachable addresses
        self.addrs = list()
        addrlen = self._read_byte(fd)
        if addrlen is None:
            return
        for n in range(addrlen):
            addr = self._read_addr(fd)
            if addr is None:
                return
            self.addrs.append(addr)
 
        # peer size
        self.peer_size = self._read_byte(fd)
        if self.peer_size is None:
            return
        
        # other options
        self.options = self._read_mapping(fd)
        if self.options is None:
            return

        # signature
        self.signature = sha256(self._read(fd, 40))
        if self.signature is None:
            return
        self.valid = True
        
    def verify(self):
        """
        verify router identity
        """
        #TODO: verify 

    def __repr__(self):
        val = str()
        val += 'NetDB Entry '
        val += 'pubkey={} '.format(b64encode(self.pubkey))
        val += 'signkey={} '.format(b64encode(self.signkey))
        val += 'options={} '.format(self.options)
        val += 'addrs={} '.format(self.addrs)
        val += 'cert={} '.format(self.cert)
        val += 'published={} '.format(self.published)
        val += 'signature={}'.format(b64encode(self.signature))
        return val

    def dict(self):
        """
        return dictionary in old format
        """
        return dict({
            'pubkey':b64encode(self.pubkey),
            'signkey':b64encode(self.signkey),
            'options':self.options,
            'addrs':self.addrs,
            'cert':self.cert,
            'published':self.published,
            'signature':b64encode(self.signature)
            })

def inspect(hook=None,netdb_dir=os.path.join(os.environ['HOME'],'.i2pd','netDb')):
    """
    iterate through the netdb
    
    parameters:
    
      hook - function taking 1 parameter
        - the 1 parameter is a dictionary containing the info
          of a netdb enrty
        - called on every netdb entry

      netdb_dir - path to netdb folder
        - defaults to $HOME/.i2pd/netDb/

    """ 

    insp = Inspector()
    if hook is not None:
        insp.inspect = hook 
    insp.run(netdb_dir)

