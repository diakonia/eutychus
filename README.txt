EUTYCHUS - A free Church service recording plug-in for
Audacity(R) Digital Audio Editor

EUTYCHUS source is copyright (c) 2013 Stephen Parry,
some elements copyright (c) 1999-2013 by Audaicty Team.

"Audacity" is a registered trademark of Dominic Mazzoni.

1. Licensing

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version. The program source code is also freely
available as per Section 4 of this README.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License
along with this program (in a file called LICENSE.txt); if not, go
to http://www.gnu.org/licenses/old-licenses/gpl-2.0.html or write to

  Free Software Foundation, Inc.
  59 Temple Place - Suite 330
  Boston, MA 02111-1307 USA

2. Building Eutychus

In order to build Eutychus you will need to first download and build the
main Audacity source code. See http://audacity.sourceforge.net/ for detailed
instructions.

Once you have the cloned the Audacity repo you should then clone the Eutychus
repo to <AUDACITY_SOURCE>\lib-src\mod-eutychus-service-recorder. The Visual C++
project file should be added to the main solution and the whole solution built.

3. Installation

A)
Make sure the version of Audacity you have installed matches Eutychus. All
Eutychus releases have a four digit version number. The first three must match
the Audacity version.

B)
Copy:
%AUDACITY_DEV_HOME%\win\Unicode Release\Modules\mod-eutychus-service-recorder.dll
or the releases are
To:
%AUDACITY_HOME%\Modules\

C)
Install wp-audio-uploader (https://github.com/sgparry/wp-audio-uploader) and its
dependencies as per https://github.com/sgparry/wp-audio-uploader/README.txt

D)
Install support files as directed by:
%AUDACITY_DEV_HOME%\win\Unicode Release\Modules\SupportFiles\README.txt

3. Known Issues

Configuration is by editing the audacity.cfg. Future release will include
a configuration screen.

The church logo is currently hardcoded (Sorry!).

Eutychus needs to be compiled to a specfic version of Audacity. Future releases 
will always break compatbility. Furthermore, as of writing (Audacity 2.0.5)
Eutychus uses some very ugly hacks to deliver its functionality. These hacks
include copied and edited source code that will need refreshes for each release
and one bug workaround that required hacking a private class member. The
displacement used will require review on each release until the bug is fixed.
However, the benefit of these hacks is that a custom build of Audacity is NOT
required on the target machine.

