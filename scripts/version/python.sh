#!/bin/sh

sed --in-place --regexp-extended "0,/__version__ = '.*'/s/__version__ = '.*'/__version__ = '$@'/" \
	src/main/python/inaccel/coral/__init__.py

sed --in-place --regexp-extended "s/3776ab(.*)message=.*\&/3776ab\1message=$@\&/" \
	README.md
