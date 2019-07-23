
#include <WS2812B.h>
#include <FastLED.h>
#include "geometry.hpp"
#include "animation.hpp"
#include "appState.hpp"

#define NUM_LEDS 300
#define NUM_ANIMATIONS 30

WS2812B strip = WS2812B(NUM_LEDS);
Position::Axis a[32] = {
	Position::W, Position::Z, Position::W, Position::Z, Position::X, Position::W, Position::X, Position::Y,
	Position::X, Position::Y, Position::Z, Position::W, Position::Z, Position::Y, Position::W, Position::Z,
	Position::Y, Position::X, Position::Y, Position::X, Position::W, Position::Y, Position::X, Position::Y,
	Position::X, Position::Z, Position::X, Position::Z, Position::W, Position::Z, Position::W, Position::Y};

CRGB leds[NUM_LEDS];

Tesseract t(a, leds, NUM_LEDS);
AniParticle<Tesseract> particle;
AniVertexFill vFill;
AniFillOutIn inOut;
AniWipe wipe;
AniPlaneRotate<Tesseract> planeRotate;
AniRainbow<Tesseract> rainbow;
AniRainbowFill<Tesseract> rainbowFill;
AniBuildup<Tesseract> buildUp;
AniTheater theater;
AniSolidCube cube;
AniTargeting target;
AniClimb climb;
AniSquare square;
unsigned long curTime;
unsigned long lastSwapTime;

AppState state;

typedef void (*AnimationFunc)(void);
enum AnimationMode
{
	MODE_NORMAL = 0,
	MODE_SKIP,
	MODE_EXTEND
};

struct AnimationList
{
	int count;
	AnimationFunc animation[NUM_ANIMATIONS];
	enum AnimationMode mode[NUM_ANIMATIONS];
};
AnimationList aniList;
// void sendCommand(const char *cmd)
// {
// 	Serial1.print(cmd);
// 	delay(500);
// 	while (Serial1.available())
// 	{
// 		Serial.write(Serial1.read());
// 	}
// }

void parseCommand()
{
	static char cmd[128];
	static int i = 0;
	bool cmdValid = false;
	while (Serial1.available())
	{
		cmd[i] = Serial1.read();
		if (cmd[i] == '@')
		{
			cmd[i] = 0;
			cmdValid = true;
			i = 0;
			break;
		}
		i++;
	}

	if (cmdValid)
	{
		Serial.println(cmd);
		if (cmd[0] == 'r')
			state.Reset();
		else
			state.Update(cmd);
	}
}

void registerAnimation(AnimationFunc ani, enum AnimationMode mode)
{
	aniList.animation[aniList.count] = ani;
	aniList.mode[aniList.count] = mode;
	aniList.count++;
}

void aniParticleRainbow()
{
	state.ApplyState(&particle, 1.0f, CRGB::Black);
	particle.Update(curTime, state.pause);
	if (particle.speed < 0)
		t.Clear();
	particle.colorCycle = true;
	particle.streakSize = 15;
	for (int i = 0; i < 8; i++)
	{
		particle.hue = 255 * i / 8;
		particle.startEdge = i;
		particle.Draw(&t);
	}
}

void aniParticleCycle()
{
	unsigned char colors[3] = {HUE_RED, HUE_BLUE, HUE_PURPLE};
	state.ApplyState(&particle, 1.5f, CRGB::Black);
	particle.Update(curTime, state.pause);
	if (particle.speed < 0)
		t.Clear();

	particle.colorCycle = true;
	particle.streakSize = 60;
	for (int i = 0; i < 3; i++)
	{
		particle.hue = colors[i];
		particle.startEdge = i * 2;
		particle.Draw(&t);
	}
}

