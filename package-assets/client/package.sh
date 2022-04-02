#!/bin/bash

rm -f coinfight-client-linux.zip

mkdir coinfight-client-linux
cp install.sh coinfight-client-linux/
cp ../../bin/Andale_Mono.ttf coinfight-client-linux/
cp ../../bin/client coinfight-client-linux/coinfight
cp ../../bin/coinfight_local coinfight-client-linux/local_demo

zip -r coinfight-client-linux.zip coinfight-client-linux
rm -rf coinfight-client-linux