#include <Servo.h>
Servo myservo;

//开始或结束按钮
#define pinStartButton A2
//摇头或自动跟踪按钮
#define pinShakeButton A3
//速度控制按钮
#define pinSpeedButton A4
//定时按钮
#define pinTimeButton A5
//连接超声波测距仪Ⅰ的Echo引脚
#define pinTrace1 0
//连接超声波测距仪Ⅱ的Echo引脚
#define pinTrace2 14
//连接两场声波测距仪的Trig引脚
#define pinTraceTrig 11
//连接直流电机
#define pinServo1 3
//连接伺服舵机
#define pinServo2 10

//记录开始或结束按钮的状态
int StartButton=0;
//记录摇头或自动跟踪按钮的状态
int ShakeButton=0;
//记录开始或结束按钮之前的状态
int temp1=0;
//记录摇头或自动跟踪按钮之前的状态
int temp2=0;
//记录风扇速度
int Speed=1;
//记录舵机旋转角度
int n=0;
//记录所定时间
int timecount=0;
//作计时用
long int time0;
int flag=0;
//不包含小数点位的段码
int a[10]= {126,48,109,121,51,91,95,112,127,123};

/****************************************************************************/
/*函数原型：int ButtonState(int BUTTON,int *BUTTON_STATE,int *temp)          */
/*传入参数：int BUTTON,int *BUTTON_STATE,int *temp                           */
/*返 回 值：0或1                                                             */
/*函数功能：返回按钮的当前状态                                                */
/****************************************************************************/
int ButtonState(int BUTTON,int *BUTTON_STATE,int *temp);

/****************************************************************************/
/*函数原型：void Display(int number,int location)                            */
/*传入参数：int number,int location                                          */
/*返 回 值：无                                                               */
/*函数功能：将某一数字显示在数码管的某一位置                                   */
/****************************************************************************/
void Display(int number,int location);

/****************************************************************************/
/*函数原型：void SpeedControl()                                              */
/*传入参数：无                                                               */
/*返 回 值：无                                                               */
/*函数功能：显示并调整速度                                                    */
/****************************************************************************/
void SpeedControl();

/****************************************************************************/
/*函数原型：void ShakeOrTrace()                                              */
/*传入参数：无                                                               */
/*返 回 值：无                                                               */
/*函数功能：摇头模式则控制来回摇头，自动跟踪模式则控制风扇跟踪                  */
/****************************************************************************/
void ShakeOrTrace();

/****************************************************************************/
/*函数原型：double Length(int pin)                                           */
/*传入参数：int pin                                                          */
/*返 回 值：障碍物到超声波测距仪的距离(cm)                                     */
/*函数功能：测量障碍物到超声波测距仪的距离                                     */
/****************************************************************************/
double Length(int pin);

/****************************************************************************/
/*函数原型：void TimeSet()                                                   */
/*传入参数：无                                                               */
/*返 回 值：无                                                               */
/*函数功能：定时，每按一次按钮增加十秒                                         */
/****************************************************************************/
void TimeSet();

/****************************************************************************/
/*函数原型：void CountDown()                                                 */
/*传入参数：无                                                               */
/*返 回 值：无                                                               */
/*函数功能：倒计时并显示剩余时间                                              */
/****************************************************************************/
void CountDown();