void aniPlaneRotate()
{
	unsigned char hue = HUE_GREEN;
	CRGB colors[3];
	t.Clear();
	state.ApplyState(&planeRotate, 1.5f, CHSV(hue, 255, 100));

	colors[0] = CHSV(HUE_ORANGE, 255, 100);
	colors[2] = CHSV(HUE_AQUA, 255, 100);
	colors[1] = CHSV(HUE_PURPLE, 255, 100);
	planeRotate.Update(curTime, state.pause);
	planeRotate.progessOffset = 0;
	planeRotate.planeOffset = 0.4;
	planeRotate.planeThickness = 2;
	planeRotate.planeNormal[0] = 1;
	planeRotate.planeNormal[1] = 0;
	planeRotate.planeNormal[2] = 0;
	planeRotate.color = colors[0];
	planeRotate.Draw(&t);

	planeRotate.progessOffset = 1000;
	planeRotate.color = colors[1];
	planeRotate.Draw(&t);

	planeRotate.progessOffset = 0;
	planeRotate.planeOffset = -0.4;
	planeRotate.planeThickness = 0.8;
	planeRotate.color = colors[2];
	planeRotate.Draw(&t);
}

void aniVertexFillNoClear()
{
	static int lastProgress = 0;
	static unsigned char hue = HUE_GREEN;
	state.ApplyState(&vFill, 1.5f, CHSV(hue, 255, 100));
	vFill.sink = 0;
	vFill.Update(curTime, state.pause);
	int progress = vFill.Progress();
	if ((vFill.speed > 0 && progress < lastProgress) || (vFill.speed < 0 && progress > lastProgress))
	{
		vFill.startVertex = rand() % 16;
		hue += 128 + 16;
	}
	lastProgress = progress;
	vFill.Draw(&t);
}

void aniVertexFill()
{
	t.Clear();
	state.ApplyState(&vFill, 1.0f, CHSV(HUE_GREEN, 255, 100));
	vFill.startVertex = 0;
	vFill.sink = 0;
	vFill.Update(curTime, state.pause);
	vFill.Draw(&t);
}

void aniVertexSink()
{
	static int lastProgress = 0;
	static unsigned char hue = HUE_GREEN;
	t.Clear();
	state.ApplyState(&vFill, 1.5f, CHSV(hue, 255, 100));
	vFill.Update(curTime, state.pause);
	int progress = vFill.Progress();
	if ((vFill.speed > 0 && progress < lastProgress) || (vFill.speed < 0 && progress > lastProgress))
	{
		vFill.sink = !vFill.sink;
		vFill.startVertex = rand() % 16;
		if (!vFill.sink)
			hue += 128 + 16;
	}
	lastProgress = progress;
	vFill.Draw(&t);
}

void aniInOut()
{
	t.Clear();
	state.ApplyState(&inOut, 1.0f, CRGB::Black);
	inOut.Update(curTime, state.pause);
	inOut.Draw(&t);
}

void aniConfetti()
{
	static int i = 0;
	fadeToBlackBy(leds, NUM_LEDS, 10);
	int pos = random16(NUM_LEDS);
	leds[pos] += CHSV(i / 5 + random8(64), 200, 255);
	pos = random16(NUM_LEDS);
	leds[pos] += CHSV(i / 5 + random8(64), 200, 255);
	i++;
}

void aniTheater()
{
	t.Clear();
	static int hue = 0;
	state.ApplyState(&theater, 1.0f, CHSV(hue / 20, 140, 30));
	theater.Update(curTime, state.pause);
	theater.Draw(&t);
	hue++;
}

void aniCubeFlash()
{
	t.Clear();
	state.ApplyState(&cube, 3.0f, CRGB(0, 128, 0));
	cube.Update(curTime, state.pause);
	cube.Draw(&t);
}

void staticOuter()
{
	CRGB color = state.colorLock ? (CRGB)state.color : CHSV(HUE_BLUE, 255, 100);
	t.Clear();
	SolidCube(&t, -1, color);
}

void staticInner()
{
	CRGB color = state.colorLock ? (CRGB)state.color : CHSV(HUE_RED, 255, 100);
	t.Clear();
	SolidCube(&t, 1, color);
}

void staticDiag()
{
	CRGB color = state.colorLock ? (CRGB)state.color : CHSV(HUE_GREEN, 255, 100);
	t.Clear();
	SolidCube(&t, 0, color);
}

