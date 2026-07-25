// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#include <expected>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <system_error>

#include <gtest/gtest.h>

#include "gflow-updater-sdk/Storage.hpp"
#include "gflow-updater-sdk/UpdateStorageService.hpp"

namespace {

using DirResult = std::expected<std::filesystem::path, std::error_code>;

class FakeStorage final : public Storage
{
public:
    DirResult rollback;
    DirResult patch;

    DirResult acquireRollbackDir() override { return rollback; }
    DirResult acquirePatchDir() override { return patch; }
};

class UpdateStorageServiceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        const auto* info = ::testing::UnitTest::GetInstance()->current_test_info();
        m_root = std::filesystem::temp_directory_path() / "gflow_uss" / info->name();

        std::error_code errCode;
        std::filesystem::remove_all(m_root, errCode);
        std::filesystem::create_directories(m_root, errCode);
        ASSERT_FALSE(errCode) << "failed to create temp root: " << errCode.message();
    }

    void TearDown() override
    {
        std::error_code errCode;
        std::filesystem::remove_all(m_root, errCode);
    }

    // Path under the temp root; does not create anything.
    std::filesystem::path path(const std::string& name) const { return m_root / name; }

    // Creates `dir` and writes `dir/fileName` with `contents`.
    void makeDirWithFile(const std::filesystem::path& dir,
                         const std::string& fileName,
                         const std::string& contents) const
    {
        std::filesystem::create_directories(dir);
        std::ofstream out(dir / fileName, std::ios::trunc);
        out << contents;
    }

    static std::string readFile(const std::filesystem::path& file)
    {
        std::ifstream in(file);
        std::string contents;
        std::getline(in, contents);
        return contents;
    }

    static std::unique_ptr<FakeStorage> makeStorage(DirResult rollback, DirResult patch)
    {
        auto storage = std::make_unique<FakeStorage>();
        storage->rollback = std::move(rollback);
        storage->patch = std::move(patch);
        return storage;
    }

    static std::error_code someError()
    {
        return std::make_error_code(std::errc::permission_denied);
    }

    std::filesystem::path m_root;
};

// --- backupCurrentVersion ---------------------------------------------------

TEST_F(UpdateStorageServiceTest, BackupMovesApplicationDirIntoRollback)
{
    const auto appDir = path("app");
    const auto rollbackDir = path("rollback");
    makeDirWithFile(appDir, "marker.txt", "app-contents");

    UpdateStorageService service(makeStorage(rollbackDir, DirResult{path("patch")}));

    const auto errCode = service.backupCurrentVersion(appDir);

    EXPECT_FALSE(errCode) << errCode.message();
    EXPECT_FALSE(std::filesystem::exists(appDir));
    ASSERT_TRUE(std::filesystem::exists(rollbackDir / "marker.txt"));
    EXPECT_EQ(readFile(rollbackDir / "marker.txt"), "app-contents");
}

TEST_F(UpdateStorageServiceTest, BackupPropagatesAcquireFailure)
{
    UpdateStorageService service(
        makeStorage(std::unexpected(someError()), DirResult{path("patch")}));

    const auto errCode = service.backupCurrentVersion(path("app"));

    EXPECT_EQ(errCode, someError());
}

TEST_F(UpdateStorageServiceTest, BackupReturnsErrorWhenApplicationDirMissing)
{
    UpdateStorageService service(makeStorage(path("rollback"), DirResult{path("patch")}));

    // appDir does not exist, so the rename cannot succeed.
    const auto errCode = service.backupCurrentVersion(path("app"));

    EXPECT_TRUE(errCode);
}

// --- restoreBackup ----------------------------------------------------------

TEST_F(UpdateStorageServiceTest, RestoreReplacesApplicationDirWithRollback)
{
    const auto appDir = path("app");
    const auto rollbackDir = path("rollback");
    makeDirWithFile(appDir, "stale.txt", "stale");
    makeDirWithFile(rollbackDir, "marker.txt", "backup-contents");

    UpdateStorageService service(makeStorage(rollbackDir, DirResult{path("patch")}));

    const auto errCode = service.restoreBackup(appDir);

    EXPECT_FALSE(errCode) << errCode.message();
    EXPECT_FALSE(std::filesystem::exists(rollbackDir));
    EXPECT_FALSE(std::filesystem::exists(appDir / "stale.txt"));
    ASSERT_TRUE(std::filesystem::exists(appDir / "marker.txt"));
    EXPECT_EQ(readFile(appDir / "marker.txt"), "backup-contents");
}

