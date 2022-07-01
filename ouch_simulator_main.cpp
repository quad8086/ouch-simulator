#include <stdlib.h>
#include <signal.h>

#include <string>
#include <iostream>
#include <args.hxx>

#include "ouch_simulator.h"

using namespace std;
using namespace elf;
using namespace OUCHSim;

OUCHSimulator* __ouch_sim = nullptr;

const char*
ouch_simulator_version() {
#ifdef VERSION
  return VERSION;
#else
  return "unknown";
#endif
}

void
show_version() {
  cout << ouch_simulator_version() << endl;
}

void
signal_handler(int signum) {
  if(__ouch_sim) {
    __ouch_sim->shutdown();
    __ouch_sim = nullptr;
  }
}

int
main(int argc, char** argv) {
  args::ArgumentParser parser("convert_iexpcap", "");
  parser.helpParams.addDefault = true;
  args::ValueFlag<int> port(parser, "port", "specify listen port", {'p'}, 4722);
  args::Flag version(parser, "version", "show version", {'v', "version"});
  args::Flag trace_messages(parser, "trace_messages", "trace messages", {'t', "trace-messages"}, false);

  try {
    parser.ParseCLI(argc, argv);
  } catch(const runtime_error& e) {
    cout << parser;
    cout << e.what() << endl;
    return 1;
  }

  if(version) {
    show_version();
    exit(0);
  }

  OUCHSimulator ouch_sim;
  __ouch_sim = &ouch_sim;

  ::signal(SIGINT, signal_handler);
  ::signal(SIGTERM, signal_handler);
  ::signal(SIGHUP, SIG_IGN);
  ::signal(SIGPIPE, SIG_IGN);

  try {
    ouch_sim.init(args::get(port), args::get(trace_messages));
    ouch_sim.run();
  } catch (const std::runtime_error& e) {
    cout << "Error: " << e.what() << endl;
    exit(2);
  }

  exit(0);
}
