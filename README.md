# FreelyCommute

FreelyCommute is swiss-army knife that helps ease converting and publishing mathematical documents. The major painpoint of a working mathematician is that they author mathematics in the form of hand-written notes, and later go through the pain of type-setting and publishing their documents; since the latter process is both time-consuming and tedious, a majority of the notes are simply not worth publishing. The long-term vision of FreelyCommute is to completely eliminate the second step, and it plans to achieve this gradually, over the course of several years. "Commute" is part of the name, because the author of the project studies higher category theory, and his notes have three main components: hand-writing, mathematical symbols, and commutative diagrams. These three components will be the primary focus of the project.

The author of the project uses an e-ink tablet for all his work, namely the reMarkable 2. Therefore, the project will focus on dealing with vector images, in the SVG format, and will ease connecting to the reMarkable cloud, and ease converting and publishing the documents stored therein to a website. Mathematicians are old-fashioned, and often stick to publishing plain HTML documents. The author, however, is slightly more sophisticated in this regard, and has built a static site generator, called [clayoven](https://github.com/artagnon/clayoven), to automatically produce HTML documents from a simple markup. The initial focus of the project will, therefore, be to connect to the reMarkable cloud and automatically update a clayoven site. For those not wishing to use clayoven, it will cost the project little to produce a HTML files corresponding to notebooks on the reMarkable cloud, with minimal styling. Additionally, since notebooks may be updated after they are first uploaded, FreelyCommute will maintain a mapping between the notebooks and the directory containing .svg files, and write out new .svg files only when necessary.

Finally, to alleviate the pain of having to use a command-line interface, and write mapping files by hand, an iOS application is planned.

## Command-line interface

In line with the usecases outlined above, we present a simple command-line interface:

```sh
$ fc rM <path to clayoven website> # the author's usecase
$ fc rM --no-clayoven <path to write .svg and .html files to> # for people who don't wish to use clayoven
```

In addition to these high-level commands, several low-level commands are available:

```sh
$ fc rM # connect to the reMarkable cloud, and print a directory tree
$ fc rM --sync <directory mirroring rM cloud> # a one-way mirroring
$ fc rM --local <directory containing a mirror of the rM cloud> <path to clayoven website> # do the process locally
$ fc rM  --status <path to clayoven website> # print the mapping status
$ fc parse <.rM input file> # print raw parse-information about the .rM file
$ fc parse <.rM input file> <.svg output> # convert a .rM file to a .svg file
$ fc parse --xy <file containing a snippet of xypic> # to verify the grammar model of the TeX engine
$ fc serve # start a web server serving the entire application over HTTPS
```

A unified view:

```sh
$ fc rM [--no-clayoven] [--local \<mirror directory\>] [--status] [--sync] [output directory]
$ fc parse [--xy] \<input file\> [output file]
$ fc serve
```

## Project status

The project is in its early stages of development, and the author does not commit to any timeline.
