#pragma once

class Version;
class Updater;

#include <string>
#include <vector>
#include <sstream>
#include "AppState.hpp"

class Version
{
public:
    Version(const std::string &_v)
    {
        m_version = _v;
    }

    int compareVersions(const std::string &v1, const std::string &v2) const
    {
        auto parts1 = splitVersion(v1);
        auto parts2 = splitVersion(v2);

        size_t n = std::max(parts1.size(), parts2.size());
        for (size_t i = 0; i < n; ++i)
        {
            int num1 = (i < parts1.size() ? parts1[i] : 0);
            int num2 = (i < parts2.size() ? parts2[i] : 0);

            if (num1 < num2)
                return -1;
            if (num1 > num2)
                return 1;
        }
        return 0; // equal
    }

    bool operator>(const Version& v2) const
    {
        return compareVersions(this->m_version, v2.m_version) > 0;
    }

    bool operator<(const Version& v2) const
    {
        return compareVersions(this->m_version, v2.m_version) < 0;
    }

    bool operator==(const Version& v2) const
    {
        return compareVersions(this->m_version, v2.m_version) == 0;
    }

    bool operator!=(const Version& v2) const
    {
        return !(*this == v2);
    }

    bool operator<=(const Version& v2) const
    {
        return !(*this > v2);
    }

    bool operator>=(const Version& v2) const
    {
        return !(*this < v2);
    }
    std::vector<int> splitVersion(const std::string &version) const
    {
        std::vector<int> parts;
        std::stringstream ss(version);
        std::string item;
        while (std::getline(ss, item, '.'))
        {
            parts.push_back(std::stoi(item));
        }
        return parts;
    }

    std::string m_version;
};

class Updater
{
public:
    bool checkUpdate(AppState& state);
    bool downloadUpdate(AppState& state);
    void update(AppState& state);

    bool updateAvailable = false;
};