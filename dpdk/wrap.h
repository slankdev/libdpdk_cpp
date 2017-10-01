
#pragma once

#include <dpdk/hdr.h>
#include <dpdk/struct.h>

#include <string>
#include <exception>
#include <sstream>
#include <ostream>

#define UNUSED(x) (void)(x)

namespace dpdk {

inline void hexdump(FILE* fp, const void *buffer, size_t bufferlen)
{
  const uint8_t *data = reinterpret_cast<const uint8_t*>(buffer);
  size_t row = 0;
  while (bufferlen > 0) {
    fprintf(fp, "%04zx:   ", row);

    size_t n;
    if (bufferlen < 16) n = bufferlen;
    else                n = 16;

    for (size_t i = 0; i < n; i++) {
      if (i == 8) fprintf(fp, " ");
      fprintf(fp, " %02x", data[i]);
    }
    for (size_t i = n; i < 16; i++) {
      fprintf(fp, "   ");
    }
    fprintf(fp, "   ");
    for (size_t i = 0; i < n; i++) {
      if (i == 8) fprintf(fp, "  ");
      uint8_t c = data[i];
      if (!(0x20 <= c && c <= 0x7e)) c = '.';
      fprintf(fp, "%c", c);
    }
    fprintf(fp, "\n");
    bufferlen -= n;
    data += n;
    row  += n;
  }
}

inline std::string format(const char* fmt, ...)
{
  char str[1000];
  va_list args;
  va_start(args, fmt);
  vsprintf(str, fmt, args);
  va_end(args);
  return str;
}

class exception : public std::exception {
 private:
  std::string str;
 public:
  explicit exception(const char* s="") noexcept {
    str = s;
  }

