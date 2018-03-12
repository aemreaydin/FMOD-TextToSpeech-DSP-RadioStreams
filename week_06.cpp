#include "common\src\utils\utils.h"

#include <fmod.hpp>
#include <fmod_errors.h>
#include <sapi.h>
#include <sphelper.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#define NUMBER_OF_SOUNDS 3
#define NUMBER_OF_TAGS 4

FMOD_RESULT mResult;
FMOD::System* mSystem = NULL;
FMOD::Channel *mChannels[NUMBER_OF_SOUNDS];
FMOD::Sound *mSounds[NUMBER_OF_SOUNDS];

// PART 1
char* mUrlClassicRock = "http://78.129.202.200:8040";
char* mUrlClassicJazz = "http://37.187.57.33:8028";
char* mUrlClassical = "http://144.217.49.251:80";

FMOD_OPENSTATE mOpenState[NUMBER_OF_SOUNDS] = { FMOD_OPENSTATE_READY, FMOD_OPENSTATE_READY, FMOD_OPENSTATE_READY };
FMOD_TAG mTag;
int mTagIndex = 0;
char mTagString[NUMBER_OF_TAGS][128] = { 0 };

// PART 2
FMOD::Channel* m3DChannels;
FMOD::Sound* m3DSounds;

int mNumDrivers = 0;
int mNumTries = 20;
int mDriverRate = 0;
int mDriverChannels = 0;
char* mRecordState;
bool bDSPEnabled = false;

// Part 3
ISpVoice *mVoice = NULL;
bool bSpeech = false;
std::string mParagraph0;
std::string mParagraph1;
std::string mParagraph2;

LPCWSTR mConvertedParagraph0;
LPCWSTR mConvertedParagraph1;
LPCWSTR mConvertedParagraph2;
LPCWSTR mConvertedParagraph3;

// Part 4
HRESULT hResult = S_OK;
CComPtr<ISpVoice> mISpVoice;
CComPtr<ISpStream> mISpStream;
CSpStreamFormat mStreamFormat;

FMOD::Sound* mDSPSound;
FMOD::Channel* mDSPChannel;
FMOD::ChannelGroup *master;

FMOD::DSP* lowPass = 0;
FMOD::DSP* chorus = 0;
FMOD::DSP* echo = 0;
bool bLowPass, bChorus, bEcho;
bool once = true;
//

bool bEscape = false;
bool bKeyDown = false;
int mCurrentSound = 0;

void initMod();
void clearScreen();
void checkError(FMOD_RESULT result);
void handleKeyboard();
std::wstring importParagraphs(std::string filename, std::string &output);
std::wstring s2ws(const std::string& s);


