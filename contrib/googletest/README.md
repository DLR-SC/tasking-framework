### Google Test

Googletest inside the Tasking Framework is a copy of googletest release v1.10.0 on 3 Oct. 2019.
Documentation, scripts, and tests for googletest are removed from this folder to limit the content
to the files need to build the Tasking Framework tests. A scons file is added to support the build
with Scons. The cmake file is modified to support only the build of the Tasking Framework and its tests.
From several include statements the src folder is removed instead of twiddle at the include pathes when
the includes are at the same folder.

A non-modified version of googletest can be found under https://github.com/google/googletest. 