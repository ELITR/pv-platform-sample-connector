# pv-platform-sample-connector
Sample code illustrating how to connect MT and ASR engines to Pervoice intergration platform.



SDK for PerVoice' service architecture for speech-to-speech translation
----------------------------------------------------------------------------------

This directory contains an SDK for connecting engines such as for speech
recognition or machine translation to PerVoice' service architecture
in the Cloud (MCloud) as well as for implementing clients that can use the
service architecture in order to perform speech-to-speech translation on some
audio data.


Content:
--------
doc             Additional documentation that describes the API and its usage
include         Header files necessary for developing own Backends and Clients
Linux           Example Backend and Client applications with source code and
                pre-compiled binaries for 64-bit Linux


Version history:
----------------
v0.5    First initial release
v0.6	several improvements and bugfixes partly based on comments from PerVoice


Chiara Canton
chiara.canton@pervoice.it

(c) 2013 PerVoice S.p.A.


===================================================================================================

Create audiorecorder and ebclient:

win32 (in visualstudio 2013 shell 32-bit):
in windows folder
nmake  -f makefile.win --> creates exes in exe32
nmake  -f makefile.win clean32 --> clean exe32 and .obj files
nmake  -f makefile.win all-binary--> creates exes (included client_binary and worker_binary) in exe32

win64 (in visualstudio 2013 shell 64-bit):
in windows folder
nmake  -f makefile.win all64 --> creates exes in exe64
nmake  -f makefile.win clean64 --> clean exe64 and .obj files
nmake  -f makefile.win all-binary64--> creates exes (included client_binary and worker_binary) in exe64

nmake  -f makefile.win clean --> clean exe32 and exe64 and all .obj files

 -- created audiorecorder.exe and ebclient.exe in exe32 and exe64 (created also client_binary.exe and worker_binary.exe if all-binary*) --

unix32:
in linux folder
TARGET_PLATFORM=i686 make -f makefile.linux
export LD_LIBRARY_PATH=./lib32

make -f makefile.linux clean

unix64:
in linux folder
TARGET_PLATFORM=x86_64 make -f makefile.linux
export LD_LIBRARY_PATH=./lib64

make -f makefile.linux clean

-- created audiorecorder and ebclient in linux directory --
