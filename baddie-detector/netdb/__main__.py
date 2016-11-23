#
# main driver
#


from .netdb import inspect

def print_entry(ent):
    print (ent)

if __name__ == '__main__':
    inspect(hook=print_entry)
