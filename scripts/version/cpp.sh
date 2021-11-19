#!/bin/sh

sed --in-place --regexp-extended "0,/version: .*/s/version: .*/version: $@/" \
	nfpm.yaml

sed --in-place --regexp-extended "s/00599c(.*)message=.*\&/00599c\1message=$@\&/" \
	README.md
