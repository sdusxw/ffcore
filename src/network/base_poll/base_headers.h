//
// Created by boon on 17-9-22.
//

#ifndef TLRS_BASE_HEADERS_H
#define TLRS_BASE_HEADERS_H

#if defined(WIN32) || defined(WIN64)

#include <winsock2.h>

#include <Windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "ws2_32.lib")


#ifdef _MSC_VER
#pragma warning(disable: 4309)
#endif

#else
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/un.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#if defined (__sun)
#include <sys/errno.h>
#elif	(defined(__alpha) || defined(__linux))
#include <sys/procfs.h>
#elif  defined(__hpux)
#include <sys/pstat.h>
#endif

#endif

#endif //TLRS_BASE_HEADERS_H
