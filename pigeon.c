/* pigeon.c */
#include "pigeon.h"

bool continuation;

int8 *copyuntil(int8* str, int8  start, int8 stop) {
    int8 *p, *e;

    if (!str) {
        return $1 0;
    }

    p = findchar(str, start);
    if (!p) 
        p = str;

    e = findchar(p, stop);
    if (e){
        e--; 
        *e = 0;
    }

    return p;
}

int8 *loweruntil(int8* str, int8 c) {
    int8 *p;

    if (!str) {
        return $1 0;
    }
    for (p=str; (*p) && (*p != c); p++) {
        *p = tolower(*p);
    }

    return str;
}

void cim(int8 *dir) {
    DIR *fd;

    assert(dir);
    fd = opendir($c dir);
    if (!fd) {
        if (errno == ENOENT) 
            assert(!mkdir($c dir, 00700));
        else
            fprintf(stderr, "Unexpected error:%s\n", strerror(errno));
            abort();
    }
    else
        closedir(fd);

    return;
}

bool deliver(Email *email, User *user) {
    int8 *username, *homedir, *filename;
    int8 *header, *payload, *prefix;
    int8 mail[2048];
    int8 dir[128];
    signed int fd;
    int16 size;
    bool ret;

    assert(email && user);
    username = email2user(email->src);
    homedir = user->homedir;
    prefix = $1 "mail.";
    payload = email->data;
    header = mailheader(email);
    assert(header);


    zero(dir, 128);
    snprintf($c dir, 127, "%s/mail/", $c homedir);

    cim(dir);
    filename = tempnam($c dir, $c prefix);
    assert(filename);

    zero(mail, 2048);
    snprintf($c mail, 2047, "%s\n%s\n", 
        header, payload);

    fd = open($c filename, O_WRONLY|O_CREAT, 0600);
    assert(fd > 0);
    size = $2 strlen($c mail);
    assert(size);

    ret = write(fd, $c mail, $i size) > 0; 

    close(fd);
    return ret;
}

void fail(int8* msg) {
    log("%s\n", msg);
    exit(-1);
    return;
};

bool strmatch(int8 *haystack, int8 *needle) {
    int8 *p;
    int16 len;

    len = $2 strlen($c needle);
    assert(haystack && needle && len);

    for (p = findchar(haystack, *needle); p; p = findchar((p)?++p:$1 "", *needle))
        if (p)
            if (!strncmp($c (p-1), $c needle, $i len))
                return true;
    return false;
};

// bool strmatch(int8 *haystack, int8 *needle) {
//     return (strstr($c haystack, $c needle) != NULL);
// }

int8 *findchar(int8 *haystack, int8 needle) {
    int8 *p, *ret;
    ret = $1 0;
    if (!(*haystack)) 
        return $1 0;
    for (p=haystack; *p; p++)
        if (*p == needle) {
            ret = p+1;
            break;
        };
    return ret;
}

int8 *email2user(int8 *email) {
    int8 *p;
    char buf[64];
    static int8 ret[64];

    assert(email);

    if (*email == '@')
        return $1 0;

    zero(buf, 64);
    zero(ret, 64);
    strncpy($c buf, $c email, 63);

    p = findchar(buf, '@');
    if (!p) 
        return $1 0;
    p--;

    *p = 0;
    strncpy($c ret, $c buf, 63);

    return ret;
}

User *getuser(int8 *user) {
    struct passwd *pw;
    int16 size;
    User *p;

    assert(user);
    pw = getpwnam($c user);
    if (!pw)
        return (User *)0;

    size = sizeof(struct s_user);
    p = (User *)malloc($i size);
    zero($1 p, size);

    strncpy($c p->homedir, $c pw->pw_dir, 127);
    strncpy($c p->shell, $c pw->pw_shell, 63);

    return p;
}

MsgID *mkid(int8 *domain) {
    time_t t;
    static MsgID buf[128];

    t = time((time_t)0);
    assert(domain);

    zero($1 buf, 128);
    snprintf($c buf, 127, "%d.%s", $i t, $c domain);

    return buf;
}

Server *mkserver(int8 *domain, int8 *server, int32 ip) {
    Server *p;

    assert(domain && server && ip);
    p = (Server *)malloc(sizeof(struct s_server));
    assert(p);
    zero($1 p, sizeof(struct s_server));

    strncpy($c p-> domain, $c domain, 63);
    strncpy($c p-> server, $c server, 63);
    p->ip = ip;

    return p;
}

Email *mkemail(
    int8 *src, 
    int8 *dst,
    int8 *subject,
    int8 *data,
    int16 len
) {
    Email *p;
    int16 size;

    assert(src && dst && subject && data && len);
    size = len + sizeof(struct s_email);
    p = (Email *)malloc($i size);
    assert(p);

    zero($1 p, size);
    p->state = idle;
    strncpy($c p->src, $c src, 63);
    strncpy($c p->dst, $c dst, 63);
    strncpy($c p->subject, $c subject, 127);
    memcpy($c p->data, $c data, $i len);
    p->domain = findchar(p->src, (int8)'@');
    strncpy($c p->id, $c mkid(p->domain), 127);

    return p;
}

