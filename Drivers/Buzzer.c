#include <regx52.h>
#include "Buzzer.h"
#include "Delay.h"
#include "Timer1.h"
#include "HappyBrithday.h"

#define BUZZER_TONE_LEVELS   3
#define BUZZER_TONE_NOTES    8
#define BUZZER_TONE_PALETTE  4
#define BUZZER_KEY_COUNT     24

typedef struct {
	unsigned char level;
	unsigned char note;
	unsigned char duration;
} BuzzerToneSpec;

extern const unsigned char code ToneTH[BUZZER_TONE_LEVELS][BUZZER_TONE_NOTES];
extern const unsigned char code ToneTL[BUZZER_TONE_LEVELS][BUZZER_TONE_NOTES];

static volatile unsigned char toneLevel;
static volatile unsigned char toneNote;
static void Buzzer_PlayTone(unsigned char paletteIndex);

static const BuzzerToneSpec code TonePalette[BUZZER_TONE_PALETTE] = {
	{0, 3, 50},
	{0, 5, 50},
	{0, 6, 50},
	{1, 3, 100}
};  // level, note, duration(ms)

static const char code KeyToneMap[BUZZER_KEY_COUNT] = {
	0, 0, 0, 1,
	0, 0, 0, 1,
	0, 0, 0, 1,
	0, 0, 0, 1,
	1, 1, 1, 2,
	3, 3,-1, 3
};

/**
 * @brief  初始化蜂鸣器
 * @param  无
 * @return 无
 */
void Buzzer_Init(void)
{
	Timer1_Init();
	TR1 = 0;
}

/**
 * @brief  播放指定音调
 * @param  paletteIndex 音调在 TonePalette 中的索引
 * @return 无
 */
void Buzzer_KeySound(int keyNumber)
{
	if(keyNumber < 0 || keyNumber >= BUZZER_KEY_COUNT || KeyToneMap[keyNumber] < 0)
		return;

	PlayTone(
        TonePalette[KeyToneMap[keyNumber]].level,
        TonePalette[KeyToneMap[keyNumber]].note,
        TonePalette[KeyToneMap[keyNumber]].duration
    );
}
