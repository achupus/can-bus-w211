#include "IC.h"
#include "KWP2000.h"
#include "IC_ENV.h"

extern void sendCanMessages(int addrs, uint8_t *data, int count, bool output = false);
extern IC_ENV ic;
extern MCP2515 canB;

Decapsulator::Decapsulator(unsigned long id)
{
  this->id = id;
}

bool Decapsulator::receive(CANMSG msg)
{
  if (msg.adrsValue != id) return false;
  if (msg.data[0] < 0x10){
    if (msg.data[1] == 0x7f && msg.data[3] == 0x78){ return false; }  //skip if processing
    response_len = msg.data[0];
    for (int i = 0; i < 7; i++) response[i] = msg.data[i+1];
    return true;
  }
  Serial.println("receive ddd");
  return false;
}

void Diag::start()
{
  started = true;
  Serial.println("diag start");
  last_keep_alive = millis();
  canB.sendMsg(0x1c, 8, 0x02, 0x10, 0x92, 0xff, 0xff, 0xff, 0xff, 0xff);
}

void Diag::stop()
{
  started = false;
  Serial.println("diag stop");
}

void Diag::loop()
{
  if (!started) return;
  if (millis() - last_keep_alive > 1000) {
    Serial.println("diag loop");
    last_keep_alive = millis();
    canB.sendMsg(0x1c, 8, 0x02, 0x3e, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff);
  }
}

IC_Screen::IC_Screen(Diag *diag)
{
  this->diag = diag;
  Serial.println("screen start");
}

void IC_Screen::start()
{
  Serial.println("screen start");
  started = true;
  this->diag->start();
  uint8_t data[4] = {0x31, 0x03, 0x08, 0x00};
  diag_send(1460, data, 4, false);
}

void IC_Screen::stop()
{
  Serial.println("screen stop");
  started = false;
  this->diag->stop();
  uint8_t data[2] = {0x32, 0x03};
  diag_send(1460, data, 2, false);
}


void IC_Screen::loop()
{
  if (!started) return;
  if (current_menu == 0) {
    if (millis() - temp_screen_update > 1000) {
      temp_screen_update = millis();

      //Serial.println("1 update");
      uint8_t data[5] = {0x31, 0x03, 0x05, temp_x, temp_y};
      diag_send(1460, data, 5, false);
      //for (int i = 0; i < 5; i++) {
      //  data[3] = i;
      //  
      //}
      
      

    }
  }
  //Serial.println("screen loop");
}

void IC_Screen::pixel()
{
  //Serial.println("pixel");
  temp_x++;
  if (temp_x > 120) {
    temp_y++;
    temp_x = 0;
  }
  uint8_t data[5] = {0x31, 0x03, 0x05, temp_x, temp_y};
  diag_send(1460, data, 5, false);
}

void IC_Screen::next_menu()
{
  
  if (current_menu >= 1) current_menu = 0;
  else current_menu++;
  Serial.print("menu: ");
  Serial.println(current_menu);
}

void IC_Screen::priv_menu()
{
  if (current_menu == 0) current_menu = 1;
  else current_menu--;
  Serial.print("menu: ");
  Serial.println(current_menu);
}

// ic_music_center [10 len 03 27 01  XX XX len2] [21 10 XX XX XX XX XX XX] [22 XX XX XX XX XX XX XX]
// ic_music_top    [10 len 03 29 00(c/l/r) XX XX XX]   [21 XX XX XX XX XX XX XX]
// ic_music_arrows [10 08? 03 28 02  XX 01 XX]   [21 02 HH -- -- -- -- --]
//                  08 03 28 02 XX 01 XX 02 HH

// unknown5        [10 0a  03 26 01 00 01 03]    [21 10 00 00 9d 01 00 00]
//                  10 13  03 26 01 00 01 0c      21 10 41 55 44 49 4f 20   22 6f 66 66 00 00 52 --    // Audio off
//                  10 13  05 26 01 00 01 0c      21 10 50 68 6f 6e 65 20   22 6f 66 66 00 00 c8 --    // Phone off
//                  10 11  04 26 01 00 01 0a      21 10 4e 41 56 20 6f 66   22 66 00 00 01 00 c8 --    // NAV off 

