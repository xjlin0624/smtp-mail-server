/* configuration */
int8 *whitelist[] = {
    $1 "127.0.0.1",
    $1 "192.168.10.1",
    $1 0
};

Server mx[] = {
    {"gmail.com", "alt4.gmail-smtp-in.l.google.com", "172.253.135.26", 0},
    {"doctorbirch.com", "mailcluster.loopia.se", "93.188.3.13", 0},
    {0}
};