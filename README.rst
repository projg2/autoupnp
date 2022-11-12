========
autoupnp
========

(c) 2010-2022 Michał Górny
Released under the terms of the 3-clause BSD license


Motivation
----------
The support for UPnP/IGD standards (and NAT-PMP) in open-source
applications isn't widely deployed. Although there are at least three
different open-source libraries providing quite a simple interfaces
to handle that, I can hardly mention an application using one of them.

I thought about starting to write patches to different applications
adding the support for UPnP. But that means adding a new dependency,
which many software authors don't like. And I still have to choose
between available libraries, meaning that the end-user could end up with
all of them, being used by a single application each.

That's why I've decided to write AutoUPnP -- a tool which automatically
adds and removes UPnP NAT entries as the ports are respectively being
open and closed.



How to use
----------
AutoUPnP 0.4 is supposed to be used via ld.so's ``LD_PRELOAD``
mechanism. In order to have the library preloaded, you'll probably want
to use the ``autoupnp`` wrapper script. Call:

    $ autoupnp --help

to take a short guide of the script.


Bugs
----
In order to report a bug against AutoUPnP, please use the `GitHub issue
tracker`_.

.. _GitHub issue tracker: http://github.com/projg2/autoupnp/issues
