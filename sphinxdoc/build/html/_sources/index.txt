.. _simpactcyan:

.. image:: _static/SimpactCyan_cropped.png
    :align: right
    :width: 10%

Simpact Cyan - |version|
========================

This document is the reference documentation for the Simpact Cyan
program, the C++ version of the `Simpact <http://www.simpact.org>`_
family of programs. 
The program is most easily controlled through either the Python or
R bindings, using the ``pysimpactcyan`` module and ``RSimpactCyan``
package respectively; the way to use them is described below.
The Python bindings are included in the Simpact Cyan package, the R 
bindings need to be installed separately from within R.

Apart from this document, there exists some additional documentation
as well:

 - Documentation of the program code itself can be found here: 
   `code documentation <documentation/index.html>`_.
 - A tutorial for using the R bindings: `Age-mixing tutorial Bruges 2015 <http://rpubs.com/wdelva/agemixing>`_
 - R versions of the iPython notebook examples: 
   `available on GitHub <https://github.com/dmhendrickx/R-code-examples-SIMPACT-documentation>`_
 
.. _programs:

The development source code can be found on `GitHub <https://github.com/j0r1/simpactcyan>`_.
The Simpact Cyan installers, as well as source code packages for the
program, can be found in the `'programs' <http://research.edm.uhasselt.be/jori/simpact/programs/>`_.
directory. If you're using MS-Windows, you'll need to install the 
`Visual Studio 2015 redistributable package <https://www.microsoft.com/en-us/download/details.aspx?id=48145>`_
as well (use the **x86** version) to be able to use the installed version. 

In case you're interested in :ref:`running simulations from R <startingfromR>`,
you'll need to have a working
Python version installed as well. For MS-Windows this is typically not
installed by default; when installing this it's best to use the default
directory (e.g. ``C:\Python27`` or ``C:\Python34``). 

MaxART documentation
--------------------

The configuration for running MaxART specific simulations is very similar
to the core Simpact Cyan features. The things that are
different are described in the :ref:`MaxART documentation <maxart>`.

Documentation contents
----------------------

.. toctree::
   :maxdepth: 1
   :numbered:

   simpact_introduction.rst
   simpact_conf_and_running.rst
   simpact_output.rst
   simpact_simulationdetails.rst
   maxart.rst
   simpact_references.rst


Indices and tables
------------------

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

