#!/bin/sh

sed --in-place --regexp-extended "0,/'com.inaccel:coral-api:.*'/s/'com.inaccel:coral-api:.*'/'com.inaccel:coral-api:$@'/" \
	build.gradle
sed --in-place --regexp-extended "0,/<version>.*<\/version>/s/<version>.*<\/version>/<version>$@<\/version>/" \
	pom.xml

sed --in-place --regexp-extended "s/007396(.*)message=.*\&/007396\1message=$@\&/" \
	README.md
