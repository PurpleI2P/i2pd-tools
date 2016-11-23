#
#
#

import netdb


from argparse import ArgumentParser as AP

from . import settings
from . import filter
from . import processor

def main():
    ap = AP()
    ap.add_argument("--settings", default="baddies.ini")

    args = ap.parse_args()
    s = settings.load(args.settings)
    fmax = s.get("thresholds", "max_floodfills_per_ip", fallback=5)
    f = filter.FloodfillFilter(fmax)
    p = processor.BaddieProcessor([f])
    netdb.inspect(p.hook)
    with open(s.get("output", "file", fallback="baddies.txt"), 'w') as f:
        p.write_blocklist(f)


if __name__ == "__main__":
    main()
