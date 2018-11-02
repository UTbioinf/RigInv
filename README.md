Rigorous Inversion Detection
---

This tools uses long reads to detect rigorous inversions

---

# dependencies

* cmake >= 3.6
* python 2.7.x

# Install

* `./install.py`
    * parameters:
        * `--prefix`:
            * If presented, then the package will be installed into that directory
            * Otherwise, it will be installed into `<repo>/build/local/bin`
        * `--LOONLIB_ROOT_DIR`:
            * If presented, it will use that as the root path
            * If not presented, but `brew` is installed, it will use `brew --prefix` as that path.
            * Otherwise, there will be an error
* ~~Or, if you use linuxbrew or homebrew~~ : will be available soon
    * ~~Tap the formula repo: `brew tap zijuexiansheng/homebrew-filbat`~~
    * ~~Install RigInv using brew: `brew install zijuexiansheng/filbat/riginv`~~

# TODO

* add usage
* add code for removing files if the files were not to be updated by the script every time
* `src/cpp/segment_prediction.cpp` is unfinished!!!
