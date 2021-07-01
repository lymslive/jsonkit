#!/bin/bash
# run tast from this (build) directory

TAST_RUN_DIR=../tast
TAST_EXE=tast_jsonkit.exe

cd $TAST_RUN_DIR
./$TAST_EXE $@
