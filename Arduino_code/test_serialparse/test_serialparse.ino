char cmd[9] ;
int asdf;
void setup(){

Serial.begin(9600);

//Serial.setTimeout(10); // 設定為每10毫秒結束一次讀取(數字愈小愈快)

Serial.print("wait..");
///

Serial.print("number:");
///
}

void loop(){

while (!Serial.available() > 0) {}

if(Serial.available()){

 //Serial.readBytes(cmd,9);
char cmd = Serial.read();
int a = Serial.parseInt();
int b = Serial.parseInt();
  //int b = a.toInt();
 Serial.write("OK!!");
 Serial.println(cmd);
 Serial.println(a);
 Serial.println(b);

}

}
