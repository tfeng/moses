moses.js
==========

### Moses Statistical Machine Translation (SMT) library for Node.js

[Moses](http://www.statmt.org/moses/) is a statistical machine translation system written in C++. This module enables usage of Moses in Node.js using the JavaScript language. In addition, it also adds the capability of loading multiple translation systems into the same node process. This means the same process (or web server built with node and [express](http://expressjs.com/)) can hold multiple distinctly different translation models (e.g., Chinese to English in IT and English to French in medicine) at the same time, and be able to use those models to translate user-given sentences or paragraphs on-demand.

Moses Installation
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

Download latest version of Boost library from [boost.org](http://www.boost.org/). Suppose the package is `$WORK/boost_1_54_0.tar.gz`.

```bash
$ cd $WORK
$ tar -zxvf boost_1_54_0.tar.gz
$ cd boost_1_54_0
$ ./bootstrap.sh --prefix=$TRANSLATION/boost
$ sudo ./b2 install
```

#### GIZA++

Download the latest version of [GIZA++](https://code.google.com/p/giza-pp/). Suppose the package is `$WORK/giza-pp-v1.0.7.tar.gz`.

```bash
$ cd $WORK
$ tar -zxvf giza-pp-v1.0.7.tar.gz
$ cd giza-pp
$ make
$ sudo mkdir -p $TRANSLATION/giza-pp/bin
$ sudo cp GIZA++-v2/{GIZA++,snt2cooc.out} mkcls-v2/mkcls $TRANSLATION/giza-pp/bin
```

#### SRILM

Download the latest version of [SRILM](http://www.speech.sri.com/projects/srilm/). Suppose the package is `$WORK/srilm.tgz`.

```bash
$ cd $WORK
$ mkdir srilm
$ tar -zxvf srilm.tgz -C srilm
$ cd srilm
$ env SRILM=`pwd` make
$ sudo mkdir $TRANSLATION/srilm
$ sudo cp -r {bin,include,lib} $TRANSLATION/srilm
```

SRILM build tool puts compiled libraries in `$TRANSLATION/srilm/lib/$SRILM_ARCH`, where `SRILM_ARCH` is an architecture-specific name, such as `macosx` for Mac and `i686-m64` for 64-bit Linux. For Moses build, however, the library must be located at `$TRANSLATION/srilm/lib64` for 64-bit systems. Therefore, perform the following steps to create the necessary symlink.

```bash
$ cd $TRANSLATION/srilm
$ sudo ln -s lib/* lib64
```

#### IRSTLM

Download the latest version of [IRSTLM](http://hlt.fbk.eu/en/irstlm). Suppose the package is `$WORK/irstlm-5.80.03.tgz`.

```bash
$ cd $WORK
$ tar -zxvf irstlm-5.80.03.tgz
$ cd irstlm-5.80.03
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

The following script sets up the environment for running the command-line tools. (Set variables `WORK` and `TRANSLATION` in this script according to your specific configuration.)

```bash
#!/bin/sh

export WORK=$HOME/translation
export TRANSLATION=/opt/translation

LD_LIBRARY_PATH=
for lib in $TRANSLATION/*/lib; do
  if [ x"$LD_LIBRARY_PATH" == x ]; then
    LD_LIBRARY_PATH=$lib
  else
    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$lib
  fi
done
export LD_LIBRARY_PATH
export DYLD_LIBRARY_PATH=LD_LIBRARY_PATH

for bin in $TRANSLATION/*/bin `find srilm/bin/* -type d`; do
  PATH=$bin:$PATH
done
export PATH

export BOOST=$TRANSLATION/boost
export MOSES=$TRANSLATION/moses
export SRILM=$TRANSLATION/srilm
export IRSTLM=$TRANSLATION/irstlm
```

Put this file in `$TRANSLATION/setenv.sh`. With this, each time a new command-line console is open, execute the following command to set up.

```bash
$ . $TRANSLATION/setenv.sh
```

#### Testing Moses

Download Moses' [sample models package](http://www.statmt.org/moses/download/sample-models.tgz). Suppose the package is `$WORK/sample-models.tgz`.

```bash
$ cd $WORK
$ tar -zxvf sample-models.tgz
$ cd sample-models
$ . $TRANSLATION/setenv.sh
$ moses -f phrase-model/moses.ini < phrase-model/in
```

If all is set up correctly, near the end of the output, there should be a line containing text `BEST TRANSLATION: this is a small house`.

moses.js Installation
----------

#### Patching Moses

As of release 1.0, Moses supports only one translation system per OS process. After the system is initialized, all subsequent translation must be performed in the same system. Though advanced features such as [Using Multiple Translation Systems in the Same Server](http://www.statmt.org/moses/?n=Moses.AdvancedFeatures#ntoc29) and [Alternate Weight Settings](http://www.statmt.org/moses/?n=Moses.AdvancedFeatures#ntoc52) allow users to have multiple translation tables, all those tables must be loaded at the same time, and once loaded, they cannot be changed in the entire life of the process.

A patch shipped with moses.js provides an enhancement that allows users to create and destroy translation systems on demand. This is especially useful for a Node.js server that runs for an extended period of time, and is expected to update its translation system dynamically (e.g., as result of a user request or service update). The patch also enables loading multiple translation systems at the same time, and selecting one of them for use with any given translation task.

To apply the patch, download [the latest version from moses.js' github server](https://raw.github.com/tfeng/moses/master/patches/moses-1.0.patch). Suppose the patch file is saved at `$WORK/moses-1.0.patch`.

```bash
$ cd $WORK/mosesdecoder-RELEASE-1.0
$ patch -p1 < ../moses-1.0.patch
$ sudo ./bjam --with-srilm=$TRANSLATION/srilm --with-irstlm=$TRANSLATION/irstlm --with-giza=$TRANSLATION/giza-pp --with-boost=$TRANSLATION/boost --prefix=$TRANSLATION/moses -j2 -sLDFLAGS="-liconv -lboost_program_options" install
```

This will rebuild Moses and reinstall it in `$TRANSLATION/moses`. After this, it is recommended to return to [Testing Moses](#testing-moses) to test moses again. The patch also modifies the main `moses` program so that it runs in the legacy single-translation-system mode, instead of the enhanced multi-translation-system mode.

#### Install Moses header files

Moses installation does not include header files required for building moses.js. Those files need to be manually copied to the installation directory.

```bash
$ sudo rm -rf $TRANSLATION/moses/include
$ sudo mkdir $TRANSLATION/moses/include
$ sudo cp -R $WORK/mosesdecoder-RELEASE-1.0/headers-moses $TRANSLATION/moses/include/moses
$ sudo cp -R $WORK/mosesdecoder-RELEASE-1.0/headers-base/* $TRANSLATION/moses/include/
```

#### Installing moses.js

Install latest release of moses.js from [npm](https://npmjs.org/package/moses).

```bash
$ cd $WORK
$ mkdir test-moses-js
$ . $TRANSLATION/setenv.sh
$ npm install moses
$ node node_modules/moses/sample/test-phrase-model.js
> Defined parameters (per moses.ini or switch):
	config: phrase-model/moses.ini 
	input-factors: 0 
	lmodel-file: 8 0 3 lm/europarl.srilm.gz 
	mapping: T 0 
	n-best-list: nbest.txt 100 
	ttable-file: 0 0 0 1 phrase-model/phrase-table 
	ttable-limit: 10 
	weight-d: 1 
	weight-l: 1 
	weight-t: 1 
	weight-w: 0 
Overwriting parameter verbose (the parameter does not have previous values) with the following values: 0
Started.
> this is a small house
```
