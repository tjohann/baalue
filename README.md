baalue - bananapi cluster node controller user interface
========================================================

This is a small command line tool to trigger simple tasks on a remote and/or local node.


Requirements
------------

To build and run baalue you need libbaalue (https://github.com/tjohann/libbaalue.git), no other libs/tools needed.


Examples (remote)
-----------------

To reboot remote node baalue-01:

	baalue -s baalue-01 -c reboot


To halt remote node baalue-01:

	baalue -s baalue-01 -c halt


To check if baalued on remote node baalue-01 is alive

	baalue -s baalue-01 -c ping


Examples (local)
-----------------

To reboot local node:

	baalue -l -c reboot


To halt local node:

	baalue -l -c halt


To check if baalued on local node is alive

	baalue -l -c ping