TEST_F(UpdateStorageServiceTest, RestorePropagatesAcquireFailure)
{
    UpdateStorageService service(
        makeStorage(std::unexpected(someError()), DirResult{path("patch")}));

    const auto errCode = service.restoreBackup(path("app"));

    EXPECT_EQ(errCode, someError());
}

TEST_F(UpdateStorageServiceTest, RestoreReturnsErrorWhenRollbackDirMissing)
{
    // rollback path is valid but nothing exists there, so the rename fails.
    UpdateStorageService service(makeStorage(path("rollback"), DirResult{path("patch")}));

    const auto errCode = service.restoreBackup(path("app"));

    EXPECT_TRUE(errCode);
}

// --- installUpdate ----------------------------------------------------------

TEST_F(UpdateStorageServiceTest, InstallMovesPatchIntoApplicationDir)
{
    const auto appDir = path("app");
    const auto patchDir = path("patch");
    makeDirWithFile(patchDir, "marker.txt", "patch-contents");

    UpdateStorageService service(makeStorage(DirResult{path("rollback")}, patchDir));

    const auto errCode = service.installUpdate(appDir);

    EXPECT_FALSE(errCode) << errCode.message();
    EXPECT_FALSE(std::filesystem::exists(patchDir));
    ASSERT_TRUE(std::filesystem::exists(appDir / "marker.txt"));
    EXPECT_EQ(readFile(appDir / "marker.txt"), "patch-contents");
}

TEST_F(UpdateStorageServiceTest, InstallPropagatesAcquireFailure)
{
    UpdateStorageService service(
        makeStorage(DirResult{path("rollback")}, std::unexpected(someError())));

    const auto errCode = service.installUpdate(path("app"));

    EXPECT_EQ(errCode, someError());
}

TEST_F(UpdateStorageServiceTest, InstallReturnsErrorWhenPatchDirMissing)
{
    // patch path is valid but nothing exists there, so the rename fails.
    UpdateStorageService service(makeStorage(DirResult{path("rollback")}, path("patch")));

    const auto errCode = service.installUpdate(path("app"));

    EXPECT_TRUE(errCode);
}

// --- clearStorage -----------------------------------------------------------

TEST_F(UpdateStorageServiceTest, ClearRemovesPatchAndRollbackDirs)
{
    const auto rollbackDir = path("rollback");
    const auto patchDir = path("patch");
    makeDirWithFile(rollbackDir, "a.txt", "a");
    makeDirWithFile(patchDir, "b.txt", "b");

    UpdateStorageService service(makeStorage(rollbackDir, patchDir));

    const auto errCode = service.clearStorage();

    EXPECT_FALSE(errCode) << errCode.message();
    EXPECT_FALSE(std::filesystem::exists(rollbackDir));
    EXPECT_FALSE(std::filesystem::exists(patchDir));
}

TEST_F(UpdateStorageServiceTest, ClearSucceedsWhenDirsAlreadyAbsent)
{
    // Neither dir exists; remove_all on a missing path is not an error.
    UpdateStorageService service(makeStorage(path("rollback"), path("patch")));

    const auto errCode = service.clearStorage();

    EXPECT_FALSE(errCode) << errCode.message();
}

TEST_F(UpdateStorageServiceTest, ClearPropagatesPatchAcquireFailure)
{
    UpdateStorageService service(
        makeStorage(DirResult{path("rollback")}, std::unexpected(someError())));

    const auto errCode = service.clearStorage();

    EXPECT_EQ(errCode, someError());
}

TEST_F(UpdateStorageServiceTest, ClearPropagatesRollbackAcquireFailure)
{
    UpdateStorageService service(
        makeStorage(std::unexpected(someError()), DirResult{path("patch")}));

    const auto errCode = service.clearStorage();

    EXPECT_EQ(errCode, someError());
}

} // namespace
