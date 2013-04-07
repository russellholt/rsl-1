# rsl 1

(defunct)

this is rsl, the resource scripting language, version 1, and the framework for an early application
server. Originally written by russell holt (me) in 1995 at Destiny Software Corporation, in support
of our project to build the online banking system of a large multinational bank under both America
Online and the web.

Named after the 'resource' concept in HTTP, this small scripting language was designed to be
integrated into a web server to write web applications in a more abstracted and maintainable way
than cgi scripts.

Experience with this system was the basis for a complete rewrite in rsl2 and Destiny's app server
granite foundation.

## why does this exist

the original vision for rsl was to be a scripting language specifically designed to write web
applications. It was mean to minimize the need to deal directly with the details of HTTP or HTML.

## this is dead software

I'm putting in on github for educational and historical/hysterical purposes. It may or may not work.
There may or may not be pieces missing. I don't think there is much you can do with it, unless you
are interested in the internals of a simple object-based scripting language written in C++ with a
lex and yacc definition.  See also RSL-2 for a more complex, object-oriented example.

## what's here

- src : the main source. see the README.

- rmgserver: bits and pieces of an online banking application. I have no idea what state it is, but
  there is a lot missing. RMG stands for _remote managed gateway_, the term for the AOL application
  api.  In this case rsl with the extensions in this directory ran on a server at a client site and
  talked AOL's rmg prototcol to manage windows and user interaction under AOL. yay. Subsequently,
  that same server running the same application code (written in the rsl language) served the web
  application as well, demonstrating an unusual level of abstraction and modularity for
  client-server and web applications at the time. this was also the major driving reason to develop
  version 2, which made this all easier.

## (un) LICENSE

This is free and unencumbered software released into the public domain. See the file UNLICENSE.

In 1995 this was confidential and proprietary property of Destiny Software Corporation. This entity
no longer exists and this code has not been used in production since probably 1999, and has never
been made public. Since I am the primary author and probably the only one who cares, I am releasing
it into the public domain.  I have chosen to leave the files as is, and therefore many of them still
retain their copyright notices and ownership messages.


