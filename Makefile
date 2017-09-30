
PREFIX       = /usr/local
MANPREFIX    = $(PREFIX)/man
INSTALL_PATH = $(PREFIX)/include

MAKEFLAGS += --no-print-directory

all:
	@echo testing...OK

doc:
	@$(MAKE) -C docs

install:
	@echo install to $(INSTALL_PATH)...
	@cp -rf dpdk $(INSTALL_PATH)
	@echo install to $(INSTALL_PATH)... OK

uninstall:
	@echo uninstall to rm "$(INSTALL_PATH)/dpdk"...
	@rm -rf $(INSTALL_PATH)/dpdk
	@echo uninstall to rm "$(INSTALL_PATH)/dpdk"... OK

link-install:
	@echo create symboric link to $(INSTALL_PATH)...
	@ln -s `pwd`/dpdk $(INSTALL_PATH)/dpdk
	@echo create symboric link to $(INSTALL_PATH)... OK

