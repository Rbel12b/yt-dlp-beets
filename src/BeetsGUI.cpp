#define STB_IMAGE_IMPLEMENTATION
#include "BeetsGUI.hpp"
#include <cstring>
#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>

namespace beets
{

    namespace BeetsGUI
    {
        std::unordered_map<std::string, Texture> textureCache = {};
        std::vector<BeetsTrack> albums = {};
        const BeetsTrack* trackToShow = nullptr;

        std::vector<unsigned char> extractEmbeddedArt(const std::string &filePath)
        {
            TagLib::MPEG::File file(filePath.c_str());
            if (!file.isOpen())
                return {};

            auto *tag = file.ID3v2Tag();
            if (!tag)
                return {};

            auto frames = tag->frameListMap()["APIC"];
            if (frames.isEmpty())
                return {};

            auto *pic = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(frames.front());
            if (!pic)
                return {};

            auto data = pic->picture();
            return std::vector<unsigned char>(data.begin(), data.end());
        }

        Texture loadCoverTexture(const std::string &filePath, CreateTextureFunc createTexture)
        {
            // If already cached
            if (auto it = textureCache.find(filePath); it != textureCache.end())
                return it->second;

            auto bytes = extractEmbeddedArt(filePath);
            if (bytes.empty())
                return Texture{nullptr, 0, 0};

            int w, h, channels;
            unsigned char *img = stbi_load_from_memory(bytes.data(), bytes.size(), &w, &h, &channels, 4);
            if (!img)
                return Texture{nullptr, 0, 0};

            SDL_Texture *texId = (SDL_Texture*)createTexture(img, w, h);
            stbi_image_free(img);

            if (!texId)
                return Texture{nullptr, 0, 0};

            Texture tex;

            tex.id = texId;
            tex.width = w;
            tex.height = h;

            textureCache[filePath] = tex;

            return tex;
        }

        void showTrack(const BeetsTrack &t)
        {
            trackToShow = &t;
        }

        void renderTrack(const BeetsTrack &t, CreateTextureFunc createTexture)
        {
            auto cover = loadCoverTexture(t.path, createTexture);

            if (ImGui::Button("Back"))
            {
                trackToShow = nullptr;
            }

            ImGui::Text("Track: %s", t.title.c_str());

            if (cover.id)
            {
                float aspect = (float)cover.width / (float)cover.height;
                ImGui::Image((ImTextureID)cover.id, ImVec2(100 * aspect, 100));
            }
            else
            {
                ImGui::TextDisabled("[No cover]");
            }

            ImGui::Text("Artist: %s", t.artist.c_str());
            if (t.album.length())
            {
                ImGui::Text("Album: %s", t.album.c_str());
            }
        }

        void clearTextures()
        {
            for (auto &[path, tex] : textureCache)
            {
                if (tex.id)
                    SDL_DestroyTexture((SDL_Texture*)tex.id);
            }
            textureCache.clear();
        }

        void DrawBeetsGUI(BeetsBackend &beets, CreateTextureFunc createTexture)
        {
            static bool loaded = false;
            static char search[128] = "";
            static auto grouped = beets.groupByArtistAlbum();

            if (trackToShow != nullptr)
            {
                renderTrack(*trackToShow, createTexture);
                ImGui::End();
                return;
            }

            if (ImGui::Button("Reload Library") || !loaded)
            {
                if (beets.loadLibrary())
                {
                    grouped = beets.groupByArtistAlbum();
                    loaded = true;
                }
            }

            ImGui::SameLine();
            ImGui::InputTextWithHint("##search", "Search...", search, IM_ARRAYSIZE(search));

            ImGui::Separator();

            if (!loaded)
            {
                ImGui::TextDisabled("Click 'Reload Library' to fetch Beets data.");
            }
            else
            {
                int i = 0;
                for (auto &[artist, albums] : grouped)
                {
                    if (strlen(search) > 0 && artist.find(search) == std::string::npos)
                    {
                        continue;
                    }
                    ImGui::PushID(i);
                    if (ImGui::TreeNode(artist.c_str()))
                    {
                        for (auto& [album_name, tracks] : albums)
                        {
                            bool albumVisible = false;
                            for (auto &t : tracks)
                            {
                                if (strlen(search) == 0 ||
                                    t->title.find(search) != std::string::npos ||
                                    album_name.find(search) != std::string::npos ||
                                    t->artist.find(search) != std::string::npos)
                                {
                                    albumVisible = true;
                                    break;
                                }
                            }
                            if (!albumVisible || tracks.size() == 0)
                                continue;

                            if (album_name == "" || album_name == " ")
                            {
                                for (auto &t : tracks)
                                {
                                    if (strlen(search) == 0 ||
                                        t->title.find(search) != std::string::npos ||
                                        album_name.find(search) != std::string::npos ||
                                        t->artist.find(search) != std::string::npos)
                                    {
                                        ImGui::BulletText("%s", t->title.c_str());
                                        if (ImGui::IsItemClicked()) {
                                            showTrack(*t);
                                        }
                                    }
                                }
                                continue;
                            }

                            ImGui::PushID((album_name + "album").c_str());
                            auto cover = loadCoverTexture(tracks[0]->path, createTexture);

                            if (ImGui::CollapsingHeader(album_name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                            {
                                if (cover.id)
                                {
                                    float aspect = (float)cover.width / (float)cover.height;
                                    ImGui::Image((ImTextureID)cover.id, ImVec2(100 * aspect, 100));
                                }
                                else
                                {
                                    ImGui::TextDisabled("[No cover]");
                                }

                                for (auto &t : tracks)
                                {
                                    if (strlen(search) == 0 ||
                                        t->title.find(search) != std::string::npos ||
                                        album_name.find(search) != std::string::npos ||
                                        t->artist.find(search) != std::string::npos)
                                    {
                                        ImGui::BulletText("%s", t->title.c_str());
                                        if (ImGui::IsItemClicked()) {
                                            showTrack(*t);
                                        }
                                    }
                                }
                            }
                            ImGui::PopID();
                        }
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
            }
        }
    };
};