int8 *mailheader(Email *email) {
    static int8 buf[1024];
    zero(buf, 1024);
    snprintf($c buf, 1023, 
        "Message-ID: %s\r\n"
        "From: %s\r\n" 
        "To: %s\r\n" 
        "Subject: %s\r\n", 
        $c email->id, $c email->src, $c email->dst, $c email->subject);

    return buf;
}

int32 setup() {
    struct sockaddr_in sock;
    signed int tmp;
    int32 s;

    s = $4 socket(AF_INET, SOCK_STREAM, 0);
    assert(s > 0);
    sock.sin_family = AF_INET;
    sock.sin_port = htons(PORT);
    // NEED TO CHANGE PORT
    sock.sin_addr.s_addr = IP;
    tmp = bind($i s, (struct sockaddr *)&sock, sizeof(struct sockaddr));
    assert(!tmp);
    tmp = listen($i s, 10);
    assert(!tmp);
    
    return s;
}

void senddata_(int32 s, int16 code, int8 *data) {
    int8 buf[2048];
    int16 size;
    signed int tmp;

    zero(buf, 2048);
    snprintf(buf, 2047, "%.03d-%s\n", $i code, $c data);
    size = $2 strlen($c buf);
    assert(size);
    
    tmp = write($i s, $c buf, $i size);
    assert(tmp > 0);
    
    return ;
}

Command *parse(int8 *s) {
    int16 n;
    Command *ret;
    Cmd_ cmd;
    int8 *p, *str, *args;

    assert(s);
    str = loweruntil(s, " ");

    if (!strncmp($c str, "ehlo ", (n=$2 5))) {
        cmd = ehlo;
        args = str + n;
    }
    else if (!strncmp($c str, "data", (n=$2 4))) {
        cmd = data;
        args = $1 0;
    }
    else if (!strncmp($c str, "quit", (n=$2 4))) {
        cmd = quit;
        args = $1 0;
    }
    else if (!strncmp($c str, "mail from:", (n=$2 10))) {
        cmd = mailfrom;
        args = copyuntil((str + n), '<', '>');
    }
    else if (!strncmp($c str, "rcpt to:", (n=$2 8))) {
        cmd = rcptto;
        args = copyuntil((str + n), '<', '>');
    }
    else { 
        return (Command *)0;
    }

    n = sizeof(struct s_command);
    ret = (Command *)malloc($i n);
    zero($1 ret, n);

    ret->cmd = cmd;
    strncpy($c ret->args, $c args, 127);

    return ret;

}


void childloop(Connection *c) {
    Command *cmd;
    int8 buf[2048];
    signed int tmp;

    zero(buf, 2048);
    tmp = read($i c->s, $c buf, 2047);
    if (tmp < 1) {
        sleep(1);

        return;
    }

    cmd = parse(buf);
}

void mainloop(int32 s) {
    struct sockaddr_in sock;
    signed int tmp, size;
    Connection *conn;
    int32 c;

    assert(s > 0);
    size = sizeof(struct sockaddr);
    tmp = accept($i s, (struct sockaddr *)&sock, &size);
    if (tmp < 1) {
        log("Lost incoming connection\n");
        sleep(1);

        return;
    }
    c = $4 tmp;
    tmp = fork();
    if (tmp) {
        log("Connection from %s established\n", todotted(sock.sin_addr.s_addr));
        sleep(1);
        return;
    }
    else {
        size = sizeof(struct s_connection);
        conn = (Connection *)malloc(size);
        zero($1 conn, $2 size);
        conn->s = c;
        conn->state = connected;

        senddata(c, 220, "Connected to the Carrier Pigeon v%s mailserver.",
            VERSION
        );

        continuation = true;
        while (continuation) 
            childloop(conn);
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    int32 s;

    s = setup();
    assert(s > 0);
    log("Listening on port %d\n", PORT);
    do mainloop(s);
    while(1);
    close($i s);

    return 0;
}

/*
int test(int argc, char *argv[]) {
    Email *email;
    int8 *username;
    User *user;
    bool ret;
    int8 *dir;

    // dir = $1 "/Users/joannalin/mail";
    // cim(dir);

    email = mkemail(
        $1 "joannalin@test.com",
        $1 "postmaster@gmail.com",
        $1 "Re: Test",
        $1 "Testing testing...\n",
        $2 19
    );

    // email = $1 "joannalin@doctorbirch.com";
    username = email2user(email->src);
    printf("Username: '%s'\n", username);

    user = getuser(username);
    if (user) {
        printf("Homedir: '%s'\nShell: '%s'\n", user->homedir, user->shell);
        ret = strmatch(user->shell, $1 "sh");
        if (ret) 
            printf("Matches\n");
        else 
            printf("Does not match\n");
        
        if (deliver(email, user))
            printf("True\n");
        else 
            printf("False\n");
        
        free(user);
        free(email);
    }
    else
        printf("No such user\n");
    return 0;
}
*/

// lesson 5 00:03:33