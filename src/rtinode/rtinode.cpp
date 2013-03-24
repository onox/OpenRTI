/* -*-c++-*- OpenRTI - Copyright (C) 2009-2012 Mathias Froehlich
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
#include "Server.h"
#include "StringUtils.h"

#if !defined(_WIN32)
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif

using namespace OpenRTI;

static void usage(const char* argv0)
{
  std::cerr << argv0 << ": [-b] [-c configfile] [-f file] [-h] [-i address] [-p parent]" << std::endl;
}

static Server* _server = NULL;

static void sighandler(int sig)
{
  if (!_server)
    return;
  _server->setDone(true);
}

int
main(int argc, char* argv[])
{
  Server server;
  _server = &server;

  // We want to stop gracefully
#ifndef _WIN32
  ::signal(SIGHUP, sighandler);
#endif
  ::signal(SIGTERM, sighandler);
  ::signal(SIGINT, sighandler);

  bool background = false;
  bool defaultListen = true;

  Options options(argc, argv);
  while (options.next("bc:f:hi:p:s")) {
    switch (options.getOptChar()) {
    case 'b':
      background = true;
      break;
    case 'c':
      try {
        defaultListen = false;
        server.setUpFromConfig(options.getArgument());
      } catch (const Exception& e) {
        std::cerr << "Could not set up server from config file:" << std::endl;
        std::cerr << utf8ToLocale(e.getReason()) << std::endl;
      }
      break;
    case 'f':
      try {
        defaultListen = false;
        server.listenPipe(localeToUtf8(options.getArgument()), 20);
      } catch (const Exception& e) {
        std::cerr << "Could not set up pipe server transport:" << std::endl;
        std::cerr << utf8ToLocale(e.getReason()) << std::endl;
      }
      break;
    case 'h':
      usage(argv[0]);
      exit(EXIT_SUCCESS);
      break;
    case 'i':
      try {
        defaultListen = false;
        server.listenInet(localeToUtf8(options.getArgument()), 20);
      } catch (const Exception& e) {
        std::cerr << "Could not set up inet server transport:" << std::endl;
        std::cerr << utf8ToLocale(e.getReason()) << std::endl;
      }
      break;
    case 'p':
      try {
        server.connectParentInetServer(localeToUtf8(options.getArgument()), Clock::now() + Clock::fromSeconds(75));
      } catch (const Exception& e) {
        std::cerr << "Could not connect parent server:" << std::endl;
        std::cerr << utf8ToLocale(e.getReason()) << std::endl;
      }
      break;
    }
  }

  if (defaultListen) {
    // Try to listen on all sockets by default
    try {
      server.listenInet("::", 20);
    } catch (const Exception&) {
      try {
        server.listenInet("0.0.0.0", 20);
      } catch (const Exception& e) {
        std::cerr << "Could not set up default inet server transport:" << std::endl;
        std::cerr << utf8ToLocale(e.getReason()) << std::endl;
        return EXIT_FAILURE;
      }
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

  int ret = server.exec();
  _server = NULL;

  return ret;
}
