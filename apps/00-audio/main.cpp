/*
SoLoud audio engine
Copyright (c) 2013-2014 Jari Komppa

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/

#include <stdlib.h>
#include <stdio.h>

#include "soloud.h"
#include "soloud_speech.h"
#include "soloud_modplug.h"
#include "soloud_monotone.h"
#include "soloud_sfxr.h"
#include "soloud_tedsid.h"
#include "soloud_wav.h"
#include "soloud_thread.h"

#ifdef _MSC_VER
#include <conio.h>
int mygetch()
{
	return _getch();
}
#else
#include <termios.h>
#include <unistd.h> 
int mygetch( ) 
{
  struct termios oldt, newt;
  int ch;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  return ch;
}
#endif



// Entry point
int main(int argc, char *argv[])
{
	// Declare some variables
	SoLoud::Soloud soloud; // Engine core
	SoLoud::Speech speech; // A sound source (speech, in this case)
	SoLoud::Wav wav;       // One sample source
	SoLoud::Modplug mod;   // One song source

	// Initialize SoLoud (automatic back-end selection)
	// also, enable visualization for FFT calc
	soloud.init();
	soloud.setVisualizationEnable(1);
	printf("%s\n", "Welcome to Soloud!");

	// Load background sample
	wav.load("delphi_loop.ogg");       // Load a wave file
	wav.setLooping(1);                          // Tell SoLoud to loop the sound
	int handle1 = soloud.play(wav);             // Play it
	soloud.setVolume(handle1, 0.5f);            // Set volume; 1.0f is "normal"
	soloud.setPan(handle1, -0.2f);              // Set pan; -1 is left, 1 is right
	soloud.setRelativePlaySpeed(handle1, 0.9f); // Play a bit slower; 1.0f is normal

	// Configure sound source
	printf("%s\n>", "What is your name?");
	char name[512];
	fgets(name, 511, stdin);	
	speech.setText(name);
	speech.setVolume(5);
	// Play the sound source (we could do this several times if we wanted)
	soloud.play(speech);


	// Wait for voice to finish
	while (soloud.getVoiceCount() > 1)
	{
		// Still going, sleep for a bit
		SoLoud::Thread::sleep(100);
	}

	soloud.stop(handle1); // stop the wind sound

	// Load song
	{
		SoLoud::result loaded = mod.load("bruce.s3m");
		if (SoLoud::SO_NO_ERROR == loaded) 
		{
			soloud.play(mod);
			printf("%s\n", "Playing 'bruce.s3m' MODULE music. Press a key to quit..");
			mygetch();
			mod.stop();
		} 
		else 
		{
			printf("%s\n", "Cannot find bruce.s3m (or --with-libmodplug build option may be missing)");
		}
	}

	{
		SoLoud::TedSid galway;
		SoLoud::result loaded = galway.load("modulation.sid.dump");
		if (SoLoud::SO_NO_ERROR == loaded) 
		{
			soloud.play(galway);
			printf("%s\n", "Playing 'modulation.sid.dump' TED/SID music. Press a key to quit..");
			mygetch();
			galway.stop();
		} 
	}

	{
		SoLoud::Monotone swingin1;
		SoLoud::result loaded = swingin1.load("jakim-aboriginal.mon");
		if (SoLoud::SO_NO_ERROR == loaded) 
		{
			soloud.play(swingin1);
			printf("%s\n", "Playing 'jakim-aboriginal.mon' MONOTONE music. Press a key to quit..");
			mygetch();
			swingin1.stop();
		} 
	}

	/*
	{
		SoLoud::Sfxr boom;
		SoLoud::result loaded = boom.loadParams("boom.sfx");
		if (SoLoud::SO_NO_ERROR == loaded) 
		{
			soloud.play(boom);
			printf("%s\n", "Playing 'boom.sfx' SFXR sound. Press a key to quit..");
			mygetch();
			boom.stop();
		} 
	}
	*/

	{
		SoLoud::Sfxr coin;
		coin.loadPreset(SoLoud::Sfxr::COIN, 3247);
		for( int i = 0; i < 10; ++i ) {
			soloud.play(coin);
			printf("%s\n", "Playing 'Coin #3247' SFXR sound. Press a key to quit..");
			mygetch();
		}
		coin.stop();
	}



	// Clean up SoLoud
	soloud.deinit();

	// All done.
	return 0;
}