void setup() {
  // put your setup code here, to run once:
  pinMode(pinTrace1,INPUT);
  pinMode(2,OUTPUT);
  pinMode(pinServo1,OUTPUT);
  for(int i=4; i<=9; i++) pinMode(i,OUTPUT);
  myservo.attach(pinServo2);
  pinMode(pinTraceTrig,OUTPUT);
  pinMode(12,OUTPUT);
  pinMode(13,OUTPUT);
  pinMode(pinTrace2,INPUT);
  pinMode(15,OUTPUT);
  pinMode(pinStartButton,INPUT_PULLUP);
  pinMode(pinShakeButton,INPUT_PULLUP);
  pinMode(pinSpeedButton,INPUT_PULLUP);
  pinMode(pinTimeButton,INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  while(ButtonState(pinStartButton,&StartButton,&temp1)==0) analogWrite(pinServo1,0);
  //Start
  analogWrite(pinServo1,Speed*63);
  SpeedControl();
  ShakeOrTrace();
  TimeSet();
  if(timecount>0) CountDown();
}

int ButtonState(int BUTTON,int *BUTTON_STATE,int *temp) {
  if(digitalRead(BUTTON)==0&&*BUTTON_STATE==0) *BUTTON_STATE=1;
  else if(digitalRead(BUTTON)&&*BUTTON_STATE==1) *BUTTON_STATE=2;
  else if(digitalRead(BUTTON)==0&&*BUTTON_STATE==2) *BUTTON_STATE=3;
  else if(digitalRead(BUTTON)&&*BUTTON_STATE==3) *BUTTON_STATE=0;
  //只有当按钮状态发生改变时才防抖动，防止每次调用函数时都delay(100)影响程序其它部分的运行
  if(*BUTTON_STATE!=*temp) delay(100);
  *temp=*BUTTON_STATE;
  if(*BUTTON_STATE==0||*BUTTON_STATE==1) return 0;
  else return 1;
}

void Display(int number,int location) {
  int temp,i;
  temp=a[number];
  digitalWrite(11+location,HIGH);
  for(i=9; i>=4; i--) {
    digitalWrite(i,!(temp%2));
    temp/=2;
  }
  digitalWrite(2,!(temp%2));
  delay(5);
  digitalWrite(11+location,LOW);
}

void SpeedControl() {
  //将速度显示在数码管的第四个位置
  Display(Speed,4);
  if(digitalRead(pinSpeedButton)==0) {
    if(Speed<4) Speed++;
    else Speed=1;
    //防抖动
    delay(200);
  }
}

void ShakeOrTrace() {
  //摇头
  if(ButtonState(pinShakeButton,&ShakeButton,&temp2)==0) {
    if(n>=360) n=0;
    if(n<180) {
      myservo.write(n);
      delay(10);
      n++;
    }
    if(n>=180) {
      myservo.write(360-n);
      delay(10);
      n++;
    }
  }
  //自动跟踪
  else {
    double length1,length2;
    length1=Length(pinTrace1);
    delay(2);
    length2=Length(pinTrace2);
    delay(2);
    //超过超声波测距仪的测量范围则不自动跟踪
    if(length1>100||length2>100);
    else {
      if(length1-length2>=2) {
        //向角度增大方向旋转
        if(n<=179) {
          n++;
          myservo.write(n);
          delay(10);
        } else if(n>=181) {
          n--;
          myservo.write(360-n);
          delay(10);
        } else {
          n=180;
          myservo.write(n);
          delay(10);
        }
      }
      if(length2-length1>=2) {
        //向角度减小方向旋转
        if(n>=1&&n<=180) {
          n--;
          myservo.write(n);
          delay(10);
        } else if(n>180&&n<=359) {
          n++;
          myservo.write(360-n);
          delay(10);
        } else {
          n=0;
          myservo.write(n);
          delay(10);
        }
      }
    }
  }
}

double Length(int pin) {
  int t1,t2;
  digitalWrite(pinTraceTrig,HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTraceTrig,LOW);
  while(digitalRead(pin)==0);
  t1=micros();
  while(digitalRead(pin));
  t2=micros();
  return 0.034*(t2-t1)/2;
}

void TimeSet() {
  if(digitalRead(pinTimeButton)==0) {
    timecount+=10;
    if(timecount>100) timecount-=100;
    //防抖动
    delay(200);
  }
}

void CountDown() {
  if(flag==0) {
    //倒计时开始
    time0=millis();
    flag=1;
  }
  //每过一秒timecount减1
  if(millis()-time0>=1000) {
    timecount--;
    time0+=1000;
  }
  //将剩余时间显示在数码管前两位
  Display(timecount%10,2);
  Display(timecount/10%10,1);
  //倒计时结束，风扇关闭
  if(timecount==0) {
    flag=0;
    StartButton=0;
  }
}

