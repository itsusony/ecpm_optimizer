#define POOL_MAXLEN 100000000 // 100 Million

// key: id,year,month,day,hour,min
// value: imp and cost
typedef struct dataobj {
    time_t ts; // timestamp
    int year,month,day,hour,min;
    unsigned long id; // ad id, etc.
    unsigned long imp; // count of imp
    unsigned long cost; // money
} DataObj;

pthread_mutex_t pool_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tmppool_lock = PTHREAD_MUTEX_INITIALIZER;
DataObj** pool = NULL;
DataObj** tmppool = NULL;
unsigned long pool_idx = 0;
unsigned long tmppool_idx = 0;

void init_pool() {
    pool = malloc(POOL_MAXLEN * sizeof(DataObj*));
    pool_idx = 0;
}
void init_tmppool() {
    tmppool = malloc(POOL_MAXLEN * sizeof(DataObj*));
    tmppool_idx = 0;
}

void append_tmppool(const unsigned long id, const unsigned long cost) {
    time_t now_t = time(NULL);
    struct tm *dt = localtime(&now_t);

    DataObj *o = malloc(sizeof(DataObj));
    o->ts = now_t;
    o->id = id;
    o->imp = cost == 0 ? 1 : 0;
    o->cost = cost;
    o->year = dt->tm_year+1900;
    o->month= dt->tm_mon+1;
    o->day  = dt->tm_mday;
    o->hour = dt->tm_hour;
    o->min  = dt->tm_min;

    pthread_mutex_lock(&tmppool_lock);
    tmppool[tmppool_idx++] = o;
    pthread_mutex_unlock(&tmppool_lock);
}

void* tmppool_summary(void* argv) {
    while(1) {
        DataObj** ref_tmppool = NULL;
        unsigned long ref_tmppool_idx = 0;

        pthread_mutex_lock(&tmppool_lock);
        if (tmppool_idx > 0) {
            ref_tmppool = tmppool;
            ref_tmppool_idx = tmppool_idx;
            init_tmppool();
        }
        pthread_mutex_unlock(&tmppool_lock);

        if (ref_tmppool_idx > 0) {
            DataObj *o, *so;
            unsigned long i,j;

            pthread_mutex_lock(&pool_lock);
            for (i=0;i<ref_tmppool_idx;i++) {
                o = ref_tmppool[i];
                int found = 0;
                // find the data which has same id and ts
                for (j=0;j<pool_idx;j++) {
                    so = pool[j];
                    if (so->id == o->id &&
                        so->min == o->min &&
                        so->hour == o->hour &&
                        so->day == o->day &&
                        so->month == o->month &&
                        so->year == o->year
                    ) {
                        found = 1;
                        so->imp += o->imp;
                        so->cost += o->cost;
                        break;
                    }
                }
                if (!found) {
                    pool[pool_idx++] = o;
                } else {
                    free(o);
                }
            }
            pthread_mutex_unlock(&pool_lock);

            free(ref_tmppool);
        }
        sleep(1);
    }
}
