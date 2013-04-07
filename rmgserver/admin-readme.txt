RMG Server Administration README

Quick overview of the `rmgadmin'  script
April 12, 1996

`rmgadmin' allows dynamic reconfiguration of running rmgserver
processes. It does this by creating a short rsl script based
on command line options, which are (excerpt from `rmgadmin'):

# Options:
#
#	-shutdown     seconds till all servers shut down
#
#	-logsystems   subsystems to log (a-g, see documentation)
#
#	-loglevel     1-6; 1 = Fatal, 2= Alert, ..., 6 = Debug
#
#	-maxusers     maximum users. Set to 0 to disallow new logins.
#
#	-create       specify output filename (default: reconfig.rsl)
#                 The filename that the server looks for is
#                 specified by the variable ReconfigFile in
#                 the _a*.rsl server config file.
#
#   -p            print reconfig script only (do not reconfigure servers)
#                 The new script is still saved.

Examples: ('%' is the Unix prompt)

Shutdown the server in 10 minutes:

	% rmgadmin -shutdown 600

Change to debug logging level:

	% rmgadmin -loglevel 6

Combination:

	% rmgadmin -loglevel 3 -logsystems cg -maxusers 12


------

Technical note:

After creating an rsl reconfig script, it then executes the
shell script `rmgreconfig'.  This is done so that 
an administrator can alter or write their own reconfig script to do
things other than the rmgadmin script provides, for example by
using the `-p' option with rmgadmin to create a script, modifying it,
and then executing `rmgreconfig'.

