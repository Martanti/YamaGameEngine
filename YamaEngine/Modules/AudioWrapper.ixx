module;
#define ASS_IMPLEMENTATION
#define WITH_WINMM
#define _CRT_SECURE_NO_WARNINGS
#include "ass.h";
export module AudioWrapper;
import <filesystem>;
import <unordered_map>;
import <map>;
import <string>;
import <memory>;
import Logger;

namespace Audio
{
	namespace fs = std::filesystem;

	// As there is no real plan for how audio would be used in this engine/game
	// The most basic approach is used where the sound is just played upon a call and that's it
	// If there was a need for 3D sounds, etc. an actual SoLoud system would be used instead of this simplified one

	/**
	 * @brief Audio cache, which stores already loaded sound data for reuse.
	*/
	std::map<std::string, SoLoud::AudioSource*> audioCache;

	/**
	 * @brief SoLoud object.
	*/
	SoLoud::Soloud soloud;

	/**
	 * @brief Initialize the audio player.
	*/
	export inline void Init()
	{
		soloud.init();
	}

	/**
	 * @brief Clean up the audio cache and the audio player.
	*/
	export inline void Cleanup()
	{
		for (auto& [path, source] : audioCache)
		{
			delete source;
		}

		soloud.deinit();
	}

	// As the wrapper is quite simplistic creating an additional file with the include would unnecessarily bloat the files

	/**
	 * @brief Audio wrapper.
	 * @note Used for audio source creation.
	*/
	export class AudioSource
	{
	private:
		/**
		 * @brief Handle for played sound.
		*/
		int audioHandle = -1;

		/**
		 * @brief Audio source.
		*/
		SoLoud::AudioSource* audio;
	public:

		/**
		 * @brief Set a sound file.
		 * @note If it is in the cache, the cahced file will be used instead.
		 * @param pathToSound
		*/
		inline void SetSoundPath(std::string pathToSound)
		{
			fs::path absolutePath = fs::absolute(pathToSound);

			auto absolutePathStr = absolutePath.string();
			auto found =  audioCache.find(absolutePathStr);

			if (found != audioCache.end())
			{
				audio = found->second;
				return;
			}

			auto extension = absolutePath.extension();

			if ( extension == ".wav")
			{
				auto wav = new SoLoud::Wav();
				wav->load(absolutePathStr.c_str());
				audio = wav;
			}
			else if (extension == ".mp3" || extension == ".ogg")
			{
				auto stream = new SoLoud::WavStream();
				stream->load(absolutePathStr.c_str());
				audio = stream;
			}

			else
			{
				Log(LGR_ERROR, std::format("Could not match appropriate audio type for the extension of \"{}\", which was seen as \"{}\"", absolutePath.string(), extension.string()));
				return;
			}

			audioCache[absolutePathStr] = audio;
		}

		/**
		 * @brief Play the stored audio.
		*/
		inline void PlaySource()
		{
			if (!audio)
			{
				Log(LGR_ERROR, "The object that should call for audio is not set yet! Please set up the audio source path!");
				return;
			}
			audioHandle = soloud.play(*audio);
		}
	};


	/**
	 * @brief Component will wrap around several audio sources as a game object may have more than one audio sources.
	*/
	export struct AudioComponent
	{
		// Can be easily replaced with map, but since an iteration is not expected and unordered_map gives quicker access it was chosen over standard map

		/**
		 * @brief Collection of audio sources.
		*/
		std::unordered_map<std::string, AudioSource> audioStorage;
	};
}