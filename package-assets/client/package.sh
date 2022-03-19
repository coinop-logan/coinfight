#!/bin/bash

rm -f coinfight-client.zip

mkdir coinfight-client
cp install.sh coinfight-client/
cp ../../bin/Andale_Mono.ttf coinfight-client/
cp ../../bin/client coinfight-client/coinfight

zip -r coinfight-client.zip coinfight-client
rm -rf coinfight-client