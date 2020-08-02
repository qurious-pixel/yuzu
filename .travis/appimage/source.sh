#!/bin/bash -ex

SOURCEURL=`base64 -d <<<"aHR0cHM6Ly93d3c3OS56aXBweXNoYXJlLmNvbS9kL2tQSVJCbGx4LzI1MTY1L21zdmMtc291cmNlLTIwMjAwNzMwLWVkZDg3Y2Y0MC50YXIueHo="`

mkdir -p /tmp/source
cd /tmp/source
curl -sLO $SOURCEURL
tar -xf *.xf
mv yuzu-*/ yuzu/
