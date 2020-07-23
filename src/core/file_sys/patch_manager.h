// Copyright 2018 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <map>
#include <memory>
#include <string>
#include "common/common_types.h"
#include "core/file_sys/nca_metadata.h"
#include "core/file_sys/vfs.h"
#include "core/memory/dmnt_cheat_types.h"

namespace Core {
class System;
}

namespace FileSys {

class NCA;
class NACP;

enum class TitleVersionFormat : u8 {
    ThreeElements, ///< vX.Y.Z
    FourElements,  ///< vX.Y.Z.W
};

std::string FormatTitleVersion(u32 version,
                               TitleVersionFormat format = TitleVersionFormat::ThreeElements);

// Returns a directory with name matching name case-insensitive. Returns nullptr if directory
// doesn't have a directory with name.
std::shared_ptr<VfsDirectory> FindSubdirectoryCaseless(const std::shared_ptr<VfsDirectory> dir,
                                                       std::string_view name);

// A centralized class to manage patches to games.
class PatchManager {
public:
    explicit PatchManager(u64 title_id);
    ~PatchManager();

    u64 GetTitleID() const;

    // Currently tracked ExeFS patches:
    // - Game Updates
    VirtualDir PatchExeFS(VirtualDir exefs) const;

    // Currently tracked NSO patches:
    // - IPS
    // - IPSwitch
    std::vector<u8> PatchNSO(const std::vector<u8>& nso, const std::string& name) const;

    // Checks to see if PatchNSO() will have any effect given the NSO's build ID.
    // Used to prevent expensive copies in NSO loader.
    bool HasNSOPatch(const std::array<u8, 0x20>& build_id) const;

    // Creates a CheatList object with all
    std::vector<Core::Memory::CheatEntry> CreateCheatList(
        const Core::System& system, const std::array<u8, 0x20>& build_id) const;

    // Currently tracked RomFS patches:
    // - Game Updates
    // - LayeredFS
    VirtualFile PatchRomFS(VirtualFile base, u64 ivfc_offset,
                           ContentRecordType type = ContentRecordType::Program,
                           VirtualFile update_raw = nullptr) const;

    // Returns a vector of pairs between patch names and patch versions.
    // i.e. Update 3.2.2 will return {"Update", "3.2.2"}
    std::map<std::string, std::string, std::less<>> GetPatchVersionNames(
        VirtualFile update_raw = nullptr) const;

    // If the game update exists, returns the u32 version field in its Meta-type NCA. If that fails,
    // it will fallback to the Meta-type NCA of the base game. If that fails, the result will be
    // std::nullopt
    std::optional<u32> GetGameVersion() const;

    // Given title_id of the program, attempts to get the control data of the update and parse
    // it, falling back to the base control data.
    std::pair<std::unique_ptr<NACP>, VirtualFile> GetControlMetadata() const;

    // Version of GetControlMetadata that takes an arbitrary NCA
    std::pair<std::unique_ptr<NACP>, VirtualFile> ParseControlNCA(const NCA& nca) const;

private:
    std::vector<VirtualFile> CollectPatches(const std::vector<VirtualDir>& patch_dirs,
                                            const std::string& build_id) const;

    u64 title_id;
};

} // namespace FileSys
