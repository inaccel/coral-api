#!/bin/sh

sed --in-place --regexp-extended "0,/Version: .*/s/Version: .*/Version: $@/" \
	coral-api.pc

sed --in-place --regexp-extended "s/a8b9cc(.*)message=.*\&/a8b9cc\1message=$@\&/" \
	README.md
