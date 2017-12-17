

ifeq ($(RTE_SDK),)
$(error "Please define RTE_SDK environment variable")
endif
RTE_TARGET ?= x86_64-native-linuxapp-gcc
DPDK_PATH   = $(RTE_SDK)/$(RTE_TARGET)

DPDK_CXXFLAGS = -I$(DPDK_PATH)/include -m64 -mssse3 -std=c++11
DPDK_LDFLAGS = \
	-Wl,--no-as-needed \
	-Wl,-export-dynamic \
	-L$(DPDK_PATH)/lib \
	-lpthread -ldl -lrt -lm -lpcap \
	-Wl,--whole-archive -Wl,--start-group \
	-lrte_jobstats -lrte_pipeline -lrte_pmd_ixgbe \
	-lrte_acl -lrte_kni -lrte_pmd_kni -lrte_pmd_vhost \
	-lrte_bitratestats -lrte_kvargs -lrte_pmd_virtio \
	-lrte_cfgfile -lrte_latencystats -lrte_pmd_vmxnet3_uio \
	-lrte_cmdline -lrte_lpm -lrte_pmd_null -lrte_port \
	-lrte_cryptodev -lrte_mbuf -lrte_pmd_null_crypto -lrte_power \
	-lrte_distributor -lrte_mempool -lrte_pmd_crypto_scheduler -lrte_pmd_octeontx_ssovf -lrte_reorder \
	-lrte_eal -lrte_mempool_ring  -lrte_ring \
	-lrte_efd -lrte_mempool_stack -lrte_pmd_e1000 -lrte_sched \
	-lrte_ethdev -lrte_meter -lrte_table \
	-lrte_eventdev -lrte_metrics -lrte_pmd_skeleton_event -lrte_timer \
	-lrte_hash -lrte_net -lrte_pmd_sw_event -lrte_vhost \
	-lrte_ip_frag -lrte_pdump -lrte_pmd_i40e -lrte_pmd_tap \
	-Wl,--end-group -Wl,--no-whole-archive -lnuma