  template<class T>
    exception& operator<<(const T& t) noexcept {
      std::ostringstream os;
      os << " " << t ;
      str += os.str();
      return *this;
    }
  const char* what() const noexcept {
    return str.c_str();
  }
};

inline void rte_eal_remote_launch(lcore_function_t f, void* arg, size_t lcore_id)
{
  size_t n_lcores = rte_lcore_count();
  if (n_lcores <= lcore_id) {
    throw exception("rte_eal_remote_launch: too huge lcore_id?");
  }

  int ret = ::rte_eal_remote_launch(f, arg, lcore_id);
  if (ret != 0) {
    std::string err = format("rte_eal_remote_launch(%p,%p,%zd): ",f,arg,lcore_id);
    switch (ret) {
      case -EBUSY:
        err += "the remote lcore is not in a WAIT state.";
        throw exception(err.c_str());
        break;
      default:
        err += "unknow error";
        throw exception(err.c_str());
        break;
    }
  }
}

inline void rte_acl_build(struct rte_acl_ctx* acx, struct rte_acl_config* cfg)
{
  int ret = ::rte_acl_build(acx, cfg);
  if (ret < 0) {
    std::string err = "rte_acl_build: ";
    switch (ret) {
      case -ENOMEM:
        err += format("Couldn't allocate enough memory (%d)", ret);
        break;
      case -EINVAL:
        err += format("The parameters are invalid (%d)", ret);
        break;
      case -ERANGE:
        cfg->max_size = 0;
        dpdk::rte_acl_build(acx, cfg);
        return ;
        break;
      default:
        err += format("Other error (%d)", ret);
        break;
    }
    throw dpdk::exception(err.c_str());
  }
  return;
}


inline void rte_acl_classify(struct rte_acl_ctx* acx,
                      const uint8_t**	    data,
                      uint32_t*           results,
                      uint32_t            num,
                      uint32_t            categories)
{
  int ret = ::rte_acl_classify(acx, data, results, num , categories);
  if (ret < 0) {
    std::string err = "rte_acl_classify: ";
    switch (ret) {
      case -EINVAL:
        err += "Incorrect arguments.";
        break;
      default:
        err += format("Other error (%d)", ret);
        break;
    }
    throw dpdk::exception(err.c_str());
  }
  return;
}

inline void hexdump_mbuf(FILE* fp, rte_mbuf* msg)
{
  hexdump(fp,
      rte_pktmbuf_mtod(msg, void*),
      rte_pktmbuf_pkt_len(msg));
}


inline void rte_pktmbuf_free_bulk(struct rte_mbuf* m_list[], size_t npkts)
{
  while (npkts--)
    rte_pktmbuf_free(*m_list++);
}


inline struct rte_mbuf* pktmbuf_clone(struct rte_mbuf* md, struct rte_mempool* mp)
{
  struct rte_mbuf* ret = ::rte_pktmbuf_clone(md, mp);
  if (!ret) {
    throw dpdk::exception("rte_pktmbuf_clone");
  }
  return ret;
}

inline struct rte_ring* ring_create(const char* name, size_t count, int socket_id, unsigned flags)
{
  struct rte_ring* ring = ::rte_ring_create(name, count, socket_id, flags);
  if (!ring) {
    std::string e = "rte_ring_create: ";
    switch (rte_errno) {
      case E_RTE_NO_CONFIG: e += "E_RTE_NO_CONFIG"; break;
      case E_RTE_SECONDARY: e += "E_RTE_SECONDARY"; break;
      case EINVAL         : e += "EINVAL         "; break;
      case ENOSPC         : e += "ENOSPC         "; break;
      case EEXIST         : e += "EEXIST         "; break;
      case ENOMEM         : e += "ENOMEM         "; break;
      default: e += "unknown error"; break;
    }
    throw dpdk::exception(e.c_str());
  }
}

static inline void errhandle(const char* str)
{
  std::string e = str;
  e += ": ";
  e += rte_strerror(rte_errno);
  throw dpdk::exception(e.c_str());
}



inline void dpdk_boot(int argc, char** argv)
{
  int ret = rte_eal_init(argc, argv);
  if (ret < 0) {
    std::string e = "rte_eal_init: ";
    e += rte_strerror(rte_errno);
    e += " (";
    switch (rte_errno) {
      case EAGAIN:
        e+="indicates either a bus or system resource was not available, setup may be attempted again.";
        break;
      case EALREADY:
        e+="indicates that the rte_eal_init function has already been called, and cannot be called again.";
        break;
      case EFAULT:
        e+="indicates the tailq configuration name was not found in memory configuration.";
        break;
      case EINVAL:
        e+="indicates invalid parameters were passed as argv/argc.";
        break;
      case ENOMEM:
        e+="indicates failure likely caused by an out-of-memory condition.";
        break;
      case ENODEV:
        e+="indicates memory setup issues.";
        break;
      case ENOTSUP:
        e+="indicates that the EAL cannot initialize on this system.";
        break;
      case EPROTO:
        e+="indicates that the PCI bus is either not present, or is not readable by the eal.";
        break;
      case ENOEXEC:
        e+="indicates that a service core failed to launch successfully.";
        break;
      case EACCES:
        e+="indicates a permissions issue";
        break;
      default:
        e += "UNKNOWN ";
        break;
    };
    e += ")";
    throw dpdk::exception(e.c_str());
  }
}


inline void set_mbuf_raw(rte_mbuf* mbuf, const void* data, size_t len)
{
  mbuf->data_len = len;
  mbuf->pkt_len  = len;
  uint8_t* p = reinterpret_cast<uint8_t*>(rte_pktmbuf_mtod(mbuf, uint8_t*));
  memcpy(p, data, len);
}

inline rte_mempool* mp_alloc(const char* name)
{
  constexpr size_t NUM_MBUFS       = 8191;
  constexpr size_t MBUF_CACHE_SIZE = 0;
  size_t nb_ports = rte_eth_dev_count();

	struct rte_mempool* mp = rte_pktmbuf_pool_create(
      name,
      NUM_MBUFS * (nb_ports!=0?nb_ports:1),
      MBUF_CACHE_SIZE,
      0,
      RTE_MBUF_DEFAULT_BUF_SIZE,
      rte_socket_id());
  if (!mp) {
    std::string e = "rte_pktmbuf_pool_create: ";
    e += rte_strerror(rte_errno);
    throw dpdk::exception(e.c_str());
  }
  return mp;
}

inline rte_ring* ring_alloc(const char* name, size_t sizeofring)
{
  /*
   * 2nd argument, sizeofringm must be power of 2
   * Example
   *   good: 64, 128, 1024
   *   bad : 1000
   */
  int socket_id  = rte_socket_id();
  uint32_t flags = 0;
  rte_ring* r = dpdk::ring_create(name, sizeofring, socket_id, flags);
  return r;
}


inline void init_portconf(struct rte_eth_conf* conf)
{
  memset(conf, 0, sizeof(rte_eth_conf));

  /*********************************************\
   * Rx Mode
  \*********************************************/
  auto& rxmode = conf->rxmode;
  rxmode.mq_mode        = ETH_MQ_RX_NONE;
  rxmode.max_rx_pkt_len = ETHER_MAX_LEN;
  rxmode.split_hdr_size = 0;

  /* bit fields */
  rxmode.header_split   = 0;
  rxmode.hw_ip_checksum = 0;
  rxmode.hw_vlan_filter = 0;
  rxmode.hw_vlan_strip  = 0;
  rxmode.hw_vlan_extend = 0;
  rxmode.jumbo_frame    = 0;
  rxmode.hw_strip_crc   = 0;
  rxmode.enable_scatter = 0;
  rxmode.enable_lro     = 0;


  /*********************************************\
   * Tx Mode
  \*********************************************/
  auto& txmode = conf->txmode;
  txmode.mq_mode                 = ETH_MQ_TX_NONE;
  txmode.pvid                    = 0;  /* only I40E? */
  txmode.hw_vlan_reject_tagged   = 0;  /* only I40E? */
  txmode.hw_vlan_reject_untagged = 0;  /* only I40E? */
  txmode.hw_vlan_insert_pvid     = 0;  /* only I40E? */


  /*********************************************\
   * Rx Adv Conf
  \*********************************************/
  auto& rx_adv_conf = conf->rx_adv_conf;
  UNUSED(rx_adv_conf);


  /*********************************************\
   * Tx Adv Conf
  \*********************************************/
  auto& tx_adv_conf = conf->tx_adv_conf;
  UNUSED(tx_adv_conf);


  /*********************************************\
   * FDIR conf
  \*********************************************/
  auto& fdir_conf = conf->fdir_conf;
  UNUSED(fdir_conf);
}



inline void port_configure(uint8_t port, size_t nb_rxq, size_t nb_txq,
                  const struct rte_eth_conf* port_conf, struct rte_mempool* mp)
{
  int ret;
  constexpr size_t RX_RING_SIZE = 128;
  constexpr size_t TX_RING_SIZE = 512;

  ret = rte_eth_dev_configure(port, nb_rxq, nb_txq, port_conf);
  if (ret < 0) errhandle("rte_eth_dev_configure");

  for (size_t q=0; q<nb_rxq; q++) {
    ret = rte_eth_rx_queue_setup(port, q, RX_RING_SIZE,
        rte_eth_dev_socket_id(port), NULL, mp);
    if (ret < 0) {
      std::string e = "rte_eth_rx_queue_setup: ";
      e += rte_strerror(rte_errno);
      switch (ret) {
        case -EINVAL:
          e += "(-EINVAL)";
          break;
        case -ENOMEM:
          e += "(-ENOMEM)";
          break;
        default:
          e += "unknown error";
          throw dpdk::exception(e.c_str());
          break;
      }
    }
  }

  for (size_t q=0; q<nb_txq; q++) {
    ret = rte_eth_tx_queue_setup(port, q, TX_RING_SIZE,
        rte_eth_dev_socket_id(port), NULL);
    if (ret < 0) {
      std::string e = "rte_eth_tx_queue_setup: ";
      switch (ret) {
        case -ENOMEM:
          e += "(-ENOMEM)";
          break;
        default:
          e += "unknown error";
          throw dpdk::exception(e.c_str());
          break;
      }
    }
  }

  ret = rte_eth_dev_start(port);
  if (ret < 0) {
    throw dpdk::exception("rte_eth_dev_start");
  }

  /* this function always success because of HARDWARECALL */
  rte_eth_promiscuous_enable(port);
}


inline void safe_ring_enqueue(rte_ring* ring, void* data)
{
  int ret = rte_ring_enqueue(ring, data);
  if (ret < 0) throw dpdk::exception("rte_ring_enqueue: no space in ring");
}


inline void safe_ring_dequeue(rte_ring* ring, void** data)
{
  int ret = rte_ring_dequeue(ring, data);
  if (ret < 0) throw dpdk::exception("rte_ring_dequeue: no entry in ring");
}


inline void safe_ring_enqueue_bulk(rte_ring* ring, void* const* objs, size_t n)
{
  int ret = rte_ring_enqueue_bulk(ring, objs, n, nullptr);
  if (ret != n) throw dpdk::exception("rte_ring_enqueue_bulk: miss");
}


inline void safe_ring_dequeue_bulk(rte_ring* ring, void** objs, size_t n)
{
  int ret = rte_ring_dequeue_bulk(ring, objs, n, nullptr);
  if (ret < 0) throw dpdk::exception("rte_ring_dequeue_bulk: miss");
}

} /* namespace dpdk */


