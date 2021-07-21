# **********  build.sh  ************
#!/bin/bash
aclocal
autoconf
autoheader
automake --add-missing
./configure CXXFLAGS= CFLAGS=
