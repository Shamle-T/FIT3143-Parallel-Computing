#define main wsn_program_main
#include "../src/wsn.c"
#undef main

int main(int argc,char **argv) {
    MPI_Init(&argc,&argv);
    puts("nodes,variant,repetitions,seconds,checksum");
    for(int n=10;n<=10000;n*=10) {
        Config c={0};c.rows=1;c.cols=n;c.threshold=1;c.expiry=30;
        double *last=allocate(n,sizeof(double)),*seen=allocate(n,sizeof(double));
        int *available=allocate(n,sizeof(int));
        for(int i=0;i<n;i++) last[i]=seen[i]=-1;
        for(int variant=0;variant<3;variant++) {
            c.variant=variant;volatile int checksum=0;int answer[9];
            double start=MPI_Wtime();
            for(int repetition=0;repetition<10000;repetition++)
                checksum+=candidates(&c,repetition%n,n,last,seen,available,100,answer);
            printf("%d,%d,10000,%.12g,%d\n",n,variant,MPI_Wtime()-start,checksum);
        }
        free(last);free(seen);free(available);
    }
    MPI_Finalize();return 0;
}
