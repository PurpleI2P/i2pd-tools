#!/usr/bin/env python
from setuptools import setup

setup(name = 'baddiedetector',
    version = '0.0',
    description = 'i2p netdb blocklist ',
    author = 'Jeff Becker',
    author_email = 'ampernand@gmail.com',
    install_requires = ['python-geoip','python-geoip-geolite2'],
    tests_require=['pytest'],
    url = 'https://github.com/purplei2p/i2pd-tools',
    packages = ['netdb', 'baddiefinder'],
)
