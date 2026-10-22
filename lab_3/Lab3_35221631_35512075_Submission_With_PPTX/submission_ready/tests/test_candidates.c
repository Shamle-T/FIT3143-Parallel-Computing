#define main wsn_program_main
#include "../src/wsn.c"
#undef main
#include <assert.h>

int main(void) {
    Config c={0};c.rows=3;c.cols=3;c.threshold=1;c.expiry=10;
    double alerts[9],seen[9];int free_ports[9]={0},answer[9];
    for(int i=0;i<9;i++) alerts[i]=seen[i]=-1;
    for(int version=0;version<3;version++) {
        c.variant=version;
        assert(candidates(&c,0,9,alerts,seen,free_ports,100,answer)==3);
        assert(answer[1]==2 && answer[2]==4 && answer[3]==6);
        alerts[2]=95;seen[4]=95;free_ports[4]=0;
        assert(candidates(&c,0,9,alerts,seen,free_ports,100,answer)==1);
        assert(answer[1]==6);
        /* Both exclusions expire; no evidence means inferred availability. */
        assert(candidates(&c,0,9,alerts,seen,free_ports,106,answer)==3);
        alerts[2]=seen[4]=-1;
    }
    c.rows=c.cols=1;
    assert(candidates(&c,0,1,alerts,seen,free_ports,100,answer)==0);
    /* Independent full-grid oracle covers corners, long lines, all eight ties,
       expiry boundaries and patterned recent observations. */
    for(int rows=1;rows<=7;rows++) for(int cols=1;cols<=7;cols++) {
        c.rows=rows;c.cols=cols;c.expiry=10;c.threshold=2;
        int n=rows*cols,expected[9],result[9],capacity[49];
        double last[49],observed[49];
        for(int scenario=0;scenario<4;scenario++) {
            for(int j=0;j<n;j++) {
                last[j]=(scenario && j%3==0)?90:-1;
                observed[j]=(scenario && j%4==0)?95:-1;
                capacity[j]=j%3;
            }
            double now=scenario==2?100.001:scenario==3?106:100;
            for(int source=0;source<n;source++) {
                int count=0;
                for(int j=0;j<n;j++) {
                    int hops=abs(source/cols-j/cols)+abs(source%cols-j%cols);
                    if(hops==2 && !(last[j]>=0 && now-last[j]<=c.expiry)
                       && !(observed[j]>=0 && now-observed[j]<=c.expiry && capacity[j]<c.threshold))
                        expected[++count]=j;
                }
                expected[0]=count;
                for(int version=0;version<3;version++) {
                    c.variant=version;
                    assert(candidates(&c,source,n,last,observed,capacity,now,result)==count);
                    for(int j=0;j<=count;j++) assert(result[j]==expected[j]);
                }
            }
        }
    }
    puts("PASS: ties, recent alert, recent low observation, expiry, isolated node");
    puts("PASS: independent oracle for every station in all 1..7 by 1..7 shapes, four cache states, three variants");
    return 0;
}
