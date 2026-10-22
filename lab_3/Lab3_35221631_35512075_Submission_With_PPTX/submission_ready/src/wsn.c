/* FIT3143 Lab 3. Preparation assisted by Codex; see docs/AI_RECORD.md.
 * Savin Vindiv De Alwis, 35221631, sdea0018@student.monash.edu
 * Shamle Thilaksiri, 35512075, wthi0003@student.monash.edu
 * Build: mpicc -std=c11 -O2 -fopenmp -Wall -Wextra -Wpedantic src/wsn.c -o wsn
 * MPI is called only by the initial thread (MPI_THREAD_FUNNELED).
 */
#define _POSIX_C_SOURCE 200809L
#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include <math.h>

enum { QUERY, REPLY, REPORT, REDIRECT, CONTROL, FLAGS, SYNC, ACK, ROUNDTRIP, NCLASS };
static const char *classes[] = {"query", "reply", "report", "redirect",
    "control", "flags", "sync", "ack", "alert_roundtrip"};
enum { TQUERY=10, TREPLY, TREPORT, TREDIRECT, TACK };
typedef struct {
    int rows, cols, ports, rounds, variant, threshold, scenario, warmup;
    double tick, expiry;
    unsigned seed;
    char output[256];
} Config;
typedef struct {
    int rank, row, col, round, free_ports, neighbour[4], available[4];
    int messages, bytes;
    long long discovered;
} Report;
typedef struct { double seconds[NCLASS], calls[NCLASS], bytes[NCLASS]; } Metrics;
static Metrics metric;
static int measuring;

static void fail(const char *s) {
    fprintf(stderr, "%s\n", s);
    MPI_Abort(MPI_COMM_WORLD, 2);
}
static void *allocate(size_t count, size_t size) {
    if (size && count > SIZE_MAX/size) fail("Allocation overflow");
    void *p = calloc(count ? count : 1, size);
    if (!p) fail("Allocation failed");
    return p;
}
static int integer(const char *s, int low, int high) {
    char *end; errno=0; long v=strtol(s,&end,10);
    if(errno || !*s || *end || v<low || v>high) fail("Invalid integer argument");
    return (int)v;
}
static double real(const char *s) {
    char *end; errno=0; double v=strtod(s,&end);
    if(errno || !*s || *end || !isfinite(v) || v<0) fail("Invalid time argument");
    return v;
}
static Config arguments(int argc, char **argv) {
    if(argc<4) fail("Usage: wsn rows cols ports [--rounds N --variant 0|1|2 --threshold N --tick seconds --expiry seconds --scenario random|full|empty|ties|mixed --warmup N --seed N --output prefix]");
    Config c={0}; c.rows=integer(argv[1],1,10000); c.cols=integer(argv[2],1,10000);
    c.ports=integer(argv[3],1,1024); c.rounds=10; c.threshold=1; c.tick=10;
    c.expiry=30; c.seed=3143; strcpy(c.output,"results/run");
    for(int i=4;i<argc;i+=2) {
        if(i+1>=argc) fail("Missing option value");
        const char *v=argv[i+1];
        if(!strcmp(argv[i],"--rounds")) c.rounds=integer(v,1,100000);
        else if(!strcmp(argv[i],"--variant")) c.variant=integer(v,0,2);
        else if(!strcmp(argv[i],"--threshold")) c.threshold=integer(v,1,c.ports);
        else if(!strcmp(argv[i],"--warmup")) c.warmup=integer(v,0,10000);
        else if(!strcmp(argv[i],"--seed")) c.seed=(unsigned)integer(v,0,INT_MAX);
        else if(!strcmp(argv[i],"--tick")) c.tick=real(v);
        else if(!strcmp(argv[i],"--expiry")) c.expiry=real(v);
        else if(!strcmp(argv[i],"--output")) {
            if(strlen(v)>240) fail("Output prefix too long");
            strcpy(c.output,v);
        } else if(!strcmp(argv[i],"--scenario")) {
            const char *names[]={"random","full","empty","ties","mixed"};
            int found=0;
            for(int j=0;j<5;j++) if(!strcmp(v,names[j])) {c.scenario=j;found=1;}
            if(!found) fail("Unknown scenario");
        } else fail("Unknown option");
    }
    /* POSIX/Linux time_t is signed; reject a conversion outside its range. */
    if(c.tick>=ldexp(1.0,(int)(sizeof(time_t)*CHAR_BIT-1))) fail("Tick exceeds time_t range");
    return c;
}
static void record(int type, double start, int calls, int bytes) {
    if(measuring) {metric.seconds[type]+=MPI_Wtime()-start;
        metric.calls[type]+=calls;metric.bytes[type]+=bytes;}
}
static void pause_for(double seconds) {
    struct timespec t={(time_t)seconds,(long)((seconds-floor(seconds))*1e9)};
    while(nanosleep(&t,&t) && errno==EINTR) {}
}
/* xorshift state is private to one port; no shared rand() data race. */
static unsigned next_random(unsigned *s) {
    unsigned x=*s; x^=x<<13; x^=x>>17; x^=x<<5; return *s=x;
}
static int distance(int a,int b,int cols) {
    return abs(a/cols-b/cols)+abs(a%cols-b%cols);
}
/* The example in Task 2 specifies the two-hop frontier, not a global search.
 * A recent self-alert OR a fresh low observation rules a candidate out.
 * No recent evidence is only inferred availability, never guaranteed capacity.
 */
