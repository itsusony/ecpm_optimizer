typedef struct rankobj {
    unsigned long id,imp,cost,ecpm;
} RankObj;

pthread_mutex_t rank_pool_lock = PTHREAD_MUTEX_INITIALIZER;
RankObj **rank_pool = NULL;
unsigned long rank_pool_idx = 0;

int compare_ecpm_rank(const void *a, const void *b){
    RankObj *ea = *(RankObj**)a;
    RankObj *eb = *(RankObj**)b;
    return eb->ecpm - ea->ecpm;
}

void* pool_ranking(void* argv) {
    while(1) {
        RankObj **ranks = NULL;
        int ranks_len = 0;
        unsigned long i,j;
        pthread_mutex_lock(&pool_lock);
        if (pool_idx > 0) {
            ranks = malloc(pool_idx * sizeof(RankObj*));
            for (i=0;i<pool_idx;i++) {
                DataObj* o = pool[i];
                int exist = 0;
                for (j=0;j<ranks_len;j++) {
                    RankObj* ro = ranks[j];
                    if (ro->id == o->id) {
                        exist = 1;
                        ro->imp+=o->imp;
                        ro->cost+=o->cost;
                    }
                }
                if (!exist) {
                    RankObj* new_ro = malloc(sizeof(RankObj));
                    new_ro->id = o->id;
                    new_ro->imp = o->imp;
                    new_ro->cost = o->cost;
                    new_ro->ecpm = 0;
                    ranks[ranks_len++] = new_ro;
                }
            }
        }
        pthread_mutex_unlock(&pool_lock);

        //calc ecpm
        for(i=0;i<ranks_len;i++) {
            RankObj *ro = ranks[i];
            ro->ecpm = ro->imp == 0 ? 0 : (unsigned long)((double)ro->cost / (double)ro->imp * 1000);
        }

        if (ranks_len > 0) {
            // sort by ecpm
            qsort(ranks, ranks_len, sizeof(RankObj*), compare_ecpm_rank);

            // make place to storage old array, for free
            RankObj **old_ranks = NULL;
            int old_ranks_len = 0;

            pthread_mutex_lock(&rank_pool_lock);

                old_ranks = rank_pool;
                old_ranks_len = rank_pool_idx;
    
                rank_pool = ranks;
                rank_pool_idx = ranks_len;

            pthread_mutex_unlock(&rank_pool_lock);

            // free
            for(i=0;i<old_ranks_len;i++) {
                free(old_ranks[i]);
            }
            free(old_ranks);
        }
#ifdef DEBUG
        sleep(5);
#else
        sleep(60);
#endif
    }
}
