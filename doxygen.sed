# doxygen input filter (the source files are not changed)
# the C only definitions are documented together with the C++ classes
s/^#ifndef __cplusplus\b.*$/#if 1/
# the object id types (arrays) are not documented
/^typedef [^;]* \w\+_id *\[\];/d
# the time units are documented independently of the configuration
s/^#if (OS_FREQUENCY).*$/#if 1/