static int candidates(const Config *c,int source,int n,const double *last_alert,
        const double *observed,const int *available,double now,int *answer) {
    int count=0,best=INT_MAX;
    const int dr[8]={-2,-1,-1,0,0,1,1,2};
    const int dc[8]={0,-1,1,-2,2,-1,1,0};
    int limit=c->variant==0?n:8;
    for(int index=0;index<limit;index++) {
        int j=index;
        if(c->variant) {
            int row=source/c->cols+dr[index],col=source%c->cols+dc[index];
            if(row<0 || row>=c->rows || col<0 || col>=c->cols) continue;
            j=row*c->cols+col;
        }
        int d=distance(source,j,c->cols);
        if(d!=2) continue;
        if(last_alert[j]>=0 && now-last_alert[j]<=c->expiry) continue;
        if(observed[j]>=0 && now-observed[j]<=c->expiry && available[j]<c->threshold) continue;
        if(d<best) {best=d;count=0;}
        if(d==best) answer[++count]=j;
    }
    /* Canonical rank order makes ties independent of the frontier traversal. */
    for(int i=2;i<=count;i++) for(int j=i;j>1 && answer[j]<answer[j-1];j--) {
        int tmp=answer[j];answer[j]=answer[j-1];answer[j-1]=tmp;
    }
    answer[0]=count; return count;
}
static void exchange(const Config *c,MPI_Comm cart,const int nb[4],
        const int send[4],int recv[4],const int dest[4],const int src[4],int tag,int kind) {
    MPI_Request req[8];int q=0,edges=0;
    double start=MPI_Wtime();
    if(c->variant==0) {
        /* Nonperiodic axis order forms finite chains; Sendrecv breaks send/send waits. */
        for(int d=0;d<4;d++) {
            MPI_Sendrecv(&send[d],1,MPI_INT,dest[d],tag,&recv[d],1,MPI_INT,
                         src[d],tag,cart,MPI_STATUS_IGNORE);
            if(dest[d]!=MPI_PROC_NULL) edges++;
        }
    } else {
        for(int d=0;d<4;d++) if(src[d]!=MPI_PROC_NULL)
            MPI_Irecv(&recv[d],1,MPI_INT,src[d],tag,cart,&req[q++]);
        for(int d=0;d<4;d++) if(dest[d]!=MPI_PROC_NULL) {
            MPI_Isend(&send[d],1,MPI_INT,dest[d],tag,cart,&req[q++]);edges++;
        }
        MPI_Waitall(q,req,MPI_STATUSES_IGNORE);
    }
    (void)nb;
    record(kind,start,edges,edges*(int)sizeof(int));
}
static void node_round(const Config *c,int rank,int round,int free_ports,
        MPI_Comm cart,const int nb[4],FILE *events) {
    int congested=free_ports<c->threshold,send[4],requests[4]={0};
    for(int d=0;d<4;d++) send[d]=congested;
    exchange(c,cart,nb,send,requests,nb,nb,TQUERY,QUERY);
    int dest[4],src[4],availability[4]={-1,-1,-1,-1};
    for(int d=0;d<4;d++) {
        send[d]=free_ports;
        dest[d]=requests[d] ? nb[d] : MPI_PROC_NULL;
        src[d]=congested ? nb[d] : MPI_PROC_NULL;
    }
    exchange(c,cart,nb,send,availability,dest,src,TREPLY,REPLY);
    int chosen=-1,degree=0,reply_out=0;
    for(int d=0;d<4;d++) if(nb[d]!=MPI_PROC_NULL) {
        degree++;reply_out+=requests[d];
        if(availability[d]>=c->threshold && (chosen<0 || nb[d]<chosen)) chosen=nb[d];
    }
    int alert=congested && chosen<0;
    long long discovered=alert?(long long)time(NULL):0;
    double start=MPI_Wtime();
    MPI_Gather(&alert,1,MPI_INT,NULL,1,MPI_INT,0,MPI_COMM_WORLD);
    record(FLAGS,start,1,sizeof(int));
    if(alert) {
        Report r={0};r.rank=rank;r.row=rank/c->cols;r.col=rank%c->cols;
        r.round=round;r.free_ports=free_ports;r.discovered=discovered;
        r.messages=degree+reply_out+degree+degree; /* sent/received query and reply */
        r.bytes=r.messages*(int)sizeof(int);
        memcpy(r.neighbour,nb,sizeof(r.neighbour));
        memcpy(r.available,availability,sizeof(r.available));
        double roundtrip=MPI_Wtime();start=MPI_Wtime();
        MPI_Send(&r,sizeof(r),MPI_BYTE,0,TREPORT,MPI_COMM_WORLD);
        record(REPORT,start,1,sizeof(r));
        int response[9];
        start=MPI_Wtime();
        MPI_Recv(response,9,MPI_INT,0,TREDIRECT,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        record(REDIRECT,start,1,(response[0]+1)*(int)sizeof(int));
        record(ROUNDTRIP,roundtrip,1,0);
        fprintf(events,"round=%d free=%d base_candidates=%d",round,free_ports,response[0]);
        for(int j=1;j<=response[0];j++) fprintf(events," %d",response[j]);
        fprintf(events,"\n");
    } else fprintf(events,"round=%d free=%d local_redirect=%d\n",round,free_ports,chosen);
}
static void base_round(const Config *c,int round,int n,FILE *log,
        double *last_alert,double *observed,int *available,long long *alerts) {
    int *flags=allocate(n+1,sizeof(int)),zero=0;
    double start=MPI_Wtime();
    MPI_Gather(&zero,1,MPI_INT,flags,1,MPI_INT,0,MPI_COMM_WORLD);
    record(FLAGS,start,1,(n+1)*(int)sizeof(int));
    int count=0;for(int j=1;j<=n;j++) count+=flags[j];
    Report *reports=allocate(count,sizeof(Report));
    MPI_Request *req=allocate(count,sizeof(MPI_Request));
    int k=0;
    start=MPI_Wtime();
    if(c->variant==2) {
        for(int j=1;j<=n;j++) if(flags[j]) {
            MPI_Irecv(&reports[k],sizeof(Report),MPI_BYTE,j,TREPORT,MPI_COMM_WORLD,&req[k]);
            k++;
        }
        MPI_Waitall(count,req,MPI_STATUSES_IGNORE);
    } else if(c->variant==1) {
        for(k=0;k<count;k++) {
            MPI_Status status;
            MPI_Probe(MPI_ANY_SOURCE,TREPORT,MPI_COMM_WORLD,&status);
            MPI_Recv(&reports[k],sizeof(Report),MPI_BYTE,status.MPI_SOURCE,TREPORT,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        }
    } else {
        for(int j=1;j<=n;j++) if(flags[j])
            MPI_Recv(&reports[k++],sizeof(Report),MPI_BYTE,j,TREPORT,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    }
    record(REPORT,start,count,count*(int)sizeof(Report));
    double now=MPI_Wtime();
    /* Update entire batch first, so simultaneous alerts do not depend on receive order. */
    for(k=0;k<count;k++) {
        Report *r=&reports[k];last_alert[r->rank]=now;
        observed[r->rank]=now;available[r->rank]=r->free_ports;
        for(int d=0;d<4;d++) if(r->neighbour[d]!=MPI_PROC_NULL) {
            observed[r->neighbour[d]]=now;available[r->neighbour[d]]=r->available[d];
        }
    }
    int *answers=allocate((size_t)9*count,sizeof(int));
    for(k=0;k<count;k++) {
        Report *r=&reports[k];int *answer=&answers[(size_t)k*9];
        candidates(c,r->rank,n,last_alert,observed,available,now,answer);
        char timestamp[32];time_t wall=(time_t)r->discovered;struct tm tm;
        gmtime_r(&wall,&tm);strftime(timestamp,sizeof(timestamp),"%Y-%m-%dT%H:%M:%SZ",&tm);
        fprintf(log,"{\"round\":%d,\"source\":%d,\"coordinates\":[%d,%d],\"discovered_utc\":\"%s\",\"free\":%d,\"neighbour_messages\":%d,\"neighbour_bytes\":%d,\"neighbours\":[",
                round,r->rank,r->row,r->col,timestamp,r->free_ports,r->messages,r->bytes);
        for(int d=0;d<4;d++) fprintf(log,"%s{\"rank\":%d,\"free\":%d}",d?",":"",r->neighbour[d],r->available[d]);
        fprintf(log,"],\"inferred_candidates\":[");
        for(int j=1;j<=answer[0];j++) fprintf(log,"%s%d",j>1?",":"",answer[j]);
        fprintf(log,"]}\n");
    }
    start=MPI_Wtime();
    for(k=0;k<count;k++) {
        int *answer=&answers[(size_t)k*9];
        if(c->variant==2) MPI_Isend(answer,answer[0]+1,MPI_INT,reports[k].rank+1,TREDIRECT,MPI_COMM_WORLD,&req[k]);
        else MPI_Send(answer,answer[0]+1,MPI_INT,reports[k].rank+1,TREDIRECT,MPI_COMM_WORLD);
    }
    if(c->variant==2) MPI_Waitall(count,req,MPI_STATUSES_IGNORE);
    int bytes=0;for(k=0;k<count;k++) bytes+=(answers[(size_t)k*9]+1)*(int)sizeof(int);
    record(REDIRECT,start,count,bytes);*alerts+=count;
    free(answers);free(req);free(reports);free(flags);
}
int main(int argc,char **argv) {
    int provided,world,size;
    MPI_Init_thread(&argc,&argv,MPI_THREAD_FUNNELED,&provided);
    if(provided<MPI_THREAD_FUNNELED) fail("MPI_THREAD_FUNNELED unavailable");
    MPI_Comm_rank(MPI_COMM_WORLD,&world);MPI_Comm_size(MPI_COMM_WORLD,&size);
    Config c=arguments(argc,argv);int n=c.rows*c.cols;
    if(size!=n+1) fail("MPI process count must equal rows*cols+1 (includes base)");
    MPI_Comm workers,cart=MPI_COMM_NULL;
    MPI_Comm_split(MPI_COMM_WORLD,world?1:MPI_UNDEFINED,world,&workers);
    int rank=-1,nb[4]={MPI_PROC_NULL,MPI_PROC_NULL,MPI_PROC_NULL,MPI_PROC_NULL};
    if(world) {
        int dims[2]={c.rows,c.cols},periods[2]={0,0};
        MPI_Cart_create(workers,2,dims,periods,0,&cart);
        MPI_Comm_rank(cart,&rank);
        MPI_Cart_shift(cart,0,1,&nb[0],&nb[1]);MPI_Cart_shift(cart,1,1,&nb[2],&nb[3]);
    }
    char path[320];snprintf(path,sizeof(path),"%s.%s%d.log",c.output,world?"node":"base",world?rank:0);
    FILE *log=fopen(path,"w");if(!log) fail("Cannot open log; create output directory first");
    char host[MPI_MAX_PROCESSOR_NAME];int hostlen;MPI_Get_processor_name(host,&hostlen);
    fprintf(log,"# host=%s world=%d station=%d variant=%d ports=%d threshold=%d expiry=%g tick=%g\n",host,world,rank,c.variant,c.ports,c.threshold,c.expiry,c.tick);
    long long alerts=0;double begin=MPI_Wtime();
    if(!world) {
        double *last=allocate(n,sizeof(double)),*seen=allocate(n,sizeof(double));
        int *available=allocate(n,sizeof(int));for(int j=0;j<n;j++) last[j]=seen[j]=-1;
        for(int round=0;round<c.rounds+c.warmup;round++) {
            measuring=round>=c.warmup;int control=1;double start=MPI_Wtime();
            MPI_Bcast(&control,1,MPI_INT,0,MPI_COMM_WORLD);record(CONTROL,start,1,sizeof(int));
            base_round(&c,round,n,log,last,seen,available,&alerts);
            start=MPI_Wtime();MPI_Barrier(MPI_COMM_WORLD);record(SYNC,start,1,0);
        }
        measuring=1;int control=0;double start=MPI_Wtime();
        MPI_Bcast(&control,1,MPI_INT,0,MPI_COMM_WORLD);record(CONTROL,start,1,sizeof(int));
        start=MPI_Wtime();
        for(int j=1;j<=n;j++) {int ack;MPI_Recv(&ack,1,MPI_INT,j,TACK,MPI_COMM_WORLD,MPI_STATUS_IGNORE);}
        record(ACK,start,n,n*(int)sizeof(int));
        fprintf(log,"# alerts=%lld elapsed_seconds=%.9f shutdown_acks=%d\n",alerts,MPI_Wtime()-begin,n);
        free(last);free(seen);free(available);
    } else {
        int *ports=allocate(c.ports,sizeof(int));unsigned *states=allocate(c.ports,sizeof(unsigned));
        for(int j=0;j<c.ports;j++) states[j]=c.seed+104729u*(unsigned)(rank+1)+7919u*(unsigned)(j+1);
        int control=1,free_ports=0,round=0;
        omp_set_dynamic(0);
        /* Persistent team: each port owns one array element; barriers publish snapshots. */
        #pragma omp parallel num_threads(c.ports) shared(control,free_ports,round,ports,states)
        {
            int port=omp_get_thread_num();
            #pragma omp master
            {if(omp_get_num_threads()!=c.ports) fail("OpenMP could not provide one thread per port");}
            for(;;) {
                #pragma omp master
                {
                    measuring=round>=c.warmup;double start=MPI_Wtime();
                    MPI_Bcast(&control,1,MPI_INT,0,MPI_COMM_WORLD);record(CONTROL,start,1,sizeof(int));
                    free_ports=0;
                }
                #pragma omp barrier
                if(!control) break;
                if(c.scenario==1) ports[port]=1;
                else if(c.scenario==2) ports[port]=0;
                else if(c.scenario==3) ports[port]=distance(rank,0,c.cols)<2;
                else if(c.scenario==4) ports[port]=(round%3==0 || (rank+round)%3!=0);
                else ports[port]=(next_random(&states[port])%100)<75;
                #pragma omp atomic update
                free_ports+=!ports[port];
                #pragma omp barrier
                #pragma omp master
                {
                    node_round(&c,rank,round,free_ports,cart,nb,log);
                    pause_for(c.tick);
                    double start=MPI_Wtime();MPI_Barrier(MPI_COMM_WORLD);record(SYNC,start,1,0);
                    round++;
                }
                #pragma omp barrier
            }
        }
        /* Flush port logs before acknowledging the base's termination. */
        if(fflush(log)) fail("Node log flush failed");
        measuring=1;int ack=1;double start=MPI_Wtime();
        MPI_Send(&ack,1,MPI_INT,0,TACK,MPI_COMM_WORLD);record(ACK,start,1,sizeof(int));
        free(states);free(ports);
    }
    if(fclose(log)) fail("Log close failed");
    /* Per-rank measurements preserve distributions and avoid cross-clock subtraction. */
    double *all=world?NULL:allocate((size_t)size*3*NCLASS,sizeof(double));
    MPI_Gather(&metric,3*NCLASS,MPI_DOUBLE,all,3*NCLASS,MPI_DOUBLE,0,MPI_COMM_WORLD);
    if(!world) {
        snprintf(path,sizeof(path),"%s.metrics.csv",c.output);FILE *f=fopen(path,"w");if(!f) fail("Metrics open failed");
        fprintf(f,"world_rank,class,seconds,calls,payload_bytes\n");
        for(int r=0;r<size;r++) for(int j=0;j<NCLASS;j++) {
            size_t offset=(size_t)r*3*NCLASS;
            fprintf(f,"%d,%s,%.12g,%.0f,%.0f\n",r,classes[j],all[offset+j],all[offset+NCLASS+j],all[offset+2*NCLASS+j]);
        }
        if(fclose(f)) fail("Metrics close failed");
        snprintf(path,sizeof(path),"%s.base0.log",c.output);
        f=fopen(path,"a");if(!f) fail("Base summary open failed");
        double messages=0,bytes=0,collectives=0;
        for(int r=0;r<size;r++) {
            size_t offset=(size_t)r*3*NCLASS;
            for(int j=0;j<NCLASS;j++) {
                int outgoing=(r>0 && (j==QUERY || j==REPLY || j==REPORT || j==ACK)) || (r==0 && j==REDIRECT);
                if(outgoing) {messages+=all[offset+NCLASS+j];bytes+=all[offset+2*NCLASS+j];}
                if(r==0 && (j==CONTROL || j==FLAGS || j==SYNC)) collectives+=all[offset+NCLASS+j];
            }
        }
        fprintf(f,"# measured_point_to_point_messages=%.0f measured_payload_bytes=%.0f collective_operations=%.0f (warmup excluded; MPI protocol overhead excluded)\n",messages,bytes,collectives);
        if(fclose(f)) fail("Base summary close failed");
        free(all);
    }
    if(cart!=MPI_COMM_NULL) MPI_Comm_free(&cart);
    if(workers!=MPI_COMM_NULL) MPI_Comm_free(&workers);
    MPI_Finalize();return 0;
}
