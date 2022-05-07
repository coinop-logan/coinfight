#!/bin/bash

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     machineName=linux;;
    Darwin*)    machineName=mac;;
    *)
        echo "Can't build for platform ${unameOut}"
        exit 1;;
esac

rm -f coinfight-client-${machineName}.zip

mkdir coinfight-client-${machineName}
cp install.sh coinfight-client-${machineName}/
cp ../../bin/Andale_Mono.ttf coinfight-client-${machineName}/
cp ../../bin/client coinfight-client-${machineName}/coinfight
cp ../../bin/coinfight_local coinfight-client-${machineName}/local_demo

zip -r coinfight-client-${machineName}.zip coinfight-client-${machineName}
rm -rf coinfight-client-${machineName}