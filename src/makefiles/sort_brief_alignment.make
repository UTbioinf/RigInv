ifndef INC_FNAME
  $(error INC_FNAME is not set)
endif

ifndef ALN_SORTBIN
  $(error ALN_SORTBIN is not set)
endif

include $(INC_FNAME)

ifeq($(ALN_RUN),0)
  exit 0
endif

all: $(ALN_SOURCES)

$(ALN_SOURCES):
	$(ALN_SORTBIN) $(ALN_ROOTDIR)/$@.txt $(ALN_ROOTDIR)/$@.sorted.txt
	mv $@.sorted.txt $@.txt