//                  03 26 01 00 01 0c[len2] 10 41 55 44 49 4f 20 6f 66 66 00 00 52
//                  05 26 01 00 01 0c[len2] 10 50 68 6f 6e 65 20 6f 66 66 00 00 c8

// unknown4        [10 0f  03 24 05 40 01 01]    [21 00 00 00 11 01 00 00] [22 00 24? -- -- -- -- --]
//                  10 0f  03 24 05 40 01 01      21 00 00 00 11 01 00 00   22 00 24? -- -- -- -- -- 
//                  10 0f  05 24 05 40 01 01      21 00 00 00 11 01 00 00   22 00 22? -- -- -- -- --  
//                  10 0f  04 24 05 40 01 01      21 00 00 00 11 01 00 00   22 00 23 -- -- -- -- -- 

//                  03 24 05 40 01 01 00 00 00 11 01 00 00 00 HH
//                  05 24 05 40 01 01 00 00 00 11 01 00 00 00 HH
//                  04 24 05 40 01 01 00 00 00 11 01 00 00 00 HH
     
// unknown2        [len 05  04 06 -- -- -- --]
//                  

//                  len 05  21 06 -- -- -- --
//                 [len 03  21 06 -- -- -- --]
//                  len 04  21 06 -- -- -- --  

//                 [len 03  25 06 -- -- -- --]
//                  len 03  25 06 -- -- -- -- 
//                  len 04  25 06 -- -- -- -- 
//                  len 05  25 06 -- -- -- --


// unknown6         [len 03  20 02 11 HH -- --]
//                   len 05  20 02 11 HH -- -- 
//                   len 04  20 02 11 HH -- --  


//service
// 03 13 27 06
// 13 24 01 90 11 04   00 01 00 10 50 53 65   72 76 69 63 65 20 64   61 74 61 00 31 73 0d

   


//  03  20 02 11 c3
//  03  21 06
//  03  23 d8 //off
//  03  24 05 40 01 01 00 00 00 11 01 00 00 00 24
//  03  24 02 00 01 01 00 00 00 12 [09 01] [0a 02] [00(c/l/r) (T O P) 00] 3d
//      24 01 82 11 03 00 01 00 10 50 53 65 72 76 69 63 65 20 64 61 74 61 00 31 73 0d - Service data
//      24 01 90 11 04 00 01 00 10 50 53 65 72 76 69 63 65 20 6d 65 6e 75 00 07 38 38 - Service menu



//  03  25 06

//  03  27 01 00 01 0c[len2] 10 41 55 44 49 4f 20 6f 66 66 00 00 52 //center as well?
//  03  28 02 XX 01 XX 02 HH  //arrows
//  03  29 00? XX XX XX XX XX XX XX XX XX XX  //top line

//  03  26 01 00 01 0c[len2] 10 41 55 44 49 4f 20 6f 66 66 00 00 52 //center text
//      26 01 00 03 [05[len]  02 33 42 45] [14 00 6b 6d 20 72 65 6d 61 69 6e 2e 20 2b 31 38 38 35 38] [13 00 44 61 79 73 20 72 65 6d 61 69 6e 2e 20 2b 37 30 33] 00 08 69 6f 6e 00 ab  //3BE
//      26 01 00 04 [0e 02 53 65 72 76 69 63 65 20 64 61 74 61] [07 00 49 74 65 6d 73] [0d 00 41 64 64 69 74 2e 20 57 6f 72 6b] [0e 00 43 6f 6e 66 69 72 6d 61 74 69 6f 6e] 00 ab 


//01 16 01 09 01 e1 07 04 1a 0e 0e 0a 00 63 00 00 00 00 00 00  //экран e
//



void ic_music_center_line_test(char *str, uint8_t test_param, uint8_t offset)
{
    int slen = strlen(str);
    int len = slen + 9;
    
    uint8_t data2[32] = {0x03, 0x26, 0x01, 0x00, 0x01, slen + 2, 0x10};

    for (int i = 0; i < slen; i++) data2[i+7] = str[i];
    data2[7 + slen + 0] = 0x00;

    uint8_t hash = 0xFF;
    for (int i = 0; i < len-1; i++) hash -= i;
    for (int i = 0; i < slen+7; i++) hash -= data2[i];
    
    
    hash -= offset;
    data2[7 + slen + 1] = hash;
    
    uint8_t output2[48];
    uint8_t output_len;
    diag_incapsulate(data2, slen + 9, output2, &output_len);
    sendCanMessages(420, output2, output_len/8, true);
    
}




