#!/usr/bin/env python3

import os
import sys
import gzip
from correctionlib import schemav2

def main(indir, outdir):
    os.makedirs(outdir, exist_ok=True)
    for fname in sorted(os.listdir(indir)):
        if not (fname.endswith(".json") or fname.endswith(".json.gz")): continue
        inpath  = os.path.join(indir, fname)
        outpath = os.path.join(outdir, fname)
        print(f"[FILE] {fname}")
        try:
            raw = load_text(inpath)
            cset = schemav2.CorrectionSet.parse_raw(raw) ## Parse + validate using correctionlib
            fixed = cset.json(indent=2) ## Serialize back (this fixes non-monotone binning)
            write_text(fixed, outpath)
            print("  -> fixed and written")
        except Exception as e: print(f"  [ERROR] {e}")
    print("\n[OK] Done")

#--------------------------------------------------------
def load_text(path):
    if path.endswith(".gz"):
        with gzip.open(path, "rt") as f:
            return f.read()
    else:
        with open(path) as f:
            return f.read()

def write_text(text, path):
    if path.endswith(".gz"):
        with gzip.open(path, "wt") as f:
            f.write(text)
    else:
        with open(path, "w") as f:
            f.write(text)
#--------------------------------------------------------

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: fix_jsons.py <input_dir> <output_dir>")
        sys.exit(1)
    main(sys.argv[1], sys.argv[2])
