@echo off

del eradsrc.zip
zip -r eradsrc.zip . -i *.c -i *.h -i *.asm -i *.inl -i *akefile* -i *.bat -i *.lib -i *.ovl -x data/* -x releases/*
