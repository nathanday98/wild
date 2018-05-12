@echo off
pushd build
7z a -tzip wild.zip @..\package.txt
popd
move build\wild.zip wild.zip