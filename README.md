moses.js
==========

### Moses Statistical Machine Translation (SMT) library for Node.js

[Moses](http://www.statmt.org/moses/) is a statistical machine translation system written in C++. This module enables usage of Moses in Node.js using the JavaScript language. In addition, it also adds the capability of loading multiple translation systems into the same node process. This means the same process (or web server built with node and [express](http://expressjs.com/)) can hold multiple distinctly different translation models (e.g., Chinese to English in IT and English to French in medicine) at the same time, and be able to use those models to translate user-given sentences or paragraphs on-demand.

Preparation
----------

The installation process is a little tricky, because this module depends on Moses, and Moses has a few dependencies. The instructions below originally come from Moses' [Moses Installation and Training Run-Through](http://www.statmt.org/moses_steps.html) page.

### Mac OSX

Supposing `$WORK` (which may be set to `$HOME/translation`) is the directory to hold source and build files, and `$TRANSLATION` (which may be set to `/opt/translation`) is the directory where executables, libraries and header files are to be stored.

Before proceeding, Xcode must be installed. After installing Xcode, launch it, and install `Command Line Tools` in `Xcode -> Preferences -> Downloads -> Components tab`. This will provide the standard build tool for command line usage (with some modifications).

#### MacPorts

[MacPorts](http://www.macports.org/) is provides additional necessary build tools. After installing MacPorts, install autoconf and automake.

```bash
$ sudo /opt/local/bin/port install autoconf automake
```

#### Boost

Download latest version of Boost library from [boost.org](http://www.boost.org/). Suppose the package is `$WORK/boost_1_54_0.tar.gz`. Install it with the following steps.

```bash
$ cd $WORK
$ tar -zxvf boost_1_54_0.tar.gz
$ cd boost_1_54_0
$ ./bootstrap.sh --prefix=$TRANSLATION/boost
$ sudo ./b2 install
```

#### GIZA++

Download the latest version of [GIZA++](https://code.google.com/p/giza-pp/). Suppose the package is `$WORK/giza-pp-v1.0.7.tar.gz`. Install it with the following steps.

```bash
$ cd $WORK
$ tar -zxvf giza-pp-v1.0.7.tar.gz
$ cd giza-pp
$ make
$ sudo mkdir -p $TRANSLATION/giza-pp/bin
$ sudo cp GIZA++-v2/{GIZA++,snt2cooc.out} mkcls-v2/mkcls $TRANSLATION/giza-pp/bin
```

#### SRILM

Download the latest version of [SRILM](http://www.speech.sri.com/projects/srilm/). Suppose the package is `$WORK/srilm.tgz`. Install it with the following steps.

```bash
$ cd $WORK
$ mkdir srilm
$ tar -zxvf srilm.tgz -C srilm
$ env SRILM=`pwd` make
$ sudo mkdir $TRANSLATION/srilm
$ sudo cp -r {bin,include,lib} $TRANSLATION/srilm
$ cd $TRANSLATION/srilm
$ sudo ln -s lib/macosx lib64
```

#### IRSTLM

Download the latest version of [IRSTLM](http://hlt.fbk.eu/en/irstlm). Suppose the package is `$WORK/irstlm-5.80.03.tgz`. Install it with the following steps.

```bash
$ cd $WORK
$ tar -zxvf irstlm-5.80.03.tgz
$ ./regenerate-makefiles.sh
$ ./configure --prefix=$TRANSLATION/irstlm
$ make
$ sudo make install
```

#### Moses

Download [Moses release 1.0](https://github.com/moses-smt/mosesdecoder/tree/RELEASE-1.0) (with the Download ZIP button). Suppose the package is `$WORK/mosesdecoder-RELEASE-1.0.zip`.

```bash
$ cd $WORK
$ unzip mosesdecoder-RELEASE-1.0.zip
$ cd mosesdecoder-RELEASE-1.0
$ ./bjam --with-srilm=$TRANSLATION/srilm --with-irstlm=$TRANSLATION/irstlm --with-giza=$TRANSLATION/giza-pp --with-boost=$TRANSLATION/boost --prefix=$TRANSLATION/moses -j2 -sLDFLAGS="-liconv -lboost_program_options"
$ sudo ./bjam --with-srilm=$TRANSLATION/srilm --with-irstlm=$TRANSLATION/irstlm --with-giza=$TRANSLATION/giza-pp --with-boost=$TRANSLATION/boost --prefix=$TRANSLATION/moses -j2 -sLDFLAGS="-liconv -lboost_program_options" install
```

#### Environment

The following script sets up the environment for running the command-line tools.

```bash
#!/bin/sh

where=$TRANSLATION

DYLD_LIBRARY_PATH=
for lib in $where/*/lib; do
  if [ x"$DYLD_LIBRARY_PATH" == x ]; then
    DYLD_LIBRARY_PATH=$lib
  else
    DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$lib
  fi
done
export DYLD_LIBRARY_PATH

for bin in $where/*/bin $where/*/bin/macosx; do
  PATH=$bin:$PATH
done
export PATH

export BOOST=$where/boost
export MOSES=$where/moses
export SRILM=$where/srilm
```

Put this file in `$TRANSLATION/setenv.sh`. With this, each time a new command-line console is open, execute the following command to set up.

```bash
$ . $TRANSLATION/setenv.sh
```

#### Test

Download Moses' [sample models package](http://www.statmt.org/moses/download/sample-models.tgz). Suppose the package is `$WORK/sample-models.tgz`.

```bash
$ cd $WORK
$ tar -zxvf sample-models.tgz
$ cd sample-models
$ . $TRANSLATION/setenv.sh
$ moses -f phrase-model/moses.ini < phrase-model/in
```

If all is set up correctly, near the end of the output, there should be a line containing text `BEST TRANSLATION: this is a small house`.