int main()
{
	initMod();

	mResult = mSystem->setStreamBufferSize(65536, FMOD_TIMEUNIT_RAWBYTES);
	checkError(mResult);

	// PART 1
	mResult = mSystem->createSound(mUrlClassicRock, FMOD_CREATESTREAM | FMOD_NONBLOCKING, 0, &mSounds[0]);
	checkError(mResult);
	mResult = mSystem->createSound(mUrlClassicJazz, FMOD_CREATESTREAM | FMOD_NONBLOCKING, 0, &mSounds[1]);
	checkError(mResult);
	mResult = mSystem->createSound(mUrlClassical, FMOD_CREATESTREAM | FMOD_NONBLOCKING, 0, &mSounds[2]);
	checkError(mResult);

	unsigned int position[NUMBER_OF_SOUNDS] = { 0, 0, 0 };
	unsigned int percentage[NUMBER_OF_SOUNDS] = { 0, 0, 0 };
	bool bStarving[NUMBER_OF_SOUNDS] = { false, false,  false };
	const char* mCurrentState = "Stopped.";

	// PART 2
	for(int i = 0; i < mNumTries && mNumDrivers <= 0; i++)
	{
		mNumTries--;
		Sleep(200);
		mResult = mSystem->getRecordNumDrivers(0, &mNumDrivers);
		checkError(mResult);
	}
	if (mNumDrivers == 0)
	{
		std::cout << "Drivers not found." << std::endl;
		exit(EXIT_FAILURE);
	}
	mResult = mSystem->getRecordDriverInfo(0, NULL, 0, NULL, &mDriverRate, NULL, &mDriverChannels, NULL);
	checkError(mResult);

	FMOD_CREATESOUNDEXINFO soundInfo = { 0 };
	soundInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	soundInfo.numchannels = mDriverChannels;
	soundInfo.format = FMOD_SOUND_FORMAT_PCM16;
	soundInfo.defaultfrequency = mDriverRate;
	soundInfo.length = mDriverRate * sizeof(short) * mDriverChannels;

	mResult = mSystem->createSound(0, FMOD_LOOP_NORMAL | FMOD_OPENUSER, &soundInfo, &m3DSounds);
	checkError(mResult);

	// PART 3
	std::wstring w0, w1, w2;
	w0 = importParagraphs("paragraph0.txt", mParagraph0);
	mConvertedParagraph0 = w0.c_str();
	w1 = importParagraphs("paragraph1.txt", mParagraph1);
	mConvertedParagraph1 = w1.c_str();
	w2 = importParagraphs("paragraph2.txt", mParagraph2);
	mConvertedParagraph2 = w2.c_str();
	
	if (FAILED(::CoInitialize(NULL)))
		return false;

	HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&mVoice);

	// PART 4
	std::wstring w3;
	w3 = importParagraphs("part4paragraph.txt", mParagraph2);
	mConvertedParagraph3 = w3.c_str();

	if (FAILED(::CoInitialize(NULL)))
		return false;

	hResult = mISpVoice.CoCreateInstance(CLSID_SpVoice);
	if (SUCCEEDED(hResult))
		hResult = mStreamFormat.AssignFormat(SPSF_22kHz16BitStereo);

	if (SUCCEEDED(hResult))
	{
		//hResult = SPBindToFile(L"c:\\aydin_emre\\part4.wav", SPFM_CREATE_ALWAYS, &mISpStream, &mStreamFormat.FormatId(), mStreamFormat.WaveFormatExPtr());
		hResult = SPBindToFile(L"part4.wav", SPFM_CREATE_ALWAYS, &mISpStream, &mStreamFormat.FormatId(), mStreamFormat.WaveFormatExPtr());
	}

	if (SUCCEEDED(hResult))
		hResult = mISpVoice->SetOutput(mISpStream, true);

	mResult = mSystem->getMasterChannelGroup(&master);
	checkError(mResult);
	
	mResult = mSystem->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &lowPass);
	checkError(mResult);
	mResult = mSystem->createDSPByType(FMOD_DSP_TYPE_HIGHPASS, &chorus);
	checkError(mResult);
	mResult = mSystem->createDSPByType(FMOD_DSP_TYPE_ECHO, &echo);
	checkError(mResult);
	mResult = lowPass->setBypass(true);
	checkError(mResult);
	mResult = chorus->setBypass(true);
	checkError(mResult);
	mResult = echo->setBypass(true);
	checkError(mResult);

	mResult = master->addDSP(0, lowPass);
	checkError(mResult);
	mResult = master->addDSP(0, echo);
	checkError(mResult);
	mResult = master->addDSP(0, chorus);
	checkError(mResult);


	//

	bool bPlaying = false;
	bool bPaused = false;

	while (!bEscape)
	{
		start_text();
		handleKeyboard();

		mResult = mSystem->update();
		checkError(mResult);

		if (mCurrentSound >= 0 && mCurrentSound <= 2)
		{
			mResult = mSounds[0]->getOpenState(&mOpenState[0], &percentage[0], &bStarving[0], 0);
			checkError(mResult);
			
			mResult = mSounds[1]->getOpenState(&mOpenState[1], &percentage[1], &bStarving[1], 0);
			checkError(mResult);
			mResult = mSounds[2]->getOpenState(&mOpenState[2], &percentage[2], &bStarving[2], 0);
			checkError(mResult);

			if (mChannels[mCurrentSound])
			{
				while (mSounds[mCurrentSound]->getTag(0, -1, &mTag) == FMOD_OK)
				{
					if (mTag.datatype == FMOD_TAGDATATYPE_STRING)
					{
						sprintf(mTagString[mTagIndex], "%s = %s (%d bytes)", mTag.name, (char*)mTag.data, mTag.datalen);
						mTagIndex = (mTagIndex + 1) % NUMBER_OF_TAGS;
					}
					else if (mTag.type == FMOD_TAGTYPE_FMOD)
					{
						float freq = *((float*)mTag.data);
						mResult = mChannels[mCurrentSound]->setFrequency(freq);
						checkError(mResult);
					}
				}
				mResult = mChannels[mCurrentSound]->getPaused(&bPaused);
				checkError(mResult);
				mResult = mChannels[mCurrentSound]->isPlaying(&bPlaying);
				checkError(mResult);
				mResult = mChannels[mCurrentSound]->getPosition(&position[mCurrentSound], FMOD_TIMEUNIT_MS);
				checkError(mResult);
				mResult = mChannels[mCurrentSound]->setMute(bStarving[mCurrentSound]);
				checkError(mResult);
			}
			else
			{
				mSystem->playSound(mSounds[mCurrentSound], 0, false, &mChannels[mCurrentSound]);
			}

			if (mOpenState[mCurrentSound] == FMOD_OPENSTATE_CONNECTING)
				mCurrentState = "Connecting";
			else if (mOpenState[mCurrentSound] == FMOD_OPENSTATE_BUFFERING)
				mCurrentState = "Buffering";
			else if (bPaused)
				mCurrentState = "Paused";
			else if (bPlaying)
				mCurrentState = "Playing";

			print_text("Current sound: %d", mCurrentSound);
			print_text("Time: %02d:%02d:%02d", position[mCurrentSound] / 1000 / 60, position[mCurrentSound] / 1000 % 60, position[mCurrentSound] / 10 % 100);
			print_text("Current State: %s-%s", mCurrentState, bStarving[mCurrentSound] ? "(STARVING)" : "(NOT STARVING)");
			print_text("Buffer percentage: %d", percentage[mCurrentSound]);
			print_text("Tags:");
			for (int index = mTagIndex; index < (mTagIndex + NUMBER_OF_TAGS); index++)
			{
				print_text("%s", mTagString[index % NUMBER_OF_TAGS]);
				print_text("");
			}
		}
		else if(mCurrentSound >= 3 && mCurrentSound <= 6)
		{
			if (mCurrentSound == 3)
			{
				mRecordState = "Recording.";
			}
			else if (mCurrentSound == 4)
			{
				mRecordState = "Stopped Recording. Playing.";
			}
			print_text("%s", mRecordState);
		}
		else if (mCurrentSound >= 7 && mCurrentSound <= 9)
		{
			
		}

		if (bSpeech)
		{
			//if (SUCCEEDED(hr))
			//{
			//	hr = mVoice->Speak(mConvertedParagraph0, 0, NULL);
			//	hr = mVoice->Speak(mConvertedParagraph1, 0, NULL);
			//	hr = mVoice->Speak(mConvertedParagraph2, 0, NULL);
			//}
			if (SUCCEEDED(hResult))
			{
				hResult = mISpVoice->Speak(mConvertedParagraph3, 0, NULL);
			}
			if (SUCCEEDED(hResult))
			{
				hResult = mISpStream->Close();
			}
		}

		end_text();
		Sleep(50);
	}

	for (int i = 0; i < 3; i++)
	{
		if (m3DSounds)
		{
			mResult = m3DSounds->release();
			checkError(mResult);
		}
	}

	if (mSystem)
	{
		mResult = mSystem->close();
		checkError(mResult);
		mResult = mSystem->release();
		checkError(mResult);
	}
	return 0;
}

