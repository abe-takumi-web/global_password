#include "controller.h"
#include "pinconfig.h"
#include "ikarashiMDC.h"
#include "mbed.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define TIME 1.6
//マクロ指定　爪の回す秒数
//プログラム仕様書
//https://github.com/abe-takumi-web/learn/blob/main/%E4%BB%95%E6%A7%98%E6%9B%B8.html
DigitalOut stop(PA_5);
Serial RS555(PC_10,PC_11,115200);

Bcon mycon(fepTX, fepRX, fepad);
Serial pc(pcTX, pcRX, 115200);

//五十嵐MDCの定義
ikarashiMDC motor[]={
    ikarashiMDC(0,0,SM,&RS555),
    ikarashiMDC(0,1,SM,&RS555),
    ikarashiMDC(0,2,SM,&RS555),
    ikarashiMDC(0,3,SM,&RS555),
    //４個目以降のモーターの定義がよくわからんので、のちに変わる可能性あり。
    ikarashiMDC(0,4,SM,&RS555),
    ikarashiMDC(0,5,SM,&RS555),
    ikarashiMDC(0,6,SM,&RS555)
    };
void nail(void)//指定した秒数間モーターを回し続ける。
{
    double  endTime;
    double totalTime = 0.0, setTime = 0.0;

    /* 時間をセット */
    setTime = TIME;

    /* タイマー開始（ミリ秒単位） */
    double startTime = clock() / CLOCKS_PER_SEC ;
    while(1){
        if(totalTime > setTime) break;
        endTime = (double)clock() / CLOCKS_PER_SEC ;
        totalTime = (double)endTime - (double)startTime;
        pc.printf("%.1f %d\n",(double)totalTime,(int)totalTime);
        motor[4].setSpeed(1);
    }
    motor[4].setSpeed(0);
    pc.printf("停止\n");
}
int main(){
    stop=1;
    mycon.StartReceive();
    uint8_t b[8];
    int16_t stick[4];
    int i;
    double speed[4];
    while(1){
        // スピードの定義
        for(i=0;i<4;i++){
            stick[i]  =  mycon.getStick(i);
            speed[i]  =  double(stick[i]/128.0);//stickが傾くほど強くなるシステム
            }
        int iti = 1;
        int zero = 0;
        double Turning = 0.6;
        int point = 0;
        point += b[0];  //開閉システムの角度（ざひょうてきな？）
        point -= b[1];
        double sum = 0.0;    //エレベータの座標
        sum += (double)speed[3];    //ここめちゃくちゃ重要なので、気をつけておいてください。speed[3]+?/128をするので。をするので。？は変数です。
            /** ボタンの配置
            *    0       4
            *  1   3   5   7
            *    2       6
            *
            *    1       3
            *  0─+─0   2─+─2   スティックの配置
            *    1       3
            *
            */
            /*    　ロボットのタイヤ
            *     |\\\|_______|///|            
            *       2 |       | 1            
            *         |       |             
            *       3 |_______| 0            
            *     |///|       |\\\|
            */
            /*──────────────────────────────足回り──────────────────────────────*/
            //　横の動き
            if(((stick[0]>30)||(stick[0]<-30))&&((stick[1]<70)&&(stick[1]>-70))){
                motor[0].setSpeed(speed[0]);
                motor[1].setSpeed(-speed[0]);
                motor[2].setSpeed(-speed[0]);
                motor[3].setSpeed(speed[0]);
            }
            //縦の動き
            else if(((stick[1]>30)||(stick[1]<-30))&&((stick[0]<70)&&(stick[0]>-70))){
                motor[2].setSpeed(-speed[1]);
                motor[1].setSpeed(speed[1]);
                motor[0].setSpeed(speed[1]);
                motor[3].setSpeed(-speed[1]);
                }
            //左斜め前右斜め後ろの動き
            else if(((stick[1]>70)&&(stick[0]<-70))||((stick[1]<-70)&&(stick[0]>70))){
                motor[1].setSpeed(-speed[1]);
                motor[3].setSpeed(speed[1]);
                motor[2].setSpeed(zero);
                motor[0].setSpeed(zero);
                }
            //右斜め前左斜め後ろの動き
            else if(((stick[1]>70)&&(stick[0]>70))||((stick[1]<-70)&&(stick[0]<-70))){
                motor[2].setSpeed(-speed[1]);
                motor[0].setSpeed(speed[1]);
                motor[1].setSpeed(zero);
                motor[3].setSpeed(zero);
                }
            
            //左回転
            else if(b[4]){
                int i;
                for(i=0;i<4;i++){
                    motor[i].setSpeed(Turning);
                }
            }
            //右回転
            else if(b[7]){
                int i;
                for(i=0;i<4;i++){
                    motor[i].setSpeed(-Turning);
                }
            }
    /*──────────────────────────────足回り終了──────────────────────────────*/
    /*──────────────────────────────胴体はじめ──────────────────────────────*/
    /* motor[4]=爪
    *  motor[5]=開閉システム
    *  motor[6]=エレベータ
    */
            //爪 
            else if(b[5]){
                    nail();
            }
            //爪でボールを取る
            else if(b[3]){
                motor[4].setSpeed((-iti)/2);
            }
            else if(b[2]){
                motor[4].setSpeed(iti/2);
            }
            //開閉システム
            //開けるほう
            else if(b[0]){                             //
                                                       // pointはモーターの角度を10の数字で表したものです。
                    if(point>0&&point<10){             // このプログラムだと中途半端な開き方と閉じ方だと、やばいことになりますが、
                        motor[5].setSpeed(iti);        // 中途半端に開いたり閉めたりはしないと思うので、賭けました。
                    }                                  // マジでこのプログラムの作者は国語力がないので、
                    else motor[5].setSpeed(zero);      // この説明じゃ意味が分からないかもしれませんが、
            }                                          // 解読してください。
            //閉めるほう                                 //　　
            else if(b[1]){                             //　　
                    if(point>0&&point<10){             //ここのpointのやつ後で変えるかもしれん　　
                        motor[5].setSpeed(-iti);       //　point+10 -= b[1];とかになるかも
                    }                              
                    else motor[5].setSpeed(zero);     
            }
            //エレベータ
            else if((stick[3]>30)||(stick[3]<-30)){               
                pc.printf("%d",sum);
                if(point>10){              //開いているとき
                    if((sum>0)&&(sum<100)){
                        motor[6].setSpeed(speed[3]);
                        motor[4].setSpeed((-speed[3])/2);
                    } 
                    else if(sum>100||sum<0){ //100という数字は今後変わってくる可能性あり。 
                            motor[6].setSpeed(zero);
                            motor[4].setSpeed(zero);
                    }   
                }
                else if(point<0){                 //閉まっているとき
                    if((sum>0)&&(sum<100)){
                        motor[6].setSpeed(speed[3]);
                    } 
                    else if(sum>100||sum<0){ //100という数字は今後変わってくる可能性あり。  
                        motor[6].setSpeed(zero);
                    }        
                }
            }
    /*──────────────────────────────胴体終了──────────────────────────────*/
            //ニュートラルの状態
            else{
                int i;
                for(i=0;i<7;i++){
                    motor[i].setSpeed(0);
                }
            }
                    //TeraTermで見る用のプログラム
        for (int i=0; i<8; i++) b[i] = mycon.getButton(i);
        for (int i=0; i<4; i++) stick[i] = mycon.getStick(i);
        for (int i=0; i<8; i++) pc.printf("%d ", b[i]);
        pc.printf(" | ");
        for (int i=0; i<4; i++) pc.printf("%3d ", stick[i]);
        pc.printf(" | ");
        break;
        pc.printf("%d",sum);
        pc.printf("%d ",point);
        if (mycon.status) pc.printf("received\r\n");
        else pc.printf("anything error...\r\n");
    }
}
