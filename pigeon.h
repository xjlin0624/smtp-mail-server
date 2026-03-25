/* pigeon.h */

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include "birchutils/birchutils.h"
#include <time.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#define DEBUG
#define VERSION     "0.1"
#define PORT        2525
#define IP          INADDR_ANY

typedef unsigned char int8;
typedef unsigned short int int16;
typedef unsigned int int32;
typedef unsigned long long int int64;
typedef int8 MsgID;

enum e_state {
    idle = 0,
    connecting = 1,
    connected = 2,
    hello = 3,
    mail = 4,
    rcpt = 5,
    data = 6,
    payload = 7,
    disconnecting = 8,
    disconnected = 9
};
typedef enum e_state State;

struct s_user {
    int8 homedir[128];
    int8 shell[64];
};
typedef struct s_user User;

struct s_email {
    State state;
    int8 src[64];
    int8 dst[64];
    int8 id[128];
    int8 subject[128];
    int8 *domain;
    int8 data[];
};
typedef struct s_email Email;

struct s_command {
    int8 cmd[64];
    int8 subcmd[64];
    int8 args[128];
};
typedef struct s_command Command;

struct s_connection {
    int32 s;
    int8 src[64];
    int8 dst[64];
    State state;
};
typedef struct s_connection Connection;

struct s_server {
    int8 domain[64];
    int8 server[64];
    int32 ip;
};
typedef struct s_server Server;

#define $1 (int8 *)
#define $2 (int16)
#define $4 (int32)
#define $8 (int64)
#define $c (char *)
#define $i (int)

#ifdef DEBUG
    #define log(f, args...)             printf(f, ##args)
#else
    #define log(x, xs...) void
#endif 

#define sendcmd(_s, e, f, args...)  do {    \
    dprintf($i _s, f, ##args);              \
    e->state++;                             \
    fprintf(stderr, "-> ");                 \
    fprintf(stderr, f, ##args);             \
} while(0)
#define mailerror(_s, e, msg)       do {    \
    log("%s\n", msg);                       \
    close(_s);                              \
    e->state = disconnected;                \
    return false;                           \
} while(0)

#define senddata(s,c,f,args...)      do {   \
    int8 _buf[2048];                        \
    zero(_buf, 2048);                       \
    snprintf(_buf, 2047, f, args);          \
    (volatile void)senddata_(s, c, _buf);   \
} while(0)

bool deliver(Email*, User*);
int8 *email2user(int8*);
User *getuser(int8*);
void fail(int8*);
bool sendmail(Email*, Server*);
bool strmatch(int8*, int8*); // might not need
int8 *mailheader(Email*);
Email *mkemail(int8*, int8*, int8*, int8*, int16);
Server *mkserver(int8*, int8*, int32);
int8 *findchar(int8*, int8);
MsgID *mkid(int8*);
void cim(int8*);
int32 setup(void);
void senddata_(int32, int16, int8*);
void mainloop(int32);
void childloop(Connection*);
int main(int, char**);

