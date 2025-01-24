//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <algorithm>

//engine
#include "audioplayercomponent.hpp"
#include "audio.hpp"

using std::clamp;

using Core::Audio;

namespace Graphics::Components
{
	void AudioPlayerComponent::SetVolume(const string& name, float newVolume)
	{
		volume = newVolume;
		Audio::SetVolume(name, volume);
	}
}