void ic_music_center_line(char *str)
{
    uint8_t data[24];
  
    int slen = min(11,strlen(str));
    int len = slen + 9;
    
    for (int i = 0; i < 24; i++) data[i] = 0x00;
    
    data[0] = 0x10;
    data[1] = len; //len
    data[2] = 0x05;
    data[3] = 0x26;
    data[4] = 0x01;
    data[5] = 0x00;
    data[6] = 0x01;
    data[7] = slen + 2; //len2
    
    data[8] =  0x21;
    data[9] =  0x10;
    
    data[16] = 0x22;
    
    for (int i = 0; i < min(6, slen); i++) data[10+i] = str[i];
    for (int i = 0; i < min(11, slen) - 6; i++) data[17+i] = str[i+6];
    
    uint8_t hash = 0xCA - 2;
    for (int i = 0; i < len; i++) hash -= i;
    
    for (int i = 10; i < 16; i++) hash -= data[i];
    for (int i = 17; i < 23; i++) hash -= data[i];
    
    data[(len > 13) ? (len + 3) : (len + 2)] = hash;

    sendCanMessages(420, data, 3, false);
}

void ic_music_top_line2(char *str) 
{
  uint8_t data[16];
  data[0] = 0x03;
  data[1] = 0x29;
  data[2] = 0x00;
  
}

void ic_music_top_line(char *str) 
{
  //r 420 8 10 08 03 29 00 46 4d 35;r 420 8 21 00 f6 30 32 2e 38 20 
    uint8_t data[16];
  
    int slen = min(8,strlen(str));
    int len = strlen(str) + 5;

    for (int i = 0; i < 16; i++) data[i] = 0x00;

    data[0] = 0x10;
    
    data[1] = len; //len
    data[2] = 0x03;
    data[3] = 0x29;
    data[4] = 0x00;
    
    data[8] =  0x21;

    for (int i = 0; i < min(3, slen); i++) data[5+i] = str[i];
    for (int i = 0; i < min(8, slen); i++) data[9+i] = str[i+3];
    
    uint8_t hash = 0xd3;
    for (int i = 5; i < 8; i++) hash -= data[i];
    for (int i = 9; i < 14; i++) hash -= data[i];
    
    for (int i = 0; i < len - 1; i++) hash -= i;
    
    data[len + 2] = hash;

    sendCanMessages(420, data, 2, true);
}



