Code based on [this blog post by phil](http://pbambridge.brunelweb.net/blog/?p=64).

Code requires the [Jansson C library for encoding, decoding and manipulating JSON data](http://www.digip.org/jansson/).

Build
-----

Follow [the instructions here](http://www.digip.org/jansson/doc/dev/gettingstarted.html#compiling-and-installing-jansson).

To run `autoreconf` you must first install `libtool` (`sudo apt-get install libtool`).

To run the Jansson examples you need `curl`. Run `sudo apt-get install libcurl4-gnutls-dev`.

Once curl and Jansson have been installed/built, you need to install the built Jansson library. Run `sudo ldconfig`.

Example output
--------------

`bbc-curl.c` uses `libcurl` and in-memory download. Use this.

```
now=1346097509, last=1346097451, diff=58.000000, should refresh=0
now=1346097511, last=1346097451, diff=60.000000, should refresh=0
now=1346097513, last=1346097451, diff=62.000000, should refresh=1
38311 bytes retrieved
Found 30 broadcasts
now=1346097515, last=1346097513, diff=2.000000, should refresh=0
now=1346097517, last=1346097513, diff=4.000000, should refresh=0
```

Old example output
------------------

`bbc.c` uses `wget` and files. Do not use.

```
pi@raspberrypi /git/raspi-fun/c/lcd-bbc-json/grimbo $ sudo ./bbctests
grimbo_url_url_to_buffer(http://www.bbc.co.uk/tv/programmes/genres/factual/scienceandnature/schedules/upcoming.json)
Using temp file /tmp/grimbo_url_R6v1wk
File descriptor 3
grimbo_url_url_to_file_descriptor(http://www.bbc.co.uk/tv/programmes/genres/factual/scienceandnature/schedules/upcoming.json, 7704584)
grimbo_url_wget(http://www.bbc.co.uk/tv/programmes/genres/factual/scienceandnature/schedules/upcoming.json)
cmd=wget -qO- http://www.bbc.co.uk/tv/programmes/genres/factual/scienceandnature/schedules/upcoming.json
grimbo_url_copy(7704952, 7704584)
read 10239 bytes
read 10239 bytes
read 10239 bytes
read 8768 bytes
written 39485 bytes
written=39485
status=0
Child exit code: 0
buffer=7704952
page=1.000000
total=66.000000
offset=0.000000
```