void staticX()
{
	CRGB color = state.colorLock ? (CRGB)state.color : CHSV(HUE_ORANGE, 255, 100);
	t.Clear();
	Parallel(&t, Position::Axis::X, color);
}
void staticY()
{
	CRGB color = state.colorLock ? (CRGB)state.color : CHSV(HUE_AQUA, 255, 100);
	t.Clear();
	Parallel(&t, Position::Axis::Y, color);
}
void staticZ()
{
	CRGB color = state.colorLock ? (CRGB)state.color : CHSV(HUE_PURPLE, 255, 100);
	t.Clear();
	Parallel(&t, Position::Axis::Z, color);
}

void staticCube()
{
	CRGB color = state.colorLock ? (CRGB)state.color : CHSV(HUE_YELLOW, 255, 50);
	SolidCube(&t, 1, color);
	SolidCube(&t, -1, color);
	SolidCube(&t, 0, color);
}

void aniTarget()
{
	t.Clear();
	state.ApplyState(&target, 1.0f, CRGB::Red);
	target.Update(curTime, state.pause);
	target.Draw(&t);
}

void aniClimb()
{
	t.Clear();
	state.ApplyState(&climb, 1.0f, CRGB::Black);
	climb.Update(curTime, state.pause);
	climb.Draw(&t);
}

void aniRainbow()
{
	t.Clear();
	state.ApplyState(&rainbow, 1.0f, CRGB::Black);
	rainbow.mode = 0;
	rainbow.Update(curTime, state.pause);
	rainbow.Draw(&t);
}

void aniSparkle()
{
	state.ApplyState(&rainbow, 1.0f, CRGB::Black);
	rainbow.mode = 1;
	rainbow.Update(curTime, state.pause);
	rainbow.Draw(&t);
}

void aniRainbowFill()
{
	//t.Clear();
	static int hue = 0;
	state.ApplyState(&rainbowFill, 1.5f, CRGB::Black);
	fadeToBlackBy(leds, NUM_LEDS, 20 * rainbowFill.speed);
	rainbowFill.Update(curTime, state.pause);
	rainbowFill.progessOffset = 0;
	rainbowFill.hue = hue / 20;
	rainbowFill.Draw(&t);

	rainbowFill.progessOffset = 1000;
	// rainbowFill.hue = 150+128;
	rainbowFill.Draw(&t);
	hue++;
}

void aniBuildUp()
{
	t.Clear();
	state.ApplyState(&buildUp, 1.0f, CRGB::Black);
	buildUp.Update(curTime, state.pause);
	buildUp.Draw(&t);
}

void aniEdgeFlash()
{
	state.ApplyState(&buildUp, 1.0f, CRGB::Black);
	buildUp.Update(curTime, state.pause);
	buildUp.Draw(&t);
}

void aniWipe()
{
	static int lastProgress = 0;
	static unsigned char hue = HUE_GREEN;
	t.Clear();
	state.ApplyState(&wipe, 2.0f, CHSV(hue, 255, 100));
	wipe.Update(curTime, state.pause);
	int progress = wipe.Progress();
	if ((wipe.speed > 0 && progress < lastProgress) || (wipe.speed < 0 && progress > lastProgress))
	{
		hue += 144;
	}
	lastProgress = progress;
	wipe.invert = 0;
	wipe.Draw(&t);
	wipe.color = CHSV(hue + 111, 255, 100);
	wipe.invert = 1;
	wipe.Draw(&t);
}

static void weaveHelper2(Tesseract *t, AniSquare *sq, unsigned char color1, unsigned char color2)
{
	sq->hue = color1;
	sq->progessOffset = 0;
	sq->Draw(t);
	sq->hue = color2;
	sq->progessOffset = sq->duration / 2;
	sq->Draw(t);
}

