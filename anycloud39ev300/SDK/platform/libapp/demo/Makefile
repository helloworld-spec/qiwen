dir_y := ini_demo
dir_y += vpss_demo
dir_y += dvr_record_demo
dir_y += dvr_replay_demo
dir_y += rtsp_demo

LOCAL_LDFLAGS := -L$(PLAT_LIB_DIR) -L$(MPI_LIB_DIR)                           
export LOCAL_LDFLAGS

all:
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i; \
		if [ $$? -ne 0 ];then exit 1; fi \
	done

install:
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i install; \
	done

clean:
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i clean; \
	done

.PHONY: all install clean

