#!/bin/sh

sed --in-place --regexp-extended "0,/<version>.*<\/version>/s/<version>.*<\/version>/<version>$@<\/version>/" \
	pom.xml

sed --in-place --regexp-extended "s/007396(.*)message=.*\&/007396\1message=$@\&/" \
	README.md
