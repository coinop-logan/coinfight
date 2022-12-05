#!/bin/bash

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     machineName=linux;;
    Darwin*)    machineName=mac-x86;;
    *)
        echo "Can't build for platform ${unameOut}"
        exit 1;;
esac

rm -f coinfight-${machineName}.zip

mkdir coinfight-${machineName}
cp install.sh coinfight-${machineName}/
cp ../../bin/* coinfight-${machineName}/

zip -r coinfight-${machineName}.zip coinfight-${machineName}
rm -rf coinfight-${machineName}
