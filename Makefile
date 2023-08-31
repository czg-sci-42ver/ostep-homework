SUBDIRS := $(wildcard */.)
all: $(SUBDIRS)
# clean: $(SUBDIRS)

# $(foreach dir,$(SUBDIRS),$(ifneq ("$(wildcard $(dir)/Makefile)","") $(MAKE) -C $@ else endif))
# ifneq ($(wildcard $@/Makefile),)
# 	$(MAKE) -C $@
# else
# endif

######
# $(shell) can't recognize the `$(MAKE)`
# 1. direct using if similar to bash in make -> https://stackoverflow.com/a/58602879/21294350
# 2. foreach -> https://stackoverflow.com/a/978304/21294350
# 3. check all subdirs -> https://stackoverflow.com/a/17845120/21294350
######

$(SUBDIRS):
	if [ -e $@/Makefile ];then $(MAKE) -C $@;fi;

# $(foreach dir,$(SUBDIRS),$(shell if [ -e $(dir)/Makefile ];then $(MAKE) -C $@ clean;fi))
# $(foreach dir,$(SUBDIRS),@echo $(dir))
clean:
	$(foreach dir,$(SUBDIRS),if [ -e $(dir)/Makefile ];then $(MAKE) -C $(dir) clean;fi;)

.PHONY: clean $(SUBDIRS)
