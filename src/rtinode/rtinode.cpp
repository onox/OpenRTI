/* -*-c++-*- OpenRTI - Copyright (C) 2009-2022 Mathias Froehlich
 *
 * This file is part of OpenRTI.
 *
 * OpenRTI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * OpenRTI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenRTI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <signal.h>
#include <iostream>

#include "Exception.h"
#include "Options.h"
#include "NetworkServer.h"
#include "StringUtils.h"

#if !defined(_WIN32)
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif

static void usage(const char* argv0)
{
  std::cerr << argv0 << ": [-b] [-c configfile] [-f file] [-h] [-i address] [-p parent]" << std::endl;
}

class SignalNetworkServer : public OpenRTI::NetworkServer {
public:
  SignalNetworkServer();
  virtual ~SignalNetworkServer();

  static void setDoneStatic();

private:
  static SignalNetworkServer* _networkServer;
};

SignalNetworkServer::SignalNetworkServer()
{
  _networkServer = this;
}

SignalNetworkServer::~SignalNetworkServer()
{
  _networkServer = NULL;
}

void
SignalNetworkServer::setDoneStatic()
{
  if (!_networkServer)
    return;
  _networkServer->setDone(true);
}

SignalNetworkServer* SignalNetworkServer::_networkServer = NULL;

extern "C" {

static void sighandler(int sig)
{
  SignalNetworkServer::setDoneStatic();
}

}

int
main(int argc, char* argv[])
{
  SignalNetworkServer networkServer;

  // We want to stop gracefully
#ifndef _WIN32
  ::signal(SIGHUP, sighandler);
#endif
  ::signal(SIGTERM, sighandler);
  ::signal(SIGINT, sighandler);

  bool background = false;
  bool defaultListen = true;

  OpenRTI::Options options(argc, argv);
  while (options.next("bc:f:hi:p:s")) {
    switch (options.getOptChar()) {
    case 'b':
      background = true;
      break;
    case 'c':
      try {
        defaultListen = false;
        networkServer.setUpFromConfig(options.getArgument());
      } catch (const OpenRTI::Exception& e) {
        std::cerr << "Could not set up server from config file:" << std::endl;
        std::cerr << OpenRTI::utf8ToLocale(e.getReason()) << std::endl;
        return EXIT_FAILURE;
      }
      break;
    case 'f':
      try {
        defaultListen = false;
        OpenRTI::URL url = OpenRTI::URL::fromUrl(OpenRTI::localeToUtf8(options.getArgument()));
        url.setProtocol("pipe");
        networkServer.listen(url, 20);
      } catch (const OpenRTI::Exception& e) {
        std::cerr << "Could not set up pipe server transport:" << std::endl;
        std::cerr << OpenRTI::utf8ToLocale(e.getReason()) << std::endl;
        return EXIT_FAILURE;
      }
      break;
    case 'h':
      usage(argv[0]);
      return EXIT_SUCCESS;
    case 'i':
      try {
        defaultListen = false;
        OpenRTI::URL url = OpenRTI::URL::fromUrl(OpenRTI::localeToUtf8(options.getArgument()));
        if (url.getProtocol().empty())
          url.setProtocol("rti");
        networkServer.listen(url, 20);
      } catch (const OpenRTI::Exception& e) {
        std::cerr << "Could not set up inet server transport:" << std::endl;
        std::cerr << OpenRTI::utf8ToLocale(e.getReason()) << std::endl;
        return EXIT_FAILURE;
      }
      break;
    case 'p':
      try {
        OpenRTI::URL url = OpenRTI::URL::fromUrl(OpenRTI::localeToUtf8(options.getArgument()));
        if (url.getProtocol().empty()) {
          if (!url.getPath().empty())
            url.setProtocol("pipe");
          else
            url.setProtocol("rti");
        }
        networkServer.connectParentServer(url, OpenRTI::Clock::now() + OpenRTI::Clock::fromSeconds(75));
      } catch (const OpenRTI::Exception& e) {
        std::cerr << "Could not connect parent server:" << std::endl;
        std::cerr << OpenRTI::utf8ToLocale(e.getReason()) << std::endl;
        return EXIT_FAILURE;
      }
      break;
    }
  }

  if (defaultListen) {
    // Try to listen on all sockets by default
    try {
      networkServer.listen(OpenRTI::URL::fromUrl("rti://"), 20);
    } catch (const OpenRTI::Exception& e) {
      std::cerr << "Could not set up default inet server transport:" << std::endl;
      std::cerr << OpenRTI::utf8ToLocale(e.getReason()) << std::endl;
      return EXIT_FAILURE;
    }
  }

#if defined(_WIN32) || defined(__sun) || defined(__hpux)
  if (background)
    std::cerr << "Running in background is not yet supported!" << std::endl;
#else
  if (background)
    daemon(0, 0);
#endif

#if !defined(_WIN32)
  struct rlimit limit;
  getrlimit(RLIMIT_NOFILE, &limit);
  limit.rlim_cur = limit.rlim_max;
  setrlimit(RLIMIT_NOFILE, &limit);
#endif

  return networkServer.exec();
}