static void weaveHelper3(Tesseract *t, AniSquare *sq, unsigned char color1, unsigned char color2, unsigned char color3)
{
	sq->hue = color1;
	sq->progessOffset = 0;
	sq->Draw(t);
	sq->hue = color2;
	sq->progessOffset = sq->duration / 3;
	sq->Draw(t);
	sq->hue = color3;
	sq->progessOffset = sq->duration * 2 / 3;
	sq->Draw(t);
}

static void weaveHelper4(Tesseract *t, AniSquare *sq, unsigned char color1, unsigned char color2)
{
	sq->hue = color1;
	sq->progessOffset = 0;
	sq->Draw(t);
	sq->progessOffset = sq->duration / 2;
	sq->Draw(t);
	sq->hue = color2;
	sq->progessOffset = sq->duration / 4;
	sq->Draw(t);
	sq->progessOffset = sq->duration * 3 / 4;
	sq->Draw(t);
}

void aniWeave()
{
	t.Clear();
	unsigned char outHue1 = HUE_GREEN, outHue2 = HUE_PURPLE, inHue1 = HUE_YELLOW, inHue2 = HUE_PURPLE;
	state.ApplyState(&square, 2.0f, CHSV(HUE_GREEN, 255, 100));
	square.Update(curTime, state.pause);

	square.streakSize = 10;
	square.maxOffset = t.outerSize * 4;

	square.turnAxis[0] = Position::Axis::Z;
	square.turnAxis[1] = Position::Axis::Y;
	square.startPos = Position(-1, -1, -1, -1);
	weaveHelper4(&t, &square, outHue1, outHue2);

	square.turnAxis[0] = Position::Axis::Y;
	square.turnAxis[1] = Position::Axis::Z;
	square.startPos = Position(1, 1, 1, -1);
	weaveHelper4(&t, &square, outHue2, outHue1);

	square.turnAxis[0] = Position::Axis::X;
	square.turnAxis[1] = Position::Axis::Y;
	square.startPos = Position(-1, -1, -1, -1);
	weaveHelper4(&t, &square, outHue1, outHue2);

	square.turnAxis[0] = Position::Axis::Y;
	square.turnAxis[1] = Position::Axis::X;
	square.startPos = Position(1, 1, 1, -1);
	weaveHelper4(&t, &square, outHue2, outHue1);
	////////////////////////////////////////////////

	square.streakSize = 5;
	square.maxOffset = t.innerSize * 4;
	square.turnAxis[0] = Position::Axis::Y;
	square.turnAxis[1] = Position::Axis::Z;
	square.startPos = Position(-1, -1, -1, 1);
	weaveHelper4(&t, &square, inHue1, inHue2);

	square.turnAxis[0] = Position::Axis::Z;
	square.turnAxis[1] = Position::Axis::Y;
	square.startPos = Position(1, 1, 1, 1);
	weaveHelper4(&t, &square, inHue2, inHue1);

	square.turnAxis[0] = Position::Axis::Y;
	square.turnAxis[1] = Position::Axis::X;
	square.startPos = Position(-1, -1, -1, 1);
	weaveHelper4(&t, &square, inHue1, inHue2);

	square.turnAxis[0] = Position::Axis::X;
	square.turnAxis[1] = Position::Axis::Y;
	square.startPos = Position(1, 1, 1, 1);
	weaveHelper4(&t, &square, inHue2, inHue1);
}

void aniCyclo()
{
	t.Clear();
	unsigned char outHue1 = HUE_RED, outHue2 = HUE_PURPLE, outHue3 = HUE_BLUE;
	state.ApplyState(&square, 2.0f, CHSV(HUE_GREEN, 255, 100));
	square.Update(curTime, state.pause);

	square.streakSize = 5;
	square.maxOffset = t.outerSize + t.innerSize + 2 * t.diagSize;

	square.turnAxis[0] = Position::Axis::W;
	square.turnAxis[1] = Position::Axis::Y;
	square.startPos = Position(-1, 1, -1, -1);
	weaveHelper3(&t, &square, outHue1, outHue2, outHue3);
	square.startPos = Position(-1, 1, 1, -1);
	weaveHelper3(&t, &square, outHue1, outHue2, outHue3);
	square.startPos = Position(1, 1, 1, -1);
	weaveHelper3(&t, &square, outHue1, outHue2, outHue3);
	square.startPos = Position(1, 1, -1, -1);
	weaveHelper3(&t, &square, outHue1, outHue2, outHue3);
}

