
/*
 * main.h
 *
 *  Created on: 2010-4-4
 *      Author: knuth
 */

#ifndef MAIN_H_
#define MAIN_H_

#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include 	<stdarg.h>
#include 	<errno.h>
#include 	<math.h>
#include 	<sys/types.h>
#include 	<netinet/in.h>
#include 	<sys/socket.h>
#include 	<sys/wait.h>
#include 	<unistd.h>
#include 	<arpa/inet.h>
#include 	<sys/mman.h>
#include 	<syscall.h>
#include 	<fcntl.h>
#include 	<sys/epoll.h>
#include 	<sys/time.h>
#include 	<sys/resource.h>
#include 	<pthread.h>
#include 	<sys/stat.h>
#include 	<locale.h>
#include 	<wchar.h>
#include 	<semaphore.h>
#include 	<signal.h>
#include 	<setjmp.h>
#include 	<time.h>
#include 	<limits.h>
#include 	<ctype.h>
#include 	<float.h>
#include 	<stddef.h>
#include	<netdb.h>
#include 	<iconv.h>
#define NDEBUG
#include <assert.h>
#include 	"pool.h"
#include 	"log.h"
#include 	"logic.h"
#include 	"hash.h"
#include 	"robot.h"
#include 	"public.h"
#include	"network.h"
#include 	"sms.h"
#include 	"automic.h"
#define ID_LEN	16

extern sem_t sem_rcm;
extern sem_t sem_rtcp;
extern tpool_t *pool;

#define NIPQUAD(addr) \
((unsigned char *)&addr)[0], \
((unsigned char *)&addr)[1], \
((unsigned char *)&addr)[2], \
((unsigned char *)&addr)[3]
#define NIPQUAD_FMT "%u.%u.%u.%u"

#define HI_8BIT(A) ((A&0xFF00)>>8)
#define LO_8BIT(A) (A&0xFF)
extern atomic_t *R_CON_NUM;
extern atomic_t *M_CON_NUM;

#endif /* MAIN_H_ */

