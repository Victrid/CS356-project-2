# This Makefile comes from project 1.

EXECS=$(wildcard *.exc)
CUR=$(shell pwd)
DEST_FOLDERS=$${DEST_FOLDER}

.PHONY: all $(EXECS) $(pathsubst %.exc, push_%,${EXECS}) $(patsubst %.exc, run_%,${EXECS}) clean clean_exec $(patsubst %.exc,clean_exec_%,${EXECS}) push
all: | $(EXECS)

$(EXECS): 
	+ndk-build -C $@
	
push: | $(patsubst %.exc, push_%, ${EXECS})

$(patsubst %.exc, push_%, ${EXECS}): push_%: %.exc ${CUR}/../avd.lock
	adb shell "mkdir -p ${DEST_FOLDERS}"
	adb push ${patsubst push_%, %.exc, $@}/libs/armeabi/* ${DEST_FOLDERS}

$(patsubst %.exc, run_%, ${EXECS}): run_%: push_%
	_exe_file="$(notdir $(wildcard ${patsubst run_%,%.exc,$@}/libs/armeabi/*))"; \
		  adb shell "${DEST_FOLDERS}/$${_exe_file}"

clean_exec: | $(patsubst %.exc,clean_exec_%,${EXECS})

$(patsubst %.exc,clean_exec_%,${EXECS}):
	rm -rf ${patsubst clean_exec_%,%.exc,$@}/libs
	rm -rf ${patsubst clean_exec_%,%.exc,$@}/obj
		  
clean: | clean_exec