void setup()
{
	Serial.begin(9600);
	Serial1.begin(9600);
	strip.begin(); // Sets up the SPI
	strip.show();  // Clears the strip, as by default the strip data is set to all LED's off.
	aniList.count = 0;
	for (int i = 0; i < NUM_ANIMATIONS; i++)
		aniList.animation[i] = NULL;

	registerAnimation(&aniParticleRainbow, MODE_EXTEND);
	registerAnimation(&aniParticleCycle, MODE_EXTEND);
	registerAnimation(&aniWipe, MODE_NORMAL);

	registerAnimation(&aniVertexFill, MODE_NORMAL);
	registerAnimation(&aniVertexFillNoClear, MODE_NORMAL);
	registerAnimation(&aniVertexSink, MODE_NORMAL);

	registerAnimation(&aniConfetti, MODE_EXTEND);
	registerAnimation(&aniInOut, MODE_NORMAL);
	registerAnimation(&aniPlaneRotate, MODE_NORMAL);

	registerAnimation(&aniTarget, MODE_NORMAL);
	registerAnimation(&aniClimb, MODE_SKIP);
	registerAnimation(&aniTheater, MODE_NORMAL);

	registerAnimation(&aniRainbow, MODE_NORMAL);
	registerAnimation(&aniSparkle, MODE_NORMAL);
	registerAnimation(&aniRainbowFill, MODE_EXTEND);

	registerAnimation(&aniCubeFlash, MODE_NORMAL);
	registerAnimation(&aniEdgeFlash, MODE_NORMAL);
	registerAnimation(&aniBuildUp, MODE_NORMAL);

	registerAnimation(&aniWeave, MODE_NORMAL);
	registerAnimation(&aniCyclo, MODE_NORMAL);
	registerAnimation(&staticCube, MODE_SKIP);

	registerAnimation(&staticOuter, MODE_SKIP);
	registerAnimation(&staticInner, MODE_SKIP);
	registerAnimation(&staticDiag, MODE_SKIP);

	registerAnimation(&staticX, MODE_SKIP);
	registerAnimation(&staticY, MODE_SKIP);
	registerAnimation(&staticZ, MODE_SKIP);
	curTime = millis();
	lastSwapTime = curTime;
}

int loopAnimation = 0;
void loop()
{
	//  Accept BT Commands for setup
	//  while (Serial.available()) {
	//    cmd = Serial.readString();
	//    Serial.write(cmd.c_str());
	//    Serial.write("\n");
	//    sendCommand(cmd.c_str());
	//  }

	int minDelay = 16;
	curTime = millis();
	parseCommand();

	if (state.off)
	{
		t.Clear();
	}
	else if (state.animation == -1)
	{
		int swapTime = 10000;
		if (aniList.mode[loopAnimation] == MODE_EXTEND)
			swapTime *= 2;

		if (curTime - lastSwapTime > swapTime)
		{
			lastSwapTime = curTime;
			do
			{
				loopAnimation = (loopAnimation + 1) % aniList.count;
			} while (aniList.mode[loopAnimation] == MODE_SKIP);
			do
			{
				loopAnimation = (loopAnimation + 1) % aniList.count;
			} while (aniList.mode[loopAnimation] == MODE_SKIP);
		}
		aniList.animation[loopAnimation]();
	}
	else if (state.animation < NUM_ANIMATIONS && aniList.animation[state.animation])
	{
		aniList.animation[state.animation]();
	}

	for (int i = 0; i < NUM_LEDS; i++)
		strip.setPixelColor(i, strip.Color(leds[i].r, leds[i].g, leds[i].b));
	strip.show();

	unsigned long duration = millis() - curTime;
	if (duration < minDelay)
		delay(minDelay - duration);
}