void ic_package24(uint8_t val1, uint8_t val2, uint8_t val3, uint8_t val4, uint8_t val5, uint8_t val6)
{
    //t24 2 0 11 0 0 10 - corvice data
    //t24 2 0 11 2 1 10 - iUDIO,  AUa

  
  //  03  24 05 40 01 01 00 00 00 11 01 00 00 00 24
  //  03  24 02 00 01 01 00 00 00 12 [09 01] [0a 02] [00(c/l/r) (T O P) 00] 3d
//        24 01 82 11 03 00 01 00 10 50 53 65 72 76 69 63 65 20 64 61 74 61 00 31 - Service data
//        24 01 90 11 04 00 01 00 10 50 53 65 72 76 69 63 65 20 6d 65 6e 75 00 07 - Service menu
//           v1 v2 v3 v4    v5     v6


//13 >>> 24($) 1() 92(�) 11() 3() 0( ) 1() 0( ) 10() 50(P) 43(C) 6F(o) 6E(n) 66(f) 69(i) 72(r) 6D(m) 61(a) 74(t) 69(i) 6F(o) 6E(n) 0( ) C3(�) 


//13 >>> 24($) 1() 92(�) 11() 3() 0( ) 1() 0( ) 10() 50(P) 43(C) 6F(o) 6E(n) 66(f) 69(i) 72(r) 6D(m) 61(a) 74(t) 69(i) 6F(o) 6E(n) 0( ) C3(�) 

//Service menu
//13 >>> 22(") 6() 
//13 >>> 21(!) 6() 
//13 >>> 24($) 1() 90(�) 11() 4() 0( ) 1() 0( ) 10() 50(P) 53(S) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 20( ) 6D(m) 65(e) 6E(n) 75(u) 0( ) 7() 
//13 >>> 25(%) 6() 
//13 >>> 26(&) 1() 0( ) 4() E() 2() 53(S) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 20( ) 64(d) 61(a) 74(t) 61(a) 7() 0( ) 49(I) 74(t) 65(e) 6D(m) 73(s) D(
//) 0( ) 41(A) 64(d) 64(d) 69(i) 74(t) 2E(.) 20( ) 57(W) 6F(o) 72(r) 6B(k) E() 0( ) 43(C) 6F(o) 6E(n) 66(f) 69(i) 72(r) 6D(m) 61(a) 74(t) 69(i) 6F(o) 6E(n) 0( ) AB(�) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 0( ) 

/*
//service data
13 >>> 24($) 1() 82(�) 11() 3() 0( ) 1() 0( ) 10() 50(P) 53(S) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 20( ) 64(d) 61(a) 74(t) 61(a) 0( ) 31(1) 
13 <<< 24($) 6() 
13 <<< 25(%) 1() 0( ) 3() B9(�) 
13 >>> 25(%) 6() 
13 >>> 26(&) 1() 0( ) 3() 5() 2() 33(3) 42(B) 45(E) 13() 0( ) 6B(k) 6D(m) 20( ) 72(r) 65(e) 6D(m) 61(a) 69(i) 6E(n) 2E(.) 20( ) 2B(+) 31(1) 38(8) 34(4) 36(6) 30(0) 13() 0( ) 44(D) 61(a) 79(y) 73(s) 20( ) 72(r) 65(e) 6D(m) 61(a) 69(i) 6E(n) 2E(.) 20( ) 2B(+) 37(7) 30(0) 33(3) 0( ) 13() 
13 <<< 26(&) 6() 


//items
13 >>> 24($) 1() 82(�) 11() 5() 0( ) 1() 0( ) 10() 50(P) 49(I) 74(t) 65(e) 6D(m) 73(s) 0( ) 44(D) 
13 <<< 24($) 6() 
13 <<< 25(%) 1() 0( ) 5() B7(�) 
13 >>> 25(%) 6() 
13 >>> 26(&) 1() 0( ) 4() B() 2() 53(S) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 20( ) 31(1) 13() 0( ) 45(E) 78(x) 68(h) 2E(.) 2D(-) 67(g) 61(a) 73(s) 20( ) 61(a) 6E(n) 61(a) 6C(l) 79(y) 73(s) 69(i) 73(s) 14() 0( ) 47(G) 65(e) 6E(n) 65(e) 72(r) 61(a) 6C(l) 20( ) 49(I) 6E(n) 73(s) 70(p) 65(e) 63(c) 74(t) 69(i) 6F(o) 6E(n) C() 0( ) 53(S) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 20( ) 31(1) 33(3) 0( ) EB(�) 
13 <<< 25(%) 5() 0( ) 1() B7(�) 
13 >>> 25(%) 6() 
13 >>> 26(&) 5() 0( ) 1() C() 0( ) 53(S) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 20( ) 31(1) 34(4) 0( ) C5(�) 
13 <<< 26(&) 6() 

//items
13 >>> 24($) 1() 82(�) 11() 5() 0( ) 1() 0( ) 10() 50(P) 49(I) 74(t) 65(e) 6D(m) 73(s) 0( ) 44(D) 
13 <<< 24($) 6() 
13 <<< 25(%) 1() 0( ) 5() B7(�) 
13 >>> 25(%) 6() 
13 >>> 26(&) 1() 0( ) 4() B() 2() 53(S) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 20( ) 31(1) 13() 0( ) 45(E) 78(x) 68(h) 2E(.) 2D(-) 67(g) 61(a) 73(s) 20( ) 61(a) 6E(n) 61(a) 6C(l) 79(y) 73(s) 69(i) 73(s) 14() 0( ) 47(G) 65(e) 6E(n) 65(e) 72(r) 61(a) 6C(l) 20( ) 49(I) 6E(n) 73(s) 70(p) 65(e) 63(c) 74(t) 69(i) 6F(o) 6E(n) C() 0( ) 53(S) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 20( ) 31(1) 33(3) 0( ) EB(�) 
13 <<< 25(%) 5() 0( ) 1() B7(�) 
13 >>> 25(%) 6() 
13 >>> 26(&) 5() 0( ) 1() C() 0( ) 53(S) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 20( ) 31(1) 34(4) 0( ) C5(�) 
13 <<< 26(&) 6() 

//main view
13 >>> 24($) 1() 90(�) 11() 4() 0( ) 1() 0( ) 10() 50(P) 53(S) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 20( ) 6D(m) 65(e) 6E(n) 75(u) 0( ) 7() 
13 <<< 24($) 6() 
13 <<< 25(%) 1() 0( ) 4() B8(�) 
13 >>> 25(%) 6() 
13 >>> 26(&) 1() 0( ) 4() E() 2() 53(S) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 20( ) 64(d) 61(a) 74(t) 61(a) 7() 0( ) 49(I) 74(t) 65(e) 6D(m) 73(s) D(
) 0( ) 41(A) 64(d) 64(d) 69(i) 74(t) 2E(.) 20( ) 57(W) 6F(o) 72(r) 6B(k) E() 0( ) 43(C) 6F(o) 6E(n) 66(f) 69(i) 72(r) 6D(m) 61(a) 74(t) 69(i) 6F(o) 6E(n) 0( ) AB(�) 

//addit. Work
13 >>> 24($) 1() 92(�) 11() B() 0( ) 1() 0( ) 10() 50(P) 41(A) 64(d) 64(d) 69(i) 74(t) 2E(.) 20( ) 57(W) 6F(o) 72(r) 6B(k) 0( ) E4(�) 
13 <<< 24($) 6() 
13 <<< 25(%) 1() 0( ) 7() B5(�) 
13 >>> 25(%) 6() 
13 >>> 26(&) 1() 0( ) 4() B() 2() 53(S) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 20( ) 31(1) B() 0( ) 53(S) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 20( ) 33(3) B() 0( ) 53(S) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 20( ) 35(5) B() 0( ) 53(S) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 20( ) 36(6) 0( ) 37(7) 
13 <<< 26(&) 6() 
13 <<< 25(%) 5() 0( ) 3() B5(�) 
13 >>> 25(%) 6() 
13 >>> 26(&) 5() 0( ) 3() 13() 0( ) 45(E) 78(x) 68(h) 2E(.) 2D(-) 67(g) 61(a) 73(s) 20( ) 61(a) 6E(n) 61(a) 6C(l) 79(y) 73(s) 69(i) 73(s) 14() 0( ) 47(G) 65(e) 6E(n) 65(e) 72(r) 61(a) 6C(l) 20( ) 49(I) 6E(n) 73(s) 70(p) 65(e) 63(c) 74(t) 69(i) 6F(o) 6E(n) B() 0( ) 53(S) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 20( ) 39(9) 0( ) 25(%) 
13 <<< 26(&) 6() 

//error and back
13 <<< 22(") C9(�) 
13 >>> 22(") 6() 
13 <<< 21(!) 1() 1() C3(�) 
13 >>> 21(!) 6() 
13 <<< 4() E7(�) 
13 >>> 4() E7(�) 
13 <<< 24($) 6() 
13 >>> 4() 6() 
13 <<< 25(%) 1() 0( ) 4() B8(�) 
13 >>> 25(%) 6() 
13 >>> 1() 0( ) E8(�) 
13 <<< 1() 6() 
13 <<< 9(  ) 0( ) E0(�) 
13 >>> 9( ) 6() 
13 >>> 26(&) 1() 0( ) 4() E() 2() 53(S) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 20( ) 64(d) 61(a) 74(t) 61(a) 7() 0( ) 49(I) 74(t) 65(e) 6D(m) 73(s) D(
) 0( ) 41(A) 64(d) 64(d) 69(i) 74(t) 2E(.) 20( ) 57(W) 6F(o) 72(r) 6B(k) E() 0( ) 43(C) 6F(o) 6E(n) 66(f) 69(i) 72(r) 6D(m) 61(a) 74(t) 69(i) 6F(o) 6E(n) 0( ) AB(�) 



//hidding
13 <<< 22(") C9(�) 
13 >>> 22(") 6() 
13 <<< 4() E7(�) 
262016 7C1 6 00 00 04 00 00 04  
13 >>> 4() 6() 
13 <<< 21(!) 1() 1() C3(�) 
13 >>> 21(!) 6() 
262215 7C1 6 00 00 00 00 00 04  
13 >>> 24($) 1() 90(�) 11() 4() 0( ) 1() 0( ) 10() 50(P) 53(S) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 20( ) 6D(m) 65(e) 6E(n) 75(u) 0( ) 7() 
13 <<< 24($) 6() 
13 >>> 1() 0( ) E8(�) 
13 <<< 25(%) 1() 0( ) 4() B8(�) 
262416 7C1 6 00 00 00 00 00 04  
13 <<< 1() 6() 
13 >>> 25(%) 6() 
13 <<< 9(  ) 0( ) E0(�) 
13 >>> 9( ) 6() 
262616 7C1 6 00 00 00 00 00 04  
13 >>> 26(&) 1() 0( ) 4() E() 2() 53(S) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 20( ) 64(d) 61(a) 74(t) 61(a) 7() 0( ) 49(I) 74(t) 65(e) 6D(m) 73(s) D(\n) 0( ) 41(A) 64(d) 64(d) 69(i) 74(t) 2E(.) 20( ) 57(W) 6F(o) 72(r) 6B(k) E() 0( ) 43(C) 6F(o) 6E(n) 66(f) 69(i) 72(r) 6D(m) 61(a) 74(t) 69(i) 6F(o) 6E(n) 0( ) AB(�) 

//confirmation
13 >>> 24($) 1() 92(�) 11() 3() 0( ) 1() 0( ) 10() 50(P) 43(C) 6F(o) 6E(n) 66(f) 69(i) 72(r) 6D(m) 61(a) 74(t) 69(i) 6F(o) 6E(n) 0( ) C3(�) 
13 <<< 24($) 6() 
13 <<< 25(%) 1() 0( ) 3() B9(�) 
13 >>> 25(%) 6() 
13 >>> 26(&) 1() 0( ) 3() E() 2() 46(F) 75(u) 6C(l) 6C(l) 20( ) 73(s) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 1B() 0( ) 43(C) 61(a) 6E(n) 63(c) 65(e) 6C(l) 20( ) 66(f) 75(u) 6C(l) 6C(l) 20( ) 20( ) 20( ) 20( ) 20( ) 20( ) 20( ) 73(s) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 19() 0( ) 43(C) 61(a) 6E(n) 63(c) 65(e) 6C(l) 20( ) 73(s) 69(i) 6E(n) 67(g) 6C(l) 65(e) 20( ) 20( ) 20( ) 20( ) 20( ) 69(i) 74(t) 65(e) 6D(m) 73(s) 0( ) 4A(J) 1() 
13 <<< 4() E7(�) 
13 >>> 4() 6() 
13 >>> 1() 0( ) E8(�) 
13 <<< 1() 6() 
13 <<< 9(  ) 0( ) E0(�) 
13 >>> 9( ) 6() 




//fm
3 >>> 29()) 0( ) 46(F) 4D(M) 33(3) 0( ) F8(�) 
3 <<< 29()) 6() 
3 >>> 24($) 2() 0( ) 1() 1() 0( ) 0( ) 0( ) 12() 9(  ) 1() A(\n) 2() 0( ) 46(F) 4D(M) 33(3) 0( ) 3B(;) 
3 <<< 24($) 6() 
3 <<< 25(%) 1() 0( ) 1() CB(�) 
3 >>> 25(%) 6() 
3 >>> 26(&) 1() 0( ) 1() C() 10() 31(1) 30(0) 32(2) 2E(.) 31(1) 20( ) 4D(M) 48(H) 7A(z) 0( ) 0( ) FE(�) 
3 <<< 26(&) 6() 
3 >>> 29()) 0( ) 46(F) 4D(M) 34(4) 0( ) F7(�) 
3 <<< 29()) 6() 
3 >>> 28(() 2() 0( ) 1() 0( ) 2() BA(�) 
3 <<< 28(() 6() 
3 >>> 24($) 2() 0( ) 1() 1() 0( ) 0( ) 0( ) 12() 9( ) 1() A(\n) 2() 0( ) 46(F) 4D(M) 34(4) 0( ) 3A(:) 
3 <<< 24($) 6() 
3 <<< 25(%) 1() 0( ) 1() CB(�) 
3 >>> 25(%) 6() 
3 >>> 26(&) 1() 0( ) 1() C() 10() 31(1) 30(0) 32(2) 2E(.) 38(8) 20( ) 4D(M) 48(H) 7A(z) 0( ) 0( ) F7(�) 
3 <<< 26(&) 6() 

//        24 01 90 11 04 00 01 00 10 50 53 65 72 76 69 63 65 20 6d 65 6e 75 00 07 - Service menu
//           v1 v2 v3 v4    v5     v6

       24    5    40     01   01  00  00  00  11   01   00      00 00
5  >>> 24($) 2() 0( )    1() 2()  0( ) 0( ) 0( ) 13() 0( ) 1() 0( ) 2() 0( ) 3() 0( ) 54(T) 45(E) 4C(L) 0( ) 1() 
3  >>> 24($) 2()  0( )    1() 1()  0( ) 0( ) 0( ) 12() 9(  )  1()   A(\n) 2()  0( )  46(F) 4D(M) 33(3) 0( )  3B(;) //fm
13 >>> 24($) 1() 90(�) 11() 4() 0( ) 1() 0( ) 10() 50(P) 53(S) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 20( ) 6D(m) 65(e) 6E(n) 75(u) 0( )    7() //service menu
13 >>> 24($) 1() 82(�) 11() 5() 0( ) 1() 0( ) 10() 50(P) 49(I) 74(t) 65(e) 6D(m) 73(s) 0( ) 44(D)                                             //items
13 >>> 24($) 1() 82(�) 11() 3() 0( ) 1() 0( ) 10() 50(P) 53(S) 65(e) 72(r) 76(v) 69(i) 63(c) 65(e) 20( ) 64(d) 61(a) 74(t) 61(a) 0( ) 31(1)   //service data
13 >>> 24($) 1() 92(�) 11() B() 0( ) 1() 0( ) 10() 50(P) 41(A) 64(d) 64(d) 69(i) 74(t) 2E(.) 20( ) 57(W) 6F(o) 72(r) 6B(k)   0( ) E4(�)      //additional work many items
13 >>> 24($) 1() 92(�) 11() 3() 0( ) 1() 0( ) 10() 50(P) 43(C) 6F(o) 6E(n) 66(f) 69(i) 72(r) 6D(m) 61(a) 74(t) 69(i) 6F(o) 6E(n) 0( ) C3(�)  //confirmation 3 items
              v1 v2     v3   v4   v5  v6      
1 

t24 2 0 1 2 0 13
t24 2 0 1 1 0 10 - works but doesnt update
t24 2 0 1 2 0 10


t26
t24 2 0 1 2 0 10
kwp 17 5 24 1 82 11 5 0 1 0 10 50 49 74 65 6d 73 0
kwp 24($) 1() 82(�) 11() 5() 0( ) 1() 0( ) 10() 50(P) 49(I) 74(t) 65(e) 6D(m) 73(s) 0( ) 44(D) 

kwp 17 5 24 1 82 11 5 0 1 0 10 50 49 74 65 6d 73 0
kwp 17 5 24 2 0 1 1 0 1 0 10 50 49 74 65 6d 73 0 - big
kwp 17 5 24 1 0 1 1 0 1 0 10 50 49 74 65 6d 73 0 - works?


kwp 19 5 24 2 0 1 2 0 0 0 13 0 1 0 2 0 3 0 54 45 4c 4c 4c 4c
kwp 19 5 24 2 0 1 1 0 0 0 12 9 1 a 2 10 54 45 4c 4c 4c 4c - with buttons

kwp 19 5 24 1 40 11 02 00 0 0 11 1 0 0 0
kwp 19 5 24 1 40 11 02 00 0 0 11 1 0 0 0

kwp 19 5 24 2 0 11 01 00 0 0 11 1 30 30 0
kwp 19 5 24 2 0 11 01 00 0 0 10 50 49 74 65 6d 73 0
kwp 19 5 24 2 0 11 2 0 0 0 12 9 0 9 1 0 49 74 65 6d 73 73 73
kwp 19 5 24 1 0 11 2 0 0 0 12 9 0 9 1 0 49 74 65 6d 73 73 73
kwp 19 5 24 2 0 11 3 0 0 0 12 9 0 9 1 0 49 74 65 6d 73 73 73
                    [1   1] - crashes
                    [X] - strange, draws empty pixels on first line
kwp 19 5 24 2 0 11 3 1 0 0 12 9 0 9 1 0 49 74 65 6d 73 73 73

*/

    uint8_t len = 21 - 6;
    uint8_t data[len+1] = {0x05, 0x24, rand()%255, 0, 0x11, 3, 0, 0, 0, 0x10, 0x10, 't', 'e', 's', 0};
    
    diag_send_kwp(data, len, true);
}








