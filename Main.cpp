#include <iostream>
#include <SDL2/SDL.h>
#include <rtmidi/RtMidi.h>

using namespace std;

vector<int> VelocityVector = vector<int>(128);

void MidiInCallback(double TimeStamp, vector<unsigned char> *Message, void *UserData)
{
	if (Message->size() == 3)
	{
		int Event = (*Message)[0] & 0xF0;
		int Note = (*Message)[1];
		int Velocity = (*Message)[2];

		if (Event == 0x80) { VelocityVector[Note] = 0; }
		if (Event == 0x90) { VelocityVector[Note] = Velocity; }
	}
}

struct Particle
{
	float PositionX;
	float PositionY;
	float VelocityX;
	float VelocityY;
	float AcellerationX;
	float AcellerationY;
	float MaxVelocity;
	float R;
	float G;
	float B;
	float Fading;
	float ParticleModulationX;
};
vector<Particle> ParticleVector;

int main()
{
	string Title = "MIDI-Visualizer";
	int Width = 1280;
	int Height = 720;

	RtMidiIn TheRtMidiIn;
	int MidiInDevice = -1;

	int WhiteBarR = 63;
	int WhiteBarG = 127;
	int WhiteBarB = 255;

	int BlackBarR = 47;
	int BlackBarG = 95;
	int BlackBarB = 191;

	int NoteSpeed = 3;
	int BarArray[128][664];
	int BarsCount = 0;

	double D2R = M_PI / 180.0;

	for (int I1 = 0; I1 < 128; I1++)
	{
		VelocityVector[I1] = 0;

		for (int I2 = 0; I2 < 664; I2++)
		{
			BarArray[I1][I2] = 0;
		}
	}

	srand(time(nullptr));
	int Result = 0;

	if (MidiInDevice == -1)
	{
		while (true)
		{
			int PortCount = TheRtMidiIn.getPortCount();

			Result = system("clear");
			cout << "Select MIDI Input Device (-1 for exit)" << endl;

			for (int I = 0; I < PortCount; I++)
			{
				cout << I << ". " << TheRtMidiIn.getPortName(I) << endl;
			}

			cin >> MidiInDevice;

			if (MidiInDevice == -1) { return 0; }
			if (MidiInDevice >= 0 && MidiInDevice < PortCount) { break; }
		}
	}

	Result = system("clear");

	TheRtMidiIn.openPort(MidiInDevice);

	vector<unsigned char> MessageBuffer;
	while (true)
	{
		TheRtMidiIn.getMessage(&MessageBuffer);
		if (MessageBuffer.size() == 0) { break; }
	}

	TheRtMidiIn.ignoreTypes(false, false, true);
	TheRtMidiIn.setCallback(MidiInCallback, nullptr);

	Result = SDL_Init(SDL_INIT_VIDEO);
	if (Result < 0) { throw runtime_error("Error: SDL_Init"); }

	SDL_Window* SdlWindow = SDL_CreateWindow(Title.data(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Width, Height, SDL_WINDOW_HIDDEN);
	if (SdlWindow == NULL) { throw runtime_error("Error: SDL_CreateWindow"); }

	SDL_Renderer* SdlRenderer = SDL_CreateRenderer(SdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (SdlRenderer == NULL) { throw runtime_error("Error: SDL_CreateRenderer"); }

	SDL_Event SdlEvent;
	SDL_Rect SdlRect;

	int Frame = 0;
	bool IsRunning = true;

	SDL_ShowWindow(SdlWindow);

	while (IsRunning)
	{
		//Handle Events
		while (SDL_PollEvent(&SdlEvent))
		{
			if (SdlEvent.type == SDL_QUIT) { IsRunning = false; }
		}



		//Update: Move Bars
		BarsCount = 0;
		for (int I1 = 0; I1 < 128; I1++)
		{
			BarArray[I1][663] = VelocityVector[I1];

			bool Flag = false;
			for (int I2 = 0; I2 < NoteSpeed; I2++)
			{
				for (int I3 = 1; I3 < 664; I3++)
				{
					BarArray[I1][I3 - 1] = BarArray[I1][I3];

					if (I2 == 0)
					{
						if (BarArray[I1][I3] > 0 && !Flag) { Flag = true; BarsCount++; }
						if (BarArray[I1][I3] == 0 && Flag) { Flag = false; }
					}
				}
			}
		}

		//Update: Create Particles White Keys
		int OffsetX = 0;
		for (int I1 = 0; I1 < 128; I1++)
		{
			if (((1354 >> (I1 % 12)) % 2) == 1) { continue; }

			int X = OffsetX * 17 + 3;

			if (VelocityVector[I1] > 0)
			{
				Particle NewParticle;
				NewParticle.PositionX = X + 8;
				NewParticle.PositionY = 663;
				NewParticle.VelocityX = 0;
				NewParticle.VelocityY = 0;
				NewParticle.AcellerationX = 0;
				NewParticle.AcellerationY = -(rand() / (float)RAND_MAX) - 0.01;

				NewParticle.MaxVelocity = NoteSpeed;
				NewParticle.R = WhiteBarR * VelocityVector[I1] * 2 / 255;
				NewParticle.G = WhiteBarG * VelocityVector[I1] * 2 / 255;
				NewParticle.B = WhiteBarB * VelocityVector[I1] * 2 / 255;

				NewParticle.Fading = 0.99;
				NewParticle.ParticleModulationX = BarsCount - 1;

				ParticleVector.push_back(NewParticle);
			}

			OffsetX++;
		}

		//Update: Create Particles Black Keys
		OffsetX = 0;
		for (int I1 = 0; I1 < 128; I1++)
		{
			if (((1354 >> (I1 % 12)) % 2) == 0) { continue; }

			int X = OffsetX * 17 + 13;

			if (VelocityVector[I1] > 0)
			{
				Particle NewParticle;
				NewParticle.PositionX = X + 6.5;
				NewParticle.PositionY = 663;
				NewParticle.VelocityX = 0;
				NewParticle.VelocityY = 0;
				NewParticle.AcellerationX = 0;
				NewParticle.AcellerationY = -(rand() / (float)RAND_MAX) - 0.01;

				NewParticle.MaxVelocity = NoteSpeed;
				NewParticle.R = BlackBarR * VelocityVector[I1] / 255;
				NewParticle.G = BlackBarG * VelocityVector[I1] / 255;
				NewParticle.B = BlackBarB * VelocityVector[I1] / 255;
				NewParticle.Fading = 0.99;
				NewParticle.ParticleModulationX = BarsCount - 1;

				ParticleVector.push_back(NewParticle);
			}

			OffsetX++;
			if (I1 % 12 == 3) { OffsetX++; }
			if (I1 % 12 == 10) { OffsetX++; }
		}

		//Update: Move Particles
		for (int I = 0; I < ParticleVector.size(); I++)
		{
			ParticleVector[I].AcellerationX = sin(D2R * Frame * ParticleVector[I].PositionY * ParticleVector[I].ParticleModulationX) * 0.01 * (BarsCount + 1);

			ParticleVector[I].VelocityX += ParticleVector[I].AcellerationX;
			ParticleVector[I].VelocityY += ParticleVector[I].AcellerationY;

			if (ParticleVector[I].VelocityX < -ParticleVector[I].MaxVelocity) { ParticleVector[I].VelocityX = -ParticleVector[I].MaxVelocity; }
			if (ParticleVector[I].VelocityY < -ParticleVector[I].MaxVelocity) { ParticleVector[I].VelocityY = -ParticleVector[I].MaxVelocity; }
			if (ParticleVector[I].VelocityX > ParticleVector[I].MaxVelocity) { ParticleVector[I].VelocityX = ParticleVector[I].MaxVelocity; }
			if (ParticleVector[I].VelocityY > ParticleVector[I].MaxVelocity) { ParticleVector[I].VelocityY = ParticleVector[I].MaxVelocity; }

			ParticleVector[I].PositionX += ParticleVector[I].VelocityX;
			ParticleVector[I].PositionY += ParticleVector[I].VelocityY;

			ParticleVector[I].R *= ParticleVector[I].Fading;
			ParticleVector[I].G *= ParticleVector[I].Fading;
			ParticleVector[I].B *= ParticleVector[I].Fading;

			bool DoDelete = false;
			if (ParticleVector[I].PositionX < 0) { DoDelete = true; }
			if (ParticleVector[I].PositionY < 0) { DoDelete = true; }
			if (ParticleVector[I].PositionX > Width - 1) { DoDelete = true; }
			if (ParticleVector[I].PositionY > Height - 1) { DoDelete = true; }
			if (ParticleVector[I].R < 1 && ParticleVector[I].G < 1 && ParticleVector[I].B < 1) { DoDelete = true; }

			if (DoDelete)
			{
				ParticleVector.erase(ParticleVector.begin() + I);
				continue;
			}
		}



		SDL_SetRenderDrawColor(SdlRenderer, 0, 0, 0, 255);
		SDL_RenderClear(SdlRenderer);

		//Draw: Particles
		for (int I = 0; I < ParticleVector.size(); I++)
		{
			SdlRect.x = ParticleVector[I].PositionX - 1;
			SdlRect.y = ParticleVector[I].PositionY - 1;
			SdlRect.w = 3;
			SdlRect.h = 3;
			SDL_SetRenderDrawColor(SdlRenderer, ParticleVector[I].R, ParticleVector[I].G, ParticleVector[I].B, 255);
			SDL_RenderFillRect(SdlRenderer, &SdlRect);
		}

		//Draw: Bars White Keys
		OffsetX = 0;
		for (int I1 = 0; I1 < 128; I1++)
		{
			if (((1354 >> (I1 % 12)) % 2) == 1) { continue; }

			int X = OffsetX * 17 + 3;

			int RectStart = -1;
			int RectEnd = -1;

			for (int I2 = 0; I2 < 664; I2++)
			{
				int R = WhiteBarR * BarArray[I1][I2] * 2 / 255;
				int G = WhiteBarG * BarArray[I1][I2] * 2 / 255;
				int B = WhiteBarB * BarArray[I1][I2] * 2 / 255;

				SDL_SetRenderDrawColor(SdlRenderer, R, G, B, 255);

				if (BarArray[I1][I2] > 0 && RectStart == -1)
				{
					RectStart = I2;

					for (int I3 = I2; I3 < 664; I3++)
					{
						if (BarArray[I1][I3] == 0 && RectEnd == -1) { RectEnd = I3; break; }
					}

					if (RectEnd == -1) { RectEnd = 663; }
					I2 = RectEnd;

					SdlRect.x = X;
					SdlRect.y = RectStart;
					SdlRect.w = 16;
					SdlRect.h = RectEnd - RectStart + NoteSpeed;
					SDL_RenderFillRect(SdlRenderer, &SdlRect);

					RectStart = -1;
					RectEnd = -1;
				}
			}

			OffsetX++;
		}

		//Draw: Bars Black Keys
		OffsetX = 0;
		for (int I1 = 0; I1 < 128; I1++)
		{
			if (((1354 >> (I1 % 12)) % 2) == 0) { continue; }

			int X = OffsetX * 17 + 13;

			int RectStart = -1;
			int RectEnd = -1;

			for (int I2 = 0; I2 < 664; I2++)
			{
				int R = BlackBarR * BarArray[I1][I2] / 255;
				int G = BlackBarG * BarArray[I1][I2] / 255;
				int B = BlackBarB * BarArray[I1][I2] / 255;

				SDL_SetRenderDrawColor(SdlRenderer, R, G, B, 255);

				if (BarArray[I1][I2] > 0 && RectStart == -1)
				{
					RectStart = I2;

					for (int I3 = I2; I3 < 664; I3++)
					{
						if (BarArray[I1][I3] == 0 && RectEnd == -1) { RectEnd = I3; break; }
					}

					if (RectEnd == -1) { RectEnd = 663; }
					I2 = RectEnd;

					SdlRect.x = X;
					SdlRect.y = RectStart;
					SdlRect.w = 13;
					SdlRect.h = RectEnd - RectStart + NoteSpeed;
					SDL_RenderFillRect(SdlRenderer, &SdlRect);

					RectStart = -1;
					RectEnd = -1;
				}
			}

			OffsetX++;
			if (I1 % 12 == 3) { OffsetX++; }
			if (I1 % 12 == 10) { OffsetX++; }
		}

		//Draw: White Keys
		OffsetX = 0;
		for (int I1 = 0; I1 < 128; I1++)
		{
			if (((1354 >> (I1 % 12)) % 2) == 1) { continue; }

			int X = OffsetX * 17 + 3;

			if (VelocityVector[I1] > 0)
			{
				int R = WhiteBarR * VelocityVector[I1] * 2 / 255;
				int G = WhiteBarG * VelocityVector[I1] * 2 / 255;
				int B = WhiteBarB * VelocityVector[I1] * 2 / 255;

				SdlRect.x = X;
				SdlRect.y = Height - 52;
				SdlRect.w = 16;
				SdlRect.h = 49;
				SDL_SetRenderDrawColor(SdlRenderer, R, G, B, 255);
				SDL_RenderFillRect(SdlRenderer, &SdlRect);
			}
			else
			{
				SdlRect.x = X;
				SdlRect.y = Height - 52;
				SdlRect.w = 16;
				SdlRect.h = 49;
				SDL_SetRenderDrawColor(SdlRenderer, 255, 255, 255, 255);
				SDL_RenderFillRect(SdlRenderer, &SdlRect);
			}

			SdlRect.x = X;
			SdlRect.y = Height - 52;
			SdlRect.w = 16;
			SdlRect.h = 49;
			SDL_RenderDrawRect(SdlRenderer, &SdlRect);
			OffsetX++;
		}

		//Draw: Black Keys
		OffsetX = 0;
		for (int I1 = 0; I1 < 128; I1++)
		{
			if (((1354 >> (I1 % 12)) % 2) == 0) { continue; }

			int X = OffsetX * 17 + 13;

			if (VelocityVector[I1] > 0)
			{
				int R = BlackBarR * VelocityVector[I1] / 255;
				int G = BlackBarG * VelocityVector[I1] / 255;
				int B = BlackBarB * VelocityVector[I1] / 255;

				SdlRect.x = X;
				SdlRect.y = Height - 52;
				SdlRect.w = 13;
				SdlRect.h = 29;
				SDL_SetRenderDrawColor(SdlRenderer, R, G, B, 255);
				SDL_RenderFillRect(SdlRenderer, &SdlRect);
			}
			else 
			{
				SdlRect.x = X;
				SdlRect.y = Height - 52;
				SdlRect.w = 13;
				SdlRect.h = 29;
				SDL_SetRenderDrawColor(SdlRenderer, 0, 0, 0, 255);
				SDL_RenderFillRect(SdlRenderer, &SdlRect);
			}

			OffsetX++;
			if (I1 % 12 == 3) { OffsetX++; }
			if (I1 % 12 == 10) { OffsetX++; }
		}

		//Draw: Border
		SdlRect.x = 0;
		SdlRect.y = 0;
		SdlRect.w = Width;
		SdlRect.h = Height;
		SDL_SetRenderDrawColor(SdlRenderer, 31, 31, 31, 255);
		SDL_RenderDrawRect(SdlRenderer, &SdlRect);

		SdlRect.x = 1;
		SdlRect.y = 1;
		SdlRect.w = Width - 2;
		SdlRect.h = Height - 2;
		SDL_SetRenderDrawColor(SdlRenderer, 63, 63, 63, 255);
		SDL_RenderDrawRect(SdlRenderer, &SdlRect);

		SdlRect.x = 2;
		SdlRect.y = 2;
		SdlRect.w = Width - 4;
		SdlRect.h = Height - 4;
		SDL_SetRenderDrawColor(SdlRenderer, 127, 127, 127, 255);
		SDL_RenderDrawRect(SdlRenderer, &SdlRect);

		//Draw: Lines Above Keys
		SDL_RenderDrawLine(SdlRenderer, 2, Height - 53, Width - 4, Height - 53);

		SDL_SetRenderDrawColor(SdlRenderer, 63, 63, 63, 255);
		SDL_RenderDrawLine(SdlRenderer, 3, Height - 54, Width - 6, Height - 54);

		SDL_RenderPresent(SdlRenderer);
		Frame++;
	}

	TheRtMidiIn.cancelCallback();
	TheRtMidiIn.closePort();

	SDL_DestroyRenderer(SdlRenderer);
	SDL_DestroyWindow(SdlWindow);
	SDL_Quit();

	return 0;
}
