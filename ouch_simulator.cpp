#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1

#include <vector>
#include <functional>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

#include "ouch_simulator.h"

const char* ouch_simulator_version();

using namespace OUCHSim;
using namespace elf;
using namespace std;
using namespace boost::asio;

OUCHConnection::OUCHConnection(OUCHSimulator* sim, IOServiceRP iosvc) {
  _ouch_sim = sim;
  _logger = sim->get_logger();
  _socket = new ip::tcp::socket(*iosvc);
}

void
OUCHConnection::start() {
  boost::asio::ip::tcp::no_delay option(true);
  _socket->set_option(option);

  const ip::tcp::endpoint& remote = _socket->remote_endpoint();
  _peer = remote.address().to_string() + ":" + boost::lexical_cast<string>(remote.port());
  _name = _peer;

  LOG_INFO(_logger, "{}: new connection fd={} peer={}", _name, _socket->native_handle(), _peer);

  _recv_buffer.init(128*1024);
  _socket->async_read_some(buffer(_recv_buffer.write_head(), _recv_buffer.write_avail()),
                           std::bind(&OUCHConnection::handle_read, this,
                                     std::placeholders::_1,
                                     std::placeholders::_2));
}

void
OUCHConnection::shutdown() {
  try {
    boost::system::error_code ec;
    _socket->shutdown(socket_base::shutdown_both, ec);
    delete _socket;
    _socket = nullptr;
  } catch(boost::system::system_error& e) {
    ;
  }
}

void
OUCHConnection::send_raw(const char* buf, size_t len) {
  if(_ouch_sim->trace_messages()) {
    LOG_INFO(_logger, "{}: sending message size={}", _name, len);
  }

  _socket->send(boost::asio::buffer(buf, len));
}

void
OUCHConnection::handle_read(const boost::system::error_code& ec, size_t bytes_transferred) {
  if(ec) {
    LOG_INFO(_logger, "{}: fd={} disconnected", _name, _socket->native_handle());
    shutdown();
    return;
  }

  _recv_buffer.mark_written(bytes_transferred);

  if(_ouch_sim->trace_messages()) {
    LOG_INFO(_logger, "{}: received message xfer={} read_avail={}", _name,
             (int)bytes_transferred, (int)_recv_buffer.read_avail());
  }

  consume_buffer(_recv_buffer);

  if(!_recv_buffer.prepare_write(2048)) {
    LOG_ERROR(_logger, "{}: recv prepare_buffer failed", _name);
    _recv_buffer.clear();
    return;
  }

  _socket->async_read_some(buffer(_recv_buffer.write_head(), _recv_buffer.write_avail()),
                           std::bind(&OUCHConnection::handle_read, this,
                                     std::placeholders::_1,
                                     std::placeholders::_2));
}

void
OUCHConnection::consume_buffer(RWBuffer& buffer) {
  while(true) {
    size_t read_avail = buffer.read_avail();
    if(read_avail < 1)
      break;

    char msgtype = *buffer.read_head();
    switch(msgtype) {
    case OUCH42::MessageType::NewOrder:
      {
        if(read_avail < sizeof(OUCH42::NewOrder))
          return;

        auto new_order = buffer.try_consume_struct<const OUCH42::NewOrder>();
        oid_t oid = _ouch_sim->register_new_order(new_order);
        if(oid==INVALID_OID)
          send_reject('T', new_order->token);
        else
          send_ack(new_order, oid);

        break;
      }

    case OUCH42::MessageType::CancelOrder:
      {
        if(read_avail < sizeof(OUCH42::CancelOrder))
          return;

        auto cxl = buffer.try_consume_struct<const OUCH42::CancelOrder>();
        oid_t oid = _ouch_sim->find_order(cxl->token);
        if(oid==INVALID_OID)
          break;

        _ouch_sim->cancel_order(oid);
        send_canceled(cxl, oid);
        break;
      }

    default:
      buffer.mark_read(read_avail);
      LOG_ERROR(_logger, "{}: discarding input len={} msgtype={}", _name, read_avail, msgtype);
      break;
    }
  }
}

