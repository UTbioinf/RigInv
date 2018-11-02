#!/usr/bin/env python

import os

class FileWriteBuffer:
    def __init__(self, fname, fid, mode = "w"):
        self.fp = None
        self.fname = fname
        self.fid = fid
        self.mode = mode
        self.bytes = 0
        self.texts = []

    def is_open(self):
        return self.fp and not self.fp.closed

    def closed(self):
        return not self.fp or self.fp.closed

    def open(self):
        self.fp = open(self.fname, self.mode)
        self.mode = "a"

    def close(self, force = True):
        if not force:
            self.flush()
        self.fp.close()

    def flush(self):    # write to the disk
        self.fp.write("".join( self.texts ))
        self.bytes = 0
        self.texts = []

    def write(self, text):  # don't really write
        self.bytes += len(text)
        self.texts.append( text )

class ManyWriteFiles:
    def __init__(self, max_nfile = 512, cache_size = 1048576):
        """ManyWriteFiles

        max_nfile:  Keep at most this number of files open
        cache_size: For each file, keep this number of bytes in memory before flushing into the disk
        
        self._open_queue: A set of <fid> such that their file descriptors are open
        self._files: A dictionary of files of the format {<fid>: <FileWriteBuffer>}
        """
        self._max_nfile = max_nfile
        self._cache_size = cache_size
        self._open_queue = set()
        self._files = {}

    def open(self, fname, fid, mode = "w"):
        fp = FileWriteBuffer( fname, fid, mode )
        self._files[ fid ] = fp
        return fp

    def get(self, fid, default = None):
        return self._files.get(fid, default)

    def __getitem__(self, fid):
        return self._files[ fid ]

    def write(self, fp, text):
        fp.write( text )
        if fp.bytes >= self._cache_size:
            if fp.closed():
                self._safe_open(fp)
            fp.flush()

    def close(self):
        for fid, fp in self._files.iteritems():
            if fp.bytes > 0:
                if fp.closed():
                    self._safe_open(fp)
                fp.flush()
            fp.close()
        self._open_queue.clear()
        self._files.clear()
        
    def _safe_open(self, fp):
        if len(self._open_queue) >= self._max_nfile:
             self._files[ self._open_queue.pop() ].close()
        fp.open()
        self._open_queue.add( fp.fid )


if __name__ == "__main__":
    nfiles = 128
    n_tests = 1024
    files = ManyWriteFiles(nfiles)
    import util
    for i in xrange(n_tests):
        directory = os.path.join("working_dir", util.int2path(i))
        util.makedir( directory )
        fp = files.open( os.path.join(directory, str(i) + ".txt"), i)
        files.write(fp, "hello, world! {}\n".format(i))
    for i in xrange(n_tests):
        files.write(files[i], "hello, world again! {}\n".format(i))
    files.close()
    print "Write down! Do check"
    for i in xrange(n_tests):
        with open(os.path.join("working_dir", util.int2path(i), str(i) + ".txt")) as fin:
            assert(fin.read() == "hello, world! {}\nhello, world again! {}\n".format(i, i))
    print "Done!"