void ic_package26(char *line1, char *line2, char *line3, char *line4)
{

  // 05 26 01 00 02 08     00 69 55 44 49 4f 00   08 00 41 55 09 -- --
  // 05 26 01 00 02 05     10 62 6c 61 00 10 62   6c 61 00 d6 00 25 bb  
  // 05 26 01 00 02 08     10 62 6c 61 62 6c 00   10 62 6c 61 62 6c 00  f1 16 2b 07 16 2b 07 

  // 05 26 01 00 02 08     10 62 6c 61 62 6c 00   07 10 62 6c 61 62 6c  ea 30 ff 07 8d 00 05  

  // 05 26 01 00 02 04     00 31 00 03 00 32 00   19 16 00 00 08 37 00 

  // 05 26 01 00 03 05     00 6c 32 00 04 00 6c   33 04 00 6c 34 00 3b 

  // 05 26 01 00 04 04     00 31 00 03 00 32 03   00 33 03 00 33 00 4e


  
  uint8_t slen1 = strlen(line1);
  uint8_t slen2 = strlen(line2);
  uint8_t slen3 = strlen(line3);
  uint8_t slen4 = strlen(line4);
  uint8_t count = 1;
  uint8_t len = 6 + slen1 + 3;
  if (slen2 > 0) {
    len += slen2 + 2;
    count++;
  }
  if (slen3 > 0) {
    len += slen3 + 2;
    count++;
  }
  if (slen4 > 0) {
    len += slen4 + 2;
    count++;
  }
  uint8_t data[len+1] = {0x05, 0x26, 0x01, 0x00, count, slen1 + 3};
  data[6] = 0x00;       //format (0x10 - center aligment)
  memcpy(data + 7, line1, slen1);

  if (slen2 > 0) {
    data[6 + slen1 + 2] = slen2 + 2;
    data[6 + slen1 + 3] = 0x00;
    memcpy(data + 7 + slen1 + 3, line2, slen2);
  }

  if (slen3 > 0) {
    data[6 + slen1 + 2 + slen2 + 2] = slen3 + 2;
    data[6 + slen1 + 3 + slen2 + 2] = 0x00;
    memcpy(data + 7 + slen1 + 3 + slen2 + 2, line3, slen3);
  }

  if (slen4 > 0) {
      data[6 + slen1 + 2 + slen2 + 2 + slen3 + 2] = slen4 + 2;
      data[6 + slen1 + 3 + slen2 + 2 + slen3 + 2] = 0x00;
      memcpy(data + 7 + slen1 + 3 + slen2 + 2 + slen3 + 2, line4, slen4);
  }


  
  diag_send_kwp(data, len, true);

  ic.data26_len = len;
  for (int i = 0; i < len; i++) ic.data26[i] = data[i];
}









void ic_music_show_arrows(uint8_t up, uint8_t down)
{
    //uint8_t hash = 0xb1 + 0x09;
    //hash -= up;
    //hash -= down;

    uint8_t len = 5;
    uint8_t data[len+1] = {0x03, 0x28, 0x01, up, 0x01};

    diag_send_kwp(data, len, true);
    
    /*
    uint8_t hash2 = 0xff;
    for (int i = 0; i < len; i++) hash2 -= (i + data[i]);
    data[len] = hash2;

    uint8_t output2[48];
    uint8_t output_len;
    diag_incapsulate(data, len+1, output2, &output_len);
    sendCanMessages(420, output2, output_len/8, true);
    */
    //0x09 - up or down arrow
    
    //uint8_t data2[16] = {0x10, 0x08, 0x03, 0x28, 0x02, up, 0x01, down,
    //                    0x21, 0x02, hash, 0x00, 0x00, 0x00, 0x00, 0x00};
    //sendCanMessages(420, data2, 2, true);


    //for (int i = 0; i < len-1; i++) hash -= i;
    //for (int i = 0; i < slen+7; i++) hash -= data2[i];
}
