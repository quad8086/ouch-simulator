#pragma once

#include <string>
#include <set>
#include <unordered_map>

#include <boost/asio.hpp>

#include <quill/Quill.h>

#include "boost_enum.h"
#include "rwbuffer.h"
#include "ouch_structs.h"

namespace OUCHSim {
  using namespace std;
  using namespace elf;

  using Logger = quill::Logger;

  typedef int64_t oid_t;
  static const oid_t INVALID_OID = -1;
  typedef boost::asio::io_service IOService;
  typedef std::shared_ptr<boost::asio::io_service> IOServiceRP;

  BOOST_ENUM(ConnectionState,
             (Initial)
             (Connected)
             (Shutdown)
             );

  class OUCHSimulator;

  struct OUCHConnection {
    OUCHConnection(OUCHSimulator* sim, IOServiceRP iosvc);
    void shutdown();
    void start();
    void handle_read(const boost::system::error_code& ec, size_t bytes_transferred);
    void consume_buffer(RWBuffer& buffer);
    void send_raw(const char* buf, size_t len);

    void send_ack(const elf::OUCH42::NewOrder* new_order, oid_t oid);
    void send_reject(const char reason, const char* token);
    void send_canceled(const elf::OUCH42::CancelOrder* user_cxl, oid_t oid);

    ConnectionState _state;
    boost::asio::ip::tcp::socket* _socket;
    OUCHSimulator* _ouch_sim;
    Logger* _logger;
    RWBuffer _recv_buffer;
    string _peer;
    string _name;
  };

  typedef std::set<OUCHConnection*> OUCHConnectionSet;

  BOOST_ENUM(OrderState,
             (INITIAL)
             (NEW)
             (OPEN)
             (CANCELED)
             (FILLED)
             (REJECTED)
             );

  struct OUCHOrder {
    OrderState state = OrderState::INITIAL;
    char token[14];
    char side;
    uint32_t qty = 0;
    uint32_t filled_qty = 0;
    char symbol[8];
    uint32_t px = 0;
    uint32_t tif = 0;
    char mpid[4];
    char display;
    uint64_t oid;
    char capacity;
    char iso;
    uint32_t minqty = 0;
    char cross_type;
  };

  class OUCHSimulator {
  public:
    OUCHSimulator() = default;
    void init(int port, bool trace_messages);
    void run();
    void shutdown();
    auto get_logger() { return _logger; }
    bool trace_messages() { return _trace_messages; }

    // om
    oid_t register_new_order(const elf::OUCH42::NewOrder* new_order);
    oid_t find_order(const char* token) const;
    void cancel_order(oid_t oid);

  private:
    void init_listener();
    void stop_listener();
    void handle_accept(OUCHConnection* conn, const boost::system::error_code& error);
    void arm_acceptor();

  private:
    bool _running = false;
    string _name;
    Logger* _logger = nullptr;
    IOServiceRP _ioservice;
    int _port = 0;
    bool _trace_messages = false;
    boost::asio::ip::tcp::acceptor* _acceptor = nullptr;
    OUCHConnectionSet _conn_set;
    vector<OUCHOrder> _orders;
  };
}
