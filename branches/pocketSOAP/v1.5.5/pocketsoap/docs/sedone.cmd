@echo off 
@echo fixing links in %1
copy %1 sed.tmp
sed -e "s/ms-its:psdocs.chm::/..\/psdocs/" sed.tmp > %1
copy %1 sed.tmp
sed -e "s/ms-its:attachments.chm::/..\/attachments/" sed.tmp > %1
copy %1 sed.tmp
sed -e "s/ms-its:proxydocs.chm::/..\/proxydocs/" sed.tmp > %1
copy %1 sed.tmp
sed -e "s/ms-its:master.chm::/..\/master/" sed.tmp > %1
del sed.tmp

