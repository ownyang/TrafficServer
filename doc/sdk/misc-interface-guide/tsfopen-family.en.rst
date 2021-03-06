The TSfopen Family
******************

.. Licensed to the Apache Software Foundation (ASF) under one
   or more contributor license agreements.  See the NOTICE file
  distributed with this work for additional information
  regarding copyright ownership.  The ASF licenses this file
  to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at
 
   http://www.apache.org/licenses/LICENSE-2.0
 
  Unless required by applicable law or agreed to in writing,
  software distributed under the License is distributed on an
  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
  KIND, either express or implied.  See the License for the
  specific language governing permissions and limitations
  under the License.

The ``fopen`` family of functions in C is normally used for reading
configuration files, since ``fgets`` is an easy way to parse files on a
line-by-line basis. The ``TSfopen`` family of functions aims at solving
the same problem of buffered IO and line at a time IO in a
platform-independent manner. The ``fopen`` family of C library functions
can only open a file if a file descriptor less than 256 is available.
Since Traffic Server often has more than 2000 file descriptors open at
once, however, the likelihood of an available file descriptor less than
256 very small. To solve this problem, the ``TSfopen`` family can open
files with descriptors greater than 256.

The ``TSfopen`` family of routines is not intended for high speed IO or
flexibility - they are blocking APIs (not asynchronous). For performance
reasons, you should not directly use these APIs on a Traffic Server
thread (when being called back on an HTTP hook); it is better to use a
separate thread for doing the blocking IO. The ``TSfopen`` family is
intended for reading and writing configuration information when
corresponding usage of the ``fopen`` family of functions is
inappropriate due to file descriptor and portability limitations. The
``TSfopen`` family of functions consists of the following:

-  ```TSfclose`` <http://people.apache.org/~amc/ats/doc/html/InkAPI_8cc.html#a2efebe7583752668e6136de0b47bee4f>`__

-  ```TSfflush`` <http://people.apache.org/~amc/ats/doc/html/InkAPI_8cc.html#a3cb0cb348ed189a98577f84e0629ca9a>`__

-  ```TSfgets`` <http://people.apache.org/~amc/ats/doc/html/ts_8h.html#a6dcc724a432a287836352b31984e0de8>`__

-  ```TSfopen`` <http://people.apache.org/~amc/ats/doc/html/ts_8h.html#a53b0430d5b0c042bdb3d06689cf244f3>`__

-  ```TSfread`` <http://people.apache.org/~amc/ats/doc/html/ts_8h.html#a29f83c50b52e4fcabfe2b829de5742e2>`__

-  ```TSfwrite`` <http://people.apache.org/~amc/ats/doc/html/ts_8h.html#a596a5562db5180ea8818f7bb87336a15>`__