void
OUCHConnection::send_ack(const OUCH42::NewOrder* new_order, oid_t oid) {
  OUCH42::OrderAck ack;
  memcpy(ack.token, new_order->token, sizeof(ack.token));
  ack.side = new_order->side;
  ack.qty = new_order->qty;
  memcpy(ack.symbol, new_order->symbol, sizeof(ack.symbol));
  ack.px = new_order->px;
  ack.tif = new_order->tif;
  memcpy(ack.mpid, new_order->mpid, sizeof(ack.mpid));
  ack.display = new_order->display;
  ack.oid = oid;
  ack.capacity = new_order->capacity;
  ack.iso = new_order->iso;
  ack.minqty = new_order->minqty;
  ack.cross_type = new_order->cross_type;
  ack.state = 'L';

  send_raw(reinterpret_cast<const char*>(&ack), sizeof(ack));
}

void
OUCHConnection::send_reject(const char reason, const char* token) {
  OUCH42::OrderRejected rej;
  memcpy(rej.token, token, sizeof(rej.token));
  rej.reason = reason;
  send_raw(reinterpret_cast<char*>(&rej), sizeof(rej));
}

void
OUCHConnection::send_canceled(const OUCH42::CancelOrder* user_cxl, oid_t oid) {
  OUCH42::OrderCanceled cxl;
  memcpy(cxl.token, user_cxl->token, sizeof(cxl.token));
  cxl.qty = user_cxl->qty;
  cxl.reason = 'U';
  send_raw(reinterpret_cast<char*>(&cxl), sizeof(cxl));
}

void
OUCHSimulator::init(int port, bool trace_messages) {
  _name = "ouch_sim";

  // logger
  quill::Handler* handler = quill::stdout_handler("sh");
  handler->set_pattern("%(ascii_time) %(level_name) %(logger_name) %(message)", "%D %H:%M:%S.%Qus", quill::Timezone::LocalTime);
  quill::config::set_backend_thread_sleep_duration(std::chrono::milliseconds(10));
  _logger = quill::create_logger(_name.c_str(), handler);
  _logger->set_log_level(quill::LogLevel::TraceL3);
  quill::start();

  _port = port;
  _trace_messages = trace_messages;

  LOG_INFO(_logger, "starting");
  LOG_INFO(_logger, "version={} port={} trace_messages={}", ouch_simulator_version(), _port, _trace_messages);

  _running = true;
  _ioservice = std::make_shared<IOService>();
  init_listener();
}

void
OUCHSimulator::init_listener() {
  try {
    _acceptor = new ip::tcp::acceptor(*_ioservice, ip::tcp::endpoint(ip::tcp::v4(), _port));
  } catch(boost::system::system_error& e) {
    LOG_WARNING(_logger, "accept: {}", e.what());
    return;
  }

  arm_acceptor();
}

void
OUCHSimulator::arm_acceptor() {
  OUCHConnection* conn = new OUCHConnection(this, _ioservice);
  _acceptor->async_accept(*(conn->_socket), std::bind(&OUCHSimulator::handle_accept, this, conn, std::placeholders::_1));
}

void
OUCHSimulator::handle_accept(OUCHConnection* conn, const boost::system::error_code& error) {
  if(error) {
    LOG_WARNING(_logger, "{}: handle_accept: error={} {}", _name, error.value(), error.message());
    conn->shutdown();
    return;
  }

  conn->start();
  _conn_set.insert(conn);
  arm_acceptor();
}

void
OUCHSimulator::stop_listener() {
  delete _acceptor;
  _acceptor = nullptr;
}

void
OUCHSimulator::run() {
  while(_running)
    _ioservice->run_one();
}

void
OUCHSimulator::shutdown() {
  _running = false;
  stop_listener();
  _ioservice->stop();
}

oid_t
OUCHSimulator::register_new_order(const elf::OUCH42::NewOrder* new_order) {
  OUCHOrder order;
  oid_t oid = _orders.size();

  order.oid = oid;
  order.state = OrderState::NEW;
  order.side = new_order->side;
  _orders.push_back(order);
  return oid;
}

oid_t
OUCHSimulator::find_order(const char* token) const {
  return INVALID_OID;
}

void
OUCHSimulator::cancel_order(oid_t oid) {
}
