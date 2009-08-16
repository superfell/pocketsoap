rem @echo off 
@echo fixing links in %1
copy %1 sed.tmp
sed -e "s/interface_ISOAPTransport.html/ms-its:psdocs.chm::\/interface_ISOAPTransport.html/" sed.tmp > %1
del sed.tmp