void initMod()
{
	// Create the System
	mResult = FMOD::System_Create(&mSystem);
	checkError(mResult);
	// Initialize the System
	mResult = mSystem->init(512, FMOD_INIT_NORMAL, 0);
	checkError(mResult);
}

void clearScreen()
{
	start_text();
	for (int i = 0; i < 20; i++)
	{
		print_text("																	");
	}
	end_text();
}

void checkError(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		fprintf(stderr, "FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		exit(-1);
	}
}

//************************************************************************
// NEW

std::wstring importParagraphs(std::string filename, std::string& output)
{
	std::ifstream File(filename);
	std::stringstream Stream;
	std::string String;	

	Stream << File.rdbuf();
	String = Stream.str();

	output = String;
	std::wstring temp = s2ws(String);
	
	return temp;
}

//************************************************************************



void handleKeyboard()
{
	//Esc key pressed
	if (GetAsyncKeyState(VK_ESCAPE)) {
		bEscape = true;
	}

	//===============================================================================================
	// Space bar pressed		
	if ((GetKeyState(VK_SPACE) < 0) && !bKeyDown) {
		bKeyDown = true;

		if (mCurrentSound >= 0 && mCurrentSound <= 2)
		{
			bool isPaused = true;
			mResult = mChannels[mCurrentSound]->getPaused(&isPaused);
			checkError(mResult);
			mResult = mChannels[mCurrentSound]->setPaused(!isPaused);
			checkError(mResult);
		}		
		else if (mCurrentSound >= 3 && mCurrentSound <= 6)
		{
			bool is3DPaused = true;
			mResult = m3DChannels->getPaused(&is3DPaused);
			checkError(mResult);
			mResult = m3DChannels->setPaused(!is3DPaused);
			checkError(mResult);
		}
		
		Sleep(200);
		bKeyDown = false;
	}
	else if ((GetKeyState(VK_RETURN) < 0) && !bKeyDown)
	{
		bKeyDown = true;

		bSpeech = !bSpeech;

		Sleep(200);
		bKeyDown = false;
	}
	else if ((GetKeyState(VK_SHIFT) < 0) && !bKeyDown)
	{
		bKeyDown = true;

		if (once)
		{
			once = false;

			mResult = mSystem->createSound("part4.wav", FMOD_CREATECOMPRESSEDSAMPLE, 0, &mDSPSound);
			checkError(mResult);
			mResult = mDSPSound->setMode(FMOD_LOOP_NORMAL);
			checkError(mResult);
			mResult = mSystem->playSound(mDSPSound, 0, false, &mDSPChannel);
			checkError(mResult);
		}

		Sleep(200);
		bKeyDown = false;
	}

	//===============================================================================================
	//Arrow UP
	else if ((GetKeyState(VK_UP) < 0) && !bKeyDown) {
		bKeyDown = true;


		Sleep(200);
		bKeyDown = false;
	}

	//===============================================================================================
	//Arrow Down
	else if ((GetKeyState(VK_DOWN) < 0) && !bKeyDown) {
		bKeyDown = true;


		Sleep(200);
		bKeyDown = false;
	}

	//===============================================================================================
	//Arrow right
	else if ((GetKeyState(VK_RIGHT) < 0) && !bKeyDown) {
		bKeyDown = true;

		Sleep(50);
		bKeyDown = false;
	}

	//===============================================================================================
	//Arrow left
	else if ((GetKeyState(VK_LEFT) < 0) && !bKeyDown) {
		bKeyDown = true;


		Sleep(50);
		bKeyDown = false;
	}
	else if ((GetKeyState(0x30) < 0) && !bKeyDown) {
		bKeyDown = true;

		clearScreen();
		mCurrentSound = 0;

		Sleep(200);
		bKeyDown = false;
	}
	//===============================================================================================
	//Number 1
	else if ((GetKeyState(0x31) < 0) && !bKeyDown) {
		bKeyDown = true;

		clearScreen();
		mCurrentSound = 1;

		Sleep(200);
		bKeyDown = false;
	}

	//===============================================================================================
	//Number 2
	else if ((GetKeyState(0x32) < 0) && !bKeyDown) { //Key down
		bKeyDown = true;

		clearScreen();
		mCurrentSound = 2;

		Sleep(200);
		bKeyDown = false;
	}

	//===============================================================================================
	//Number 3
	else if ((GetKeyState(0x33) < 0) && !bKeyDown) {
		bKeyDown = true;

		clearScreen();
		mCurrentSound = 3;
		mResult = mSystem->recordStart(0, m3DSounds, true);
		checkError(mResult);
		mResult = mSystem->playSound(m3DSounds, 0, false, &m3DChannels);
		checkError(mResult);

		Sleep(200);
		bKeyDown = false;
	}

	//===============================================================================================
	//Number 4
	else if ((GetKeyState(0x34) < 0) && !bKeyDown) {
		bKeyDown = true;

		clearScreen();
		mCurrentSound = 4;

		//mResult = mSystem->recordStop(0);
		//checkError(mResult);


		FMOD_REVERB_PROPERTIES dspOn = FMOD_PRESET_CAVE;
		FMOD_REVERB_PROPERTIES dspOff = FMOD_PRESET_OFF;

		bDSPEnabled = !bDSPEnabled;

		mResult = mSystem->setReverbProperties(0, bDSPEnabled ? &dspOn : &dspOff);
		checkError(mResult);

		Sleep(200);
		bKeyDown = false;
	}
	else if ((GetKeyState(0x35) < 0) && !bKeyDown) {
		bKeyDown = true;

		mCurrentSound = 5;

		FMOD_REVERB_PROPERTIES dspOn = FMOD_PRESET_ALLEY;
		FMOD_REVERB_PROPERTIES dspOff = FMOD_PRESET_OFF;

		bDSPEnabled = !bDSPEnabled;

		mResult = mSystem->setReverbProperties(0, bDSPEnabled ? &dspOn : &dspOff);
		checkError(mResult);


		Sleep(200);
		bKeyDown = false;
	}
	else if ((GetKeyState(0x36) < 0) && !bKeyDown) {
		bKeyDown = true;

		mCurrentSound = 6;

		FMOD_REVERB_PROPERTIES dspOn = FMOD_PRESET_AUDITORIUM;
		FMOD_REVERB_PROPERTIES dspOff = FMOD_PRESET_OFF;

		bDSPEnabled = !bDSPEnabled;

		mResult = mSystem->setReverbProperties(0, bDSPEnabled ? &dspOn : &dspOff);
		checkError(mResult);


		Sleep(200);
		bKeyDown = false;
	}
	else if ((GetKeyState(0x37) < 0) && !bKeyDown) {
		bKeyDown = true;

		mCurrentSound = 7;

		mResult = lowPass->setBypass(bLowPass);
		bLowPass = !bLowPass;
		checkError(mResult);

		Sleep(200);
		bKeyDown = false;
	}
	else if ((GetKeyState(0x38) < 0) && !bKeyDown) {
		bKeyDown = true;

		mResult = chorus->setBypass(bChorus);
		bChorus = !bChorus;
		checkError(mResult);

		Sleep(200);
		bKeyDown = false;
	}
	else if ((GetKeyState(0x39) < 0) && !bKeyDown) {
		bKeyDown = true;

		mResult = echo->setBypass(bEcho);
		bEcho = !bEcho;
		checkError(mResult);

		Sleep(200);
		bKeyDown = false;
	}
	else if ((GetKeyState(0x41) < 0) && !bKeyDown) { //A Button
		bKeyDown = true;


		Sleep(200);
		bKeyDown = false;
	}
	else if ((GetKeyState(0x44) < 0) && !bKeyDown) { //D Button
		bKeyDown = true;


		Sleep(200);
		bKeyDown = false;
	}
	else if ((GetKeyState(0x50) < 0) && !bKeyDown) { //P Button
		bKeyDown = true;

		//initialize sound
		for (int i = 0; i < NUMBER_OF_SOUNDS; i++)
		{
			mResult = mChannels[i]->setPaused(true);
			checkError(mResult);
			mResult = mChannels[i]->setPosition(0, FMOD_TIMEUNIT_MS);
			checkError(mResult);
		}


		Sleep(200);
		bKeyDown = false;
	}
}
std::wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;

}