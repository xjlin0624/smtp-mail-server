/* client.c */

#include "pigeon.h"

static signed int tmp;
static int16 s;

bool expect(int16 s2, int8 *str) {
    int8 buf[256];

    assert(s2 > 0);
    assert(str && (strlen($c str) > 0));

    zero(buf, 256);
    tmp = read($i s, $c buf, 255);

    
    log("<- %s\n", $c buf);
    

    if (tmp <= 0) 
        return false;
    if (!strmatch(buf, str))
        return false;
    return true;
}

bool sendmail(Email *email, Server *server) {
    struct sockaddr_in sock;
    
    email->state++;
    tmp = socket(AF_INET, SOCK_STREAM, 0);
    assert(tmp > 0);
    s = (int16)tmp;

    sock.sin_family = AF_INET;
    sock.sin_port = htons(587);
    sock.sin_addr.s_addr = server->ip;
    tmp = connect($i s, (struct sockaddr *)&sock, sizeof(struct sockaddr));
    assert(!tmp);

    email->state++;
    log("Connected to %s\n", $c server->server);

    do switch(email->state) {

        /* CONNECTED */
        case connected:
            if (expect(s, $1 "220 "))
                sendcmd(s, email, "EHLO %s\n", email->domain);
            else 
                mailerror(s, email, "Server banner not understood");
            break;
        
        /* HELLO SENT */
        case helo:
            if (expect(s, $1 "250 "))
                sendcmd(s, email, "MAIL FROM:<%s>\n", email->src);
            else 
                mailerror(s, email, "HELLO command failed");
            break;
        
        /* [MAIL FROM] SENT */
        case mail:
            if (expect(s, $1 "250 "))
                sendcmd(s, email, "RCPT TO:<%s>\n", email->dst);
            else 
                mailerror(s, email, "MAIL command failed");
            break;
        
        /* [RCPT TO] SENT */
        case rcpt:
            if (expect(s, $1 "250 "))
                sendcmd(s, email, "DATA\n");
            else 
                mailerror(s, email, "RCPT command failed");
            break;
        
        /* DATA SENT */
        case data:
            /* (xx) */
            if (expect(s, $1 "354 "))
                sendcmd(s, email, "%s\r\n%s\n\r\n.\r\n", mailheader(email), email->data);
            else 
                mailerror(s, email, "DATA command failed");
            break;

        /* PAYLOAD SENT */
        case payload:
            if (expect(s, $1 "250 "))
                sendcmd(s, email, "QUIT\n");
            else 
                mailerror(s, email, "Email payload failed");
            break;

        /* DISCONNECTING */
        case disconnecting:
            if (expect(s, $1 "221 "))
                close(s);
            else 
                mailerror(s, email, "QUIT command failed");
            break;

        default:
            fail($1 "Unknown error (sendmail[switch])");
    } while (email->state != disconnected);

    return true;

};

// int main(int argc, char *argv[]) {
//     Email *email;
//     Server *serv;
//     bool ret;
//     email = mkemail(
//         $1 "postmaster@gmail.com",
//         $1 "jb@doctorbirch.com",
//         $1 "Hello! Coding My Own Email Server",
//         $1 "Hello from a student :) \n",
//         $2 19
//     );

//     if (email)
//         printf("%s\n", email->id);

//     serv = mkserver(
//         email->domain, 
//         $1 "mailcluster.loopia.se", 
//         inet_addr("93.188.3.13")
//     );
    
//     ret = sendmail(email, serv);
    
//     free(email);
//     free(serv);

//     return 0;
// };

