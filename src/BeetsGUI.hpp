#pragma once
#include "Beets.hpp"
#include <imgui.h>
#include <stb_image.h>
#include <unordered_map>
#include <fstream>
#include <vector>

namespace beets
{
    namespace BeetsGUI
    {
        // Abstract texture creation callback type
        using CreateTextureFunc = std::function<void *(const unsigned char *rgba, int width, int height)>;

        struct Texture
        {
            void *id = nullptr;
            int width = 0;
            int height = 0;
        };

        // Extract album art (APIC) from MP3 file
        std::vector<unsigned char> extractEmbeddedArt(const std::string &filePath);

        // Main ImGui UI
        void DrawBeetsGUI(BeetsBackend &beets,
                          CreateTextureFunc createTexture);

        
        void clearTextures();

        extern std::unordered_map<std::string, Texture> textureCache;
        extern std::vector<BeetsTrack> albums;
        extern const BeetsTrack* trackToShow;

        Texture loadCoverTexture(const std::string &filePath, CreateTextureFunc createTexture);

        void showTrack(const BeetsTrack &t);
        void renderTrack(const BeetsTrack &t, CreateTextureFunc createTexture);
    };
};