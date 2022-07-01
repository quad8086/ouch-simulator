#pragma once

#include <arpa/inet.h>

#include <string>

namespace elf {

  namespace OUCH {
    namespace Constants {
      static const char LiqAdded = 'A';
      static const char LiqRemoved = 'R';
    }

    void set_alpha_field(const std::string& src, char* dest, size_t len);
    int ouch_to_native_int(int s);
  }

  namespace OUCH42 {
    namespace Constants {
      static const char SideBuy             = 'B';
      static const char SideSell            = 'S';
      static const char SideShort           = 'T';
      static const char SideShortExempt     = 'E';
      static const char SideInvalid         = '-';
      static const int MaxQty               = 1e6;
      static const char Agency              = 'A';
      static const char Principal           = 'P';
      static const char Riskless            = 'R';
      static const char ISOEligible         = 'Y';
      static const char ISONonEligible      = 'N';
      static const char CrossNone           = 'N';
      static const char NonRetail           = 'N';
      static const char StartOfDay          = 'S';
      static const char EndOfDay            = 'E';
      static const char DisplayAttributable = 'A';
    }

    namespace MessageType {
      static const char NewOrder       = 'O';
      static const char CancelOrder    = 'X';
      static const char SystemEvent    = 'S';
      static const char OrderAck       = 'A';
      static const char OrderCanceled  = 'C';
      static const char OrderExecuted  = 'E';
      static const char OrderRejected  = 'J';
      static const char OrderBroken    = 'B';
      static const char CancelPending  = 'P';
      static const char CancelRejected = 'I';
      static const char PriorityUpdate = 'T';
      static const char OrderModified  = 'M';
    }

    struct __attribute__((__packed__)) NewOrder {
    NewOrder() : type(MessageType::NewOrder), qty(0), px(0), tif(0), minqty(0) {}
      char type;
      char token[14];
      char side;
      uint32_t qty;
      char symbol[8];
      uint32_t px;
      uint32_t tif;
      char mpid[4];
      char display;
      char capacity;
      char iso;
      uint32_t minqty;
      char cross_type;
      char customer_type;

      void prepare_send();
    };

    struct __attribute__((__packed__)) CancelOrder {
    CancelOrder() : type(MessageType::CancelOrder), qty(0) {}
      char type;
      char token[14];
      uint32_t qty;
      void prepare_send();
    };

    struct __attribute__((__packed__)) SystemEvent {
    SystemEvent() : type(MessageType::SystemEvent), timestamp(0), event_code('_') {}
      char type;
      uint64_t timestamp;
      char event_code;
    };

    struct __attribute__((__packed__)) OrderAck {
    OrderAck() : type(MessageType::OrderAck), timestamp(0) {}
      char type;
      uint64_t timestamp;
      char token[14];
      char side;
      uint32_t qty;
      char symbol[8];
      uint32_t px;
      uint32_t tif;
      char mpid[4];
      char display;
      uint64_t oid;
      char capacity;
      char iso;
      uint32_t minqty;
      char cross_type;
      char state;
    };

    struct __attribute__((__packed__)) OrderCanceled {
    OrderCanceled() : type(MessageType::OrderCanceled), timestamp(0) {}
      char type;
      uint64_t timestamp;
      char token[14];
      uint32_t qty;
      char reason;
    };

    struct __attribute__((__packed__)) OrderExecuted {
    OrderExecuted() : type(MessageType::OrderExecuted), timestamp(0) {}
      char type;
      uint64_t timestamp;
      char token[14];
      uint32_t qty;
      uint32_t px;
      char liq_flag;
      uint64_t match_id;
    };

    struct __attribute__((__packed__)) BrokenOrder {
      char type;
      uint64_t timestamp;
      char token[14];
      uint64_t match_id;
      char reason;
    };

    struct __attribute__((__packed__)) OrderRejected {
    OrderRejected() : type(MessageType::OrderRejected), timestamp(0) {}
      char type;
      uint64_t timestamp;
      char token[14];
      char reason;
    };

    struct __attribute__((__packed__)) CancelPending {
      char type;
      uint64_t timestamp;
      char token[14];
    };

    struct __attribute__((__packed__)) CancelRejected {
    CancelRejected() : type(MessageType::CancelRejected), timestamp(0) {}
      char type;
      uint64_t timestamp;
      char token[14];
    };

    size_t message_size(const char msgtype);
  }
}
