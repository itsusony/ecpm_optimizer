#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <event.h>
#include <evhttp.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define DEBUG 0
#define LISTEN_ADDR "0.0.0.0"
#define LISTEN_PORT 8888

#include "pool.c"
#include "ranking.c"

void req_handler(struct evhttp_request *r, void *arg) {
    struct evbuffer *evbuf = evbuffer_new();
    evhttp_add_header(r->output_headers, "Content-Type", "text/plain");

    struct evkeyvalq keys;
    evhttp_parse_query(r->uri,&keys);

    const char *cmd = evhttp_find_header(&keys,"cmd"); // system command call

    if (cmd && strlen(cmd)) {
        if (strncmp(cmd, "ping", 4) == 0) {
            evbuffer_add(evbuf, "pong", 4);
            evhttp_send_reply(r, HTTP_OK, "", evbuf);
        } else if (strncmp(cmd, "ranking", 7) == 0) {
            // output json format: [ {id:ID,imp:IMP,cost:COST,ecpm:ECPM}, ... ]
            pthread_mutex_lock(&rank_pool_lock);
            evbuffer_add_printf(evbuf, "id,imp,cost,ecpm\n");
            if (rank_pool_idx > 0) {
                unsigned long i;
                for(i=0;i<rank_pool_idx;i++) {
                    RankObj* ro = rank_pool[i];
                    if (!ro->ecpm) continue;
                    evbuffer_add_printf(evbuf, "%ld,%ld,%ld,%ld", ro->id,ro->imp,ro->cost,ro->ecpm);
                    if (i != rank_pool_idx-1) evbuffer_add(evbuf, "\n", 1);
                }
            }
            pthread_mutex_unlock(&rank_pool_lock);
            evhttp_send_reply(r, HTTP_OK, "", evbuf);
        } else {
            goto errproc;
        }
    } else {
        const char *id = evhttp_find_header(&keys,"id"); // id: ad id
        const char *cost = evhttp_find_header(&keys,"cost"); // cost in conv request

        unsigned long idval = id ? atol(id) : 0;
        unsigned long cval = cost ? atol(cost) : 0;
        
        if (idval > 0 && cval >= 0) {
            append_tmppool(idval, cval);
            evbuffer_add(evbuf, "ok", 2);
            evhttp_send_reply(r, HTTP_OK, "", evbuf);
        } else {
            goto errproc;
        }
    }

    if (0) {
        errproc:;
        evbuffer_add(evbuf, "err", 3);
        evhttp_send_reply(r, HTTP_BADREQUEST, "", evbuf);
    }

    evbuffer_free(evbuf);
    evhttp_clear_headers(&keys);
}

int main(int argc, const char* argv[]) {
    fprintf(stderr, "start listening: %s:%d\n", LISTEN_ADDR, LISTEN_PORT);
  
    struct event_base *ev_base;
    struct evhttp *httpd;
  
    ev_base = event_base_new();
    httpd = evhttp_new(ev_base);
    if (evhttp_bind_socket(httpd, LISTEN_ADDR, LISTEN_PORT) < 0) {
        fprintf(stderr, "error: port open failed\n");
        exit(EXIT_FAILURE);
    }

    init_tmppool();
    init_pool();

    pthread_t t1,t2;
    pthread_create(&t1, NULL, tmppool_summary, NULL);
    pthread_create(&t2, NULL, pool_ranking, NULL);

    evhttp_set_gencb(httpd, req_handler, NULL);
    event_base_dispatch(ev_base);
    evhttp_free(httpd);
    event_base_free(ev_base);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
  
    return 0;
}
