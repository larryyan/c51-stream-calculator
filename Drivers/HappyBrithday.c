#include <regx52.h>
#include "Timer1.h"
#include "Delay.h"
#include "Buzzer.h"

unsigned char code ToneTH[3][8] = {
    // ??? 1-7 (G4 - F#5)
    {0, 0xFB, 0xFB, 0xFC, 0xFC, 0xFC, 0xFD, 0xFD}, 
    // ??? 1-7 (G5 - F#6)
    {0, 0xFD, 0xFD, 0xFE, 0xFE, 0xFE, 0xFE, 0xFF}  
};

unsigned char code ToneTL[3][8] = {
    // ??? 1-7
    {0, 0x04, 0x90, 0x0C, 0x44, 0xAD, 0x0A, 0x5C}, 
    // ??? 1-7
    {0, 0x82, 0xC8, 0x06, 0x22, 0x56, 0x00, 0x00}  
};

// ==========================================
//  Music Note
// ==========================================
unsigned char code Music[] = {
    05, 05, 06, 05, 11, 07,      // ??????
    05, 05, 06, 05, 12, 11,      // ??????
    05, 05, 15, 13, 11, 07, 06,  // ?????? (??5,3,1)
    14, 14, 13, 11, 12, 11       // ?????? (??4)
};

unsigned char code Duration[] = {
    3, 1, 4, 4, 4, 8,
    3, 1, 4, 4, 4, 8,
    3, 1, 4, 4, 4, 4, 8,
    3, 1, 4, 4, 4, 12
};

unsigned char Level, Note;

sbit buzzer = P2 ^ 4; 

void Timer1_Isr(void) interrupt 3
{
	TL1 = ToneTL[Level][Note];
    TH1 = ToneTH[Level][Note];
	
	buzzer = !buzzer;
}

/**
 * @brief Play a tone on the buzzer
 * 
 * @param level The tone level
 * @param note The tone note
 * @param duration The duration to play the tone (in milliseconds)
 * @return void
 */
void PlayTone(unsigned char level, unsigned char note, unsigned char duration) {
    Level = level;
    Note = note;
    TR1 = 1;
    
    Delay(duration);
    
    TR1 = 0;
    Delay(20); // Pause between tones
}

void HappyBrithday() {
    unsigned char i;
    for(i = 0; i < sizeof(Music); i ++)
        PlayTone(Music[i] / 10, Music[i] % 10, 125 * Duration[i